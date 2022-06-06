#include "opengl3.h"

#include <stdlib.h>

xcb_visualid_t visualId_gl;
xcb_colormap_t colormap_gl;

GLXFBConfig fbConfig;
GLXWindow glxwindow;
GLXContext glxcontext;

// For clearing the window
float color = 0.0f;
float delta = 0.01f;


int setupGlx()
{
    if (!gladLoaderLoadGLX(display, XDefaultScreen(display)))
        return 1;

    const int visualAttribs[] = {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        GLX_ALPHA_SIZE,   8,
        GLX_DEPTH_SIZE,   24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    GLXFBConfig *framebufferConfigs = NULL;
    int numFbConfigs = 0;

    framebufferConfigs = glXChooseFBConfig(
        display,
        XDefaultScreen(display),
        visualAttribs,
        &numFbConfigs
    );

    fbConfig = framebufferConfigs[0];

    glXGetFBConfigAttrib(
        display,
        fbConfig,
        GLX_VISUAL_ID,
        &visualId_gl
    );

    colormap_gl = xcb_generate_id(connection);

    xcb_create_colormap(
        connection,
        XCB_COLORMAP_ALLOC_NONE,
        colormap_gl,
        screen->root,
        visualId_gl
    );

    return 0;
}


void shutdownGlx(void)
{
    xcb_free_colormap(connection, colormap_gl);
    gladLoaderUnloadGLX();
}


int initOpengl()
{
    glxcontext = glXCreateNewContext(
        display,
        fbConfig,
        GLX_RGBA_TYPE,
        0,
        True
    );

    if (!glxcontext)
        return 1;

    glxwindow = glXCreateWindow(
        display,
        fbConfig,
        window,
        NULL // Attribute list
    );

    if (!glxwindow) {
        glXDestroyContext(display, glxcontext);
        return 2;
    }

    glXMakeContextCurrent(
        display,
        glxwindow,
        glxwindow,
        glxcontext
    );

    if (!gladLoaderLoadGL()) {
        glXDestroyContext(display, glxcontext);
        glXDestroyWindow(display, glxwindow);
        return 3;
    }

    return 0;
}

void destroyOpengl() {
    glXDestroyWindow(display, glxwindow);
    glXDestroyContext(display, glxcontext);
}


void draw() {
    glXMakeContextCurrent(
        display,
        glxwindow,
        glxwindow,
        glxcontext
    );

    glViewport(0, 0, W, H);

    color += delta;

    if (color >= 1.0f) {
        delta = -delta;
        color = 1.0f;
    }

    if (color <= 0.0f) {
        delta = -delta;
        color = 0.0f;
    }

    glClearColor(color, color, color, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glXMakeContextCurrent(
        display,
        0, 0, 0
    );
}


void swapBuffers(void) {
    glXMakeContextCurrent(
        display,
        glxwindow,
        glxwindow,
        glxcontext
    );

    glXSwapBuffers(display, glxwindow);

    glXMakeContextCurrent(
        display,
        0, 0, 0
    );
}