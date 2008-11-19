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
 * @file GLSurface.h
 * @brief GLSurface definition
 */

#pragma once

#include <GL/glew.h>  // include before gl.h
#include <QGLWidget>

#include <Cg/cg.h>

struct apr_pool_t;

class GLSurface : public QGLWidget
{
    Q_OBJECT

protected:
    bool flip_y;
    GLuint tex_id;
    apr_pool_t* pool;
    QSize image_size;
    QSize surface_size;
    QPointF image_position;
    QPointF prev_mouse_point;
    float scale;

    bool use_shader;
    CGcontext cg_context;
    CGprofile cg_fragment_profile;
    CGprogram cg_fragment_program;

    struct {
        CGparameter scene_tex;
        CGparameter exposure;
    } cg_params;

    struct {
        float exposure;
    } tmapr;

public:
    GLSurface ();
    virtual ~GLSurface ();

    void load_image (QImage& image);
    void load_image (const QString& fname);

protected:
    virtual void initializeGL ();
    virtual void resizeGL (int w, int h);
    virtual void paintGL ();

    virtual void mousePressEvent (QMouseEvent* evt);
    virtual void mouseMoveEvent (QMouseEvent* evt);
    virtual void wheelEvent (QWheelEvent* evt);
    virtual void keyPressEvent (QKeyEvent* evt);

private:
    void showMessage (const QString& message, int timeout = 0);
};

// vim: sw=4 fdm=marker
