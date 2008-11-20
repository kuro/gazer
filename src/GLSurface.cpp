/*
 * Copyright 2008 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file GLSurface.cpp
 * @brief GLSurface implementation
 */

#define DCRAW_4 1

/* includes {{{*/
#include "GLSurface.moc"
#include "MainWindow.h"

#include <gas/swap.h>
#include <assert.h>

#include <QtCore>
#include <QMouseEvent>
#include <QMainWindow>
#include <QStatusBar>

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImfTestFile.h>

#include <apr_pools.h>

#include <Cg/cgGL.h>

#include "shaders.h"

/*}}}*/

struct Color/*{{{*/
{
    union {
        struct {
            float r, g, b, a;
        };
        float value[4];
    };
};/*}}}*/

/* error helpers {{{*/
#define GLERRCHK()                                                          \
    do {                                                                    \
        int glstatus = glGetError();                                        \
        if (glstatus != GL_NO_ERROR) {                                      \
            fprintf(stderr, "%s:%d: GL error: 0x%x: %s\n",                  \
                    __FILE__, __LINE__, glstatus,                           \
                    gluErrorString(glstatus));                              \
            fflush(stderr);                                                 \
            exit(1);                                                        \
        }                                                                   \
    } while (0)
/*}}}*/

GLSurface::GLSurface () :/*{{{*/
    QGLWidget(),
    flip_y(false),
    tex_id(0),
    pool(NULL),
    image_size(0, 0),
    image_position(0, 0),
    scale(1.0f),
    use_shader(true)
{
    apr_pool_create(&pool, NULL);
    setFocusPolicy(Qt::StrongFocus);

    tmapr.exposure = 1.1f;
}/*}}}*/
GLSurface::~GLSurface ()/*{{{*/
{
}/*}}}*/

void GLSurface::initializeGL ()/*{{{*/
{
    GLenum glew_status;
    if ((glew_status = glewInit()) != GLEW_OK) {
        qFatal("GLEW error: %s", glewGetErrorString(glew_status));
    }

    glGenTextures(1, &tex_id);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);

    GLERRCHK();

/* cg {{{*/
    cg_context = cgCreateContext();
    cg_fragment_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(cg_fragment_profile);
    cg_fragment_program = cgCreateProgram(cg_context, CG_SOURCE,
                                          tonemap_shader_source,
                                          cg_fragment_profile, "tonemap", NULL);
    cgGLLoadProgram(cg_fragment_program);

    cg_params.scene_tex = cgGetNamedParameter(cg_fragment_program, "scene_tex");
    cg_params.exposure = cgGetNamedParameter(cg_fragment_program,
                                             "tmapr.exposure");
/*}}}*/

}/*}}}*/
void GLSurface::resizeGL (int w, int h)/*{{{*/
{
    surface_size = QSize(w, h);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 1, -1);

    if (image_position.x() == 0 && image_position.y() == 0) {
        image_position.setX(0.5 * w);
        image_position.setY(0.5 * h);
    }

}/*}}}*/
void GLSurface::paintGL ()/*{{{*/
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (tex_id == 0) {
        return;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    if (flip_y) {
        glScalef(1.0f, -1.0f, 1.0f);
    }


    if (use_shader) {
        cgGLEnableProfile(cg_fragment_profile);
        cgGLBindProgram(cg_fragment_program);

        cgGLSetTextureParameter(cg_params.scene_tex, tex_id);
        cgGLSetParameter1f(cg_params.exposure, tmapr.exposure);
    }

    glMatrixMode(GL_MODELVIEW);
    glTranslatef(image_position.x(), image_position.y(), 0.0f);
    glScalef(scale, scale, 1.0f);
    glTranslatef(-0.5f * image_size.width(),
                 -0.5f * image_size.height(), 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);glVertex2f(0.0f              , 0.0f);
    glTexCoord2f(1.0f,0.0f);glVertex2f(image_size.width(), 0.0f);
    glTexCoord2f(1.0f,1.0f);glVertex2f(image_size.width(), image_size.height());
    glTexCoord2f(0.0f,1.0f);glVertex2f(0.0f              , image_size.height());
    glEnd();

    if (use_shader) {
        cgGLDisableProfile(cg_fragment_profile);
    }

    GLERRCHK();
}/*}}}*/

void GLSurface::load_image (QImage& image)/*{{{*/
{
    if (image.isNull()) {
        throw "image not valid";
    }

    image_size = image.size();
    tex_id = bindTexture(image, GL_TEXTURE_2D, GL_RGBA16F_ARB);

    flip_y = false;
    updateGL();
}/*}}}*/
void GLSurface::load_image (const QString& fname)/*{{{*/
{

/* try OpenEXR {{{*/
    if (fname.endsWith("exr")) {

        if ( ! Imf::isOpenExrFile(qPrintable(fname))) {
            throw "invalid exr format";
        }

        Imf::RgbaInputFile file (qPrintable(fname));
        Imath::Box2i dw = file.dataWindow();
        image_size.setWidth(dw.max.x - dw.min.x + 1);
        image_size.setHeight(dw.max.y - dw.min.y + 1);
        Imf::Array2D<Imf::Rgba> pix;
        pix.resizeErase(image_size.height(), image_size.width());
        file.setFrameBuffer(&pix[0][0], 1, image_size.width());
        file.readPixels(dw.min.y, dw.max.y);

        flip_y = true;

        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        if (GL_EXT_framebuffer_object) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB,
                         image_size.width(), image_size.height(),
                         0, GL_RGBA, GL_HALF_FLOAT_ARB, &pix[0][0]);
            glGenerateMipmapEXT(GL_TEXTURE_2D);
        } else if (GL_SGIS_generate_mipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB,
                         image_size.width(), image_size.height(),
                         0, GL_RGBA, GL_HALF_FLOAT_ARB, &pix[0][0]);
        } else {
            size_t bytes_per_line = image_size.width() * sizeof(Color);
            size_t size = image_size.height() * bytes_per_line;
            Color* fpix = (Color*)apr_palloc(pool, size);
            Color c;
            flip_y = false;  // flipping pixels since loop is already necessary
            for (int y = 0; y < image_size.height(); y++) {
                for (int x = 0; x < image_size.width(); x++) {
                    c.r = pix[y][x].r;
                    c.g = pix[y][x].g;
                    c.b = pix[y][x].b;
                    fpix[x+(image_size.height()-y-1)*image_size.width()]=c;
                }
            }
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA16F_ARB,
                              image_size.width(), image_size.height(),
                              GL_RGBA, GL_FLOAT, fpix);
            apr_pool_clear(pool);
        }
        updateGL();
        return;
    }
/*}}}*/

/* try dcraw {{{*/
    QProcess dcraw (this);
    QStringList args;
    args << "-i" << qPrintable(fname);
    dcraw.start("dcraw", args);
    if (dcraw.waitForStarted()) {
        dcraw.waitForFinished();
        if (dcraw.exitCode() == 0) {
            args.clear();
            args << "-c";

#if 1
            args << "-w";  // camera white balance
#else
            args << "-a";  // whole image average white balance
#endif

#if DCRAW_4
            args << "-4";
#endif
            args << qPrintable(fname);
            dcraw.start("dcraw", args);
            if (dcraw.waitForStarted()) {
                dcraw.waitForReadyRead();

                QString ppm_type = dcraw.readLine().trimmed();
                assert(ppm_type == "P6");
                QByteArray line = dcraw.readLine().trimmed();
                QList<QByteArray> wh = line.split(' ');
                int w = wh[0].toInt();
                int h = wh[1].toInt();
                image_size = QSize(w, h);
#if DCRAW_4
                qint64 byte_size = w * h * sizeof(quint16) * 3;
#else
                qint64 byte_size = w * h * sizeof(quint8) * 3;
#endif
                quint16* pixels = (quint16*)apr_palloc(pool, byte_size);

                dcraw.waitForFinished();
                int max = dcraw.readLine().trimmed().toInt();
#if DCRAW_4
                assert(max == 0xffff);
#else
                assert(max == 0xff);
#endif
                qint64 bytes_read = dcraw.read((char*)pixels, byte_size);
                if (bytes_read != byte_size) {
                    qDebug() << "failed to read" << byte_size;
                }

                assert(dcraw.atEnd() == true);

#if DCRAW_4
                gas_swap(pixels, sizeof(quint16), byte_size);
#endif

                glBindTexture(GL_TEXTURE_2D, tex_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_LINEAR_MIPMAP_LINEAR);
                GLenum type = 0;
#if DCRAW_4
                type = GL_UNSIGNED_SHORT;
#else
                type = GL_UNSIGNED_BYTE;
#endif
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB,
                             image_size.width(), image_size.height(),
                             0, GL_RGB, type, pixels);
                glGenerateMipmapEXT(GL_TEXTURE_2D);
                apr_pool_clear(pool);

                flip_y = true;

                updateGL();
                return;
            } else {
                qDebug() << "dcraw didn't start again";
            }
        }
    }
/*}}}*/

    QImage img (fname);
    load_image(img);

}/*}}}*/

void GLSurface::mousePressEvent (QMouseEvent* evt)/*{{{*/
{
    QPointF pos = evt->pos();
    pos.ry() *= -1;
    prev_mouse_point = pos;
}/*}}}*/
void GLSurface::mouseMoveEvent (QMouseEvent* evt)/*{{{*/
{
    QPointF pos = evt->pos();
    pos.ry() *= -1;
    QPointF delta = pos - prev_mouse_point;
    image_position += delta;
    prev_mouse_point = pos;
    updateGL();
}/*}}}*/
void GLSurface::wheelEvent (QWheelEvent* evt)/*{{{*/
{
    int degs = evt->delta() / 8;
    int steps = degs / 15;

#if 0
    scale += 0.1 * steps;
#endif

//    if (use_shader) {
//        tmapr.exposure += 0.1 * steps;
//        showMessage(QString("Exposure %1").arg(tmapr.exposure));
//    }
//
//    updateGL();


    reinterpret_cast<MainWindow*>(parent())->next(steps * -1);

}/*}}}*/
void GLSurface::keyPressEvent (QKeyEvent* evt)/*{{{*/
{
    switch (evt->key()) {
    case '-':
        scale -= 0.1f;
        showMessage(QString("Zoom %1%").arg(scale*100));
        break;
    case '+':
    case '=':
        scale += 0.1f;
        showMessage(QString("Zoom %1%").arg(scale*100));
        break;
    case 'S':
        use_shader = ! use_shader;
        showMessage(QString("Shader %1").arg((use_shader ? "on" : "off")));
        break;
    case '[':
        if (use_shader) {
            tmapr.exposure -= 0.1f;
            showMessage(QString("Exposure %1").arg(tmapr.exposure));
        }
        break;
    case ']':
        if (use_shader) {
            tmapr.exposure += 0.1f;
            showMessage(QString("Exposure %1").arg(tmapr.exposure));
        }
        break;
    }
    updateGL();
}/*}}}*/

void GLSurface::showMessage (const QString& message, int timeout)/*{{{*/
{
    reinterpret_cast<QMainWindow*>(parent()) \
        ->statusBar()->showMessage(message, timeout);
}/*}}}*/

// vim: sw=4 fdm=marker
