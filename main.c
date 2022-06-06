#include <X11/Xlib-xcb.h>
#include <xcb/sync.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "opengl3.h"

// Size of a thing in bits
#define SIZEOF_BITS(THING) (sizeof(THING) * 8)

// Length of a static array 
#define STATIC_LEN(ARRAY) (sizeof(ARRAY) / sizeof(*ARRAY))

Display *display;
xcb_connection_t *connection;
xcb_screen_t *screen;
xcb_window_t window;
xcb_sync_counter_t syncCounter;
xcb_sync_int64_t syncValue;
int W;
int H;

xcb_atom_t
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    _NET_WM_SYNC_REQUEST,
    _NET_WM_SYNC_REQUEST_COUNTER;

void setupXcb(void);
void shutdownXcb(void);

void createWindow(void);
void destroyWindow(void);

void runEventLoop(void);

void getAtoms(void);


int main() {
    setupXcb();
    createWindow();
    runEventLoop();
    destroyWindow();
    shutdownXcb();
    
    return 0;
}


void setupXcb(void) {
    display = XOpenDisplay(NULL);
    connection = XGetXCBConnection(display);
    XSetEventQueueOwner(display, XCBOwnsEventQueue);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    getAtoms();

    if (setupGlx()) {
        puts("Cannot initialize GLX!");
        exit(1);
    }
}


void shutdownXcb(void) {
    shutdownGlx();
    xcb_disconnect(connection);
}


void getAtoms(void) {
    const char *names[] = {
        "WM_PROTOCOLS",
        "WM_DELETE_WINDOW",
        "_NET_WM_SYNC_REQUEST",
        "_NET_WM_SYNC_REQUEST_COUNTER",
    };

    const int length = STATIC_LEN(names);

    xcb_intern_atom_cookie_t coockies[length];
    xcb_atom_t atoms[length];

    for (int i=0; i<length; i++)
        coockies[i] = xcb_intern_atom(
            connection,
            1,
            strlen(names[i]),
            names[i]
        );

    for (int i=0; i<length; i++)
    {
        xcb_intern_atom_reply_t *reply;
        reply = xcb_intern_atom_reply(connection, coockies[i], NULL);
        atoms[i] = reply->atom;
        free(reply);
    }

    WM_PROTOCOLS = atoms[0];
    WM_DELETE_WINDOW = atoms[1];
    _NET_WM_SYNC_REQUEST = atoms[2];
    _NET_WM_SYNC_REQUEST_COUNTER = atoms[3];
}


void createWindow(void) {
    uint32_t windowProperties[] = {
        screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY,
        colormap_gl
    };

    window = xcb_generate_id(connection);
    xcb_create_window(
        connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0, 800, 600,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        visualId_gl,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP,
        windowProperties
    );

    xcb_atom_t protocols[] = {
        WM_DELETE_WINDOW,
        _NET_WM_SYNC_REQUEST,
    };

    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        window,
        WM_PROTOCOLS,
        XCB_ATOM_ATOM,
        SIZEOF_BITS(*protocols),
        STATIC_LEN(protocols),
        protocols
    );

    syncCounter = xcb_generate_id(connection);
    syncValue.hi = 0;
    syncValue.lo = 0;
    xcb_sync_create_counter(connection, syncCounter, syncValue);

    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        window,
        _NET_WM_SYNC_REQUEST_COUNTER,
        XCB_ATOM_CARDINAL,
        SIZEOF_BITS(syncCounter),
        1,
        &syncCounter
    );

    xcb_map_window(connection, window);
    xcb_flush(connection);

    if (initOpengl()) {
        puts("Cannot initialize OpenGL!");
        exit(1);
    }
}


void destroyWindow(void) {
    destroyOpengl();
    xcb_sync_destroy_counter(connection, syncCounter);
    xcb_destroy_window(connection, window);
}


void updateSyncCounter(void) {
    syncValue.lo++;
    xcb_sync_set_counter(
        connection,
        syncCounter,
        syncValue
    );
    xcb_flush(connection);
}

void runEventLoop(void) {
    for (int alive=1; alive; ) {
        xcb_generic_event_t *event = xcb_wait_for_event(connection);

        if (event == NULL)
            return;

        switch (event->response_type & ~0x80) {
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t *clientEvent = (void *) event;
                
                uint32_t msgType = clientEvent->data.data32[0];

                if (msgType == WM_DELETE_WINDOW) {
                    alive = 0;
                }
                else if (msgType == _NET_WM_SYNC_REQUEST) {
                    syncValue.lo = clientEvent->data.data32[2];
                    syncValue.hi = clientEvent->data.data32[3];
                }

            }
            break;

            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t *configEvent = (void *) event;
                W = configEvent->width;
                H = configEvent->height;

                draw();

                updateSyncCounter();
            }
            break;

            case XCB_EXPOSE:
            {
                swapBuffers();

                updateSyncCounter();
            }
            break;
        }

        free(event);
    }
}