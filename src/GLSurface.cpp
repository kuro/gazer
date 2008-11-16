
/**
 * @file GLSurface.cpp
 * @brief GLSurface implementation
 */

/* includes {{{*/
#include "GLSurface.moc"

#include <QtCore>
#include <QMouseEvent>

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImfTestFile.h>
#include <apr_pools.h>
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
    scale(1.0f)
{
    apr_pool_create(&pool, NULL);
    setFocusPolicy(Qt::StrongFocus);
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
}/*}}}*/
void GLSurface::resizeGL (int w, int h)/*{{{*/
{
    surface_size = QSize(w, h);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 1, -1);
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

    glMatrixMode(GL_MODELVIEW);
    glTranslatef(image_position.x(), image_position.y(), 0.0f);
    glScalef(scale, scale, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);glVertex2f(0.0f              , 0.0f);
    glTexCoord2f(1.0f,0.0f);glVertex2f(image_size.width(), 0.0f);
    glTexCoord2f(1.0f,1.0f);glVertex2f(image_size.width(), image_size.height());
    glTexCoord2f(0.0f,1.0f);glVertex2f(0.0f              , image_size.height());
    glEnd();

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
}/*}}}*/
void GLSurface::load_image (const QString& fname)/*{{{*/
{
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
    } else {
        QImage img (fname);
        load_image(img);
    }
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
#if 0
    int degs = evt->delta() / 8;
    int steps = degs / 15;
    scale += 0.1 * steps;
    updateGL();
#endif
}/*}}}*/
void GLSurface::keyPressEvent (QKeyEvent* evt)/*{{{*/
{
    switch (evt->key()) {
    case '-':
        scale -= 0.1f;
        break;
    case '+':
    case '=':
        scale += 0.1f;
        break;
    }
    updateGL();
}/*}}}*/

// vim: sw=4 fdm=marker
