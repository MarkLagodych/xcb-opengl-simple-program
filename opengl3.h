#ifndef _MY_OPENGL3_H_
#define _MY_OPENGL3_H_

#include <glad/gl.h>
#include <glad/glx.h>

#include <X11/Xlib-xcb.h>

// Defined in main.c
extern Display *display;
extern xcb_connection_t *connection;
extern xcb_screen_t *screen;
extern xcb_window_t window;
extern int W;
extern int H;

// Defined in opengl3.c
extern xcb_visualid_t visualId_gl;
extern xcb_colormap_t colormap_gl;

int setupGlx(void);
void shutdownGlx(void);
int initOpengl(void);
void destroyOpengl(void);

void draw(void);
void swapBuffers(void);

#endif // _MY_OPENGL3_H_