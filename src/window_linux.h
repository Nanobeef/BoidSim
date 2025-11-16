
#pragma once

#if OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>      // XClassHint
#include <X11/Xresource.h>  // XContext
#include <X11/extensions/XShm.h> 	// XShm
#include <X11/extensions/Xpresent.h>  // XContext
#include <sys/ipc.h>
#include <sys/shm.h>


typedef struct{
    Window window;
    Display *display;

    XContext user_pointer;
    Atom fullscreen_atom;
    Atom hidden_atom;
    Atom state_atom;
    Atom delete_atom;
    s32 x,y;
    u32 width, height;
	b32 present_supported;
	b32 shared_memory_supported;
}X11WindowDriver;
#endif //OS_LINUX
