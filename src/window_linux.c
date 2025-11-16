#include "window_properties.h"

#if OS_LINUX

#include "window_linux.h"

typedef struct{
				
}LinuxWindowDriver;

void* create_window_driver_linux(WindowDriverCreateInfo info, Arena *arena)
{
	LinuxWindowDriver *wd = arena_alloc(sizeof(LinuxWindowDriver),0,0, arena);
	RUNTIME_ABORT("create_window_driver_linux() is EMPTY!");
	return wd;
}

void destroy_window_driver_linux(void *driver_data)
{
	LinuxWindowDriver *wd = driver_data;
	RUNTIME_ABORT("destroy_window_driver_linux() is EMPTY!");
}


void poll_window_driver_linux(void *driver_data, Event *event_ring_buffer)
{
	LinuxWindowDriver *wd = driver_data;
	RUNTIME_ABORT("poll_window_driver_linux() is EMPTY!");
}



void* create_window_driver_x11(WindowDriverCreateInfo info, Arena *arena)
{
	Display *display = XOpenDisplay(NULL);
	XAutoRepeatOff(display);
	if(display == NULL)
	{
		return 0;
	}
	if(XDefaultRootWindow(display) == 0)
	{
		XCloseDisplay(display);
		return 0;
	}
	if(0){
		Atom compositor_atom = XInternAtom(display, "_NET_WM_CM_S0", False);
		Window owner = XGetSelectionOwner(display, compositor_atom);
		if(owner == None)
		{
			return 0;
		}
	}

	
	u64 present_event_mask = 0;
	b32 present_supported = false;

	if(XPresentQueryExtension(display,0,0,0))
	{
		present_supported = true;	
		present_event_mask = PresentCompleteNotifyMask | PresentIdleNotifyMask;
	}

	b32 shared_memory_supported = false;
	Bool pixmaps;
	s32 shm_major, shm_minor;
	if(XShmQueryVersion(display, &shm_major, &shm_minor,&pixmaps))
	{
		if(pixmaps == True)
		{
			shared_memory_supported = true;
		}
	}

	shared_memory_supported = false;


	u32 background_color = 0X04080808;
	u32 border_color = 0XFFFFFFFF;
	XSetWindowAttributes window_attributes = {
		.background_pixel = background_color,
		.border_pixel = border_color,
		.background_pixmap = None,
		.bit_gravity = StaticGravity, // Preserve baackground when resizing
		.event_mask =     KeyPressMask | KeyReleaseMask
						| ButtonPressMask | ButtonReleaseMask
						| EnterWindowMask | LeaveWindowMask
						| PointerMotionMask | ButtonMotionMask
						| StructureNotifyMask
						| ExposureMask
						| FocusChangeMask | PropertyChangeMask
						| present_event_mask
	};
	Window window = XCreateWindow(
		display,
		XDefaultRootWindow(display),
		0,0,
		info.width, info.height,
		1,
		DefaultDepth(display, XDefaultScreen(display)),
		InputOutput,
		DefaultVisual(display, XDefaultScreen(display)),
		CWBackPixel | CWBorderPixel | CWBitGravity | CWEventMask | CWBackPixmap,
		&window_attributes
	);

	XContext user_pointer = XUniqueContext();
	if(info.window_pointer)
	{
		XSaveContext(display, window, user_pointer, (void*)info.window_pointer);
	}

	XMapWindow(display, window);

	Atom delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &delete_atom, 1);

	Atom state_atom = XInternAtom(display, "_NET_WM_STATE", False);
	Atom hidden_atom = XInternAtom(display, "_NET_WM_HIDDEN", False);
	Atom fullscreen_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

	XStoreName(display, window, "Hello");
	XClassHint *class_hint = XAllocClassHint();
	*class_hint = (XClassHint){
		.res_name = "Hello",
		.res_class = "Gello",
	};

	XSetClassHint(display, window, class_hint);
	XFree(class_hint);
	XSync(display, False);

	X11WindowDriver *wd = arena_alloc(sizeof(X11WindowDriver), 0,0,arena);
	wd[0] = (X11WindowDriver){
		.window = window,			
		.display = display,
		.user_pointer = user_pointer,
		.fullscreen_atom = fullscreen_atom,
		.state_atom = state_atom,
		.hidden_atom = fullscreen_atom,
		.delete_atom = fullscreen_atom,
		.width = info.width,
		.height = info.height,
		.x = info.x,
		.y = info.y,
		.present_supported = present_supported,
		.shared_memory_supported = shared_memory_supported,
	};
	return wd;
}

void destroy_window_driver_x11(void *driver_data)
{
	X11WindowDriver *wd = driver_data;
	
	XDeleteContext(wd->display,wd->window, wd->user_pointer);
	XDestroyWindow(wd->display, wd->window);
	XCloseDisplay(wd->display);
			
}



void poll_window_driver_x11(void *driver_data, Event *event_ring_buffer)
{
	X11WindowDriver *wd = driver_data;
	while(XPending(wd->display) > 0)
	{
		XEvent xe = {0};
		XNextEvent(wd->display, &xe);
		void *window_pointer = NULL;
		if(xe.xany.window)
		{
			if(XFindContext(wd->display, wd->window, wd->user_pointer, (XPointer*)&window_pointer))
			{
				continue;
			}
		}
		else
		{
			continue;
		}
		Event e = init_event();
		e.window_pointer = window_pointer;
		switch(xe.type)
		{
			case KeyRelease:
			case KeyPress:
			{

				KeyboardEventType keyboard_event_type = KEYBOARD_PRESS;
				if(xe.type == KeyRelease){ keyboard_event_type = KEYBOARD_RELEASE;}

				KeySym xlib_key = XLookupKeysym(&xe.xkey, 0);
				KeyType key = KEY_NONE;
				switch(xlib_key)
				{
					case XK_Escape: key = KEY_ESCAPE; break;
					case XK_a: key = KEY_A; break;
					case XK_b: key = KEY_B; break;
					case XK_c: key = KEY_C; break;
					case XK_d: key = KEY_D; break;
					case XK_e: key = KEY_E; break;
					case XK_f: key = KEY_F; break;
					case XK_g: key = KEY_G; break;
					case XK_h: key = KEY_H; break;
					case XK_i: key = KEY_I; break;
					case XK_j: key = KEY_J; break;
					case XK_k: key = KEY_K; break;
					case XK_l: key = KEY_L; break;
					case XK_m: key = KEY_M; break;
					case XK_n: key = KEY_N; break;
					case XK_o: key = KEY_O; break;
					case XK_p: key = KEY_P; break;
					case XK_q: key = KEY_Q; break;
					case XK_r: key = KEY_R; break;
					case XK_s: key = KEY_S; break;
					case XK_t: key = KEY_T; break;
					case XK_u: key = KEY_U; break;
					case XK_v: key = KEY_V; break;
					case XK_w: key = KEY_W; break;
					case XK_x: key = KEY_X; break;
					case XK_y: key = KEY_Y; break;
					case XK_z: key = KEY_Z; break;
					case XK_1: key = KEY_1; break;
					case XK_2: key = KEY_2; break;
					case XK_3: key = KEY_3; break;
					case XK_4: key = KEY_4; break;
					case XK_5: key = KEY_5; break;
					case XK_6: key = KEY_6; break;
					case XK_7: key = KEY_7; break;
					case XK_8: key = KEY_8; break;
					case XK_9: key = KEY_9; break;
					case XK_0: key = KEY_0; break;
					// LCTROL
					case XK_grave: key = KEY_GRAVE; break;

					default: break;
				}
				{
					e.type = EVENT_KEYBOARD;
					e.keyboard.type = keyboard_event_type;
					e.keyboard.key = key;
					post_event(event_ring_buffer, e);
				}

			}
			break;
			case ConfigureNotify:
			{
				e.type = EVENT_WINDOW;
				WindowEvent window_event = {0};
				u32 width = xe.xconfigure.width;
				u32 height = xe.xconfigure.height;
				s32 x = xe.xconfigure.x;
				s32 y = xe.xconfigure.y;
				if((wd->width != width) || (wd->height != height))
				{
					window_event.type = WINDOW_RESIZE;
					window_event.width = width;
					window_event.height = height;
					wd->width = width;
					wd->height = height;
					e.window =  window_event;
					post_event(event_ring_buffer, e);
				}
				if((wd->x != x) || (wd->y != y))
				{
					window_event.type = WINDOW_MOVE;
					window_event.x = x;
					window_event.y = y;
					wd->x = x;
					wd->y = y;
					e.window =  window_event;
					post_event(event_ring_buffer, e);
				}
			}
			break;
			case EnterNotify:
			case LeaveNotify:
			{
				MouseEvent mouse_event = {0};
				mouse_event.x = xe.xcrossing.x;
				mouse_event.y = xe.xcrossing.y;
				if(xe.type == EnterNotify)
				{
					mouse_event.type = MOUSE_ENTER;
				}
				else
				{
					mouse_event.type = MOUSE_LEAVE;
				}
				{
					e.type = EVENT_MOUSE;
					e.mouse =  mouse_event;
					post_event(event_ring_buffer, e);
				}
			}
			break;
			case MotionNotify:
			{
				MouseEvent mouse_event = {0};
				mouse_event.type = MOUSE_MOVE;
				mouse_event.x = xe.xmotion.x;
				mouse_event.y = xe.xmotion.y;
				{
					e.type = EVENT_MOUSE;
					e.mouse =  mouse_event;
					post_event(event_ring_buffer, e);
				}
			}
			break;

			case ButtonPress:
			case ButtonRelease:
			{
				MouseEvent mouse_event = {0};
				if(xe.type == ButtonPress)
				{
					mouse_event.type = MOUSE_PRESS;
				}
				else
				{
					mouse_event.type = MOUSE_RELEASE;
				}
				switch(xe.xbutton.button)
				{
					case Button1: mouse_event.button = MOUSE_BUTTON_LEFT; break;
					case Button2: mouse_event.button = MOUSE_BUTTON_MIDDLE; break;
					case Button3: mouse_event.button = MOUSE_BUTTON_RIGHT; break;
					case Button4:
					case Button5:
					{
						if(xe.xbutton.button == Button4)
						{
							mouse_event.scroll = -1;
						}
						else
						{
							mouse_event.scroll = 1;
						}
						mouse_event.type = MOUSE_SCROLL;
					}
					break;
					default: break;
				}
				{
					e.type = EVENT_MOUSE;
					e.mouse = mouse_event;
					if((mouse_event.type == MOUSE_SCROLL) && (xe.type == ButtonRelease))
					{
						e.type = 0;
					}
				post_event(event_ring_buffer, e);
				}
			}
			break;


			default:
			break;
		}
	}
}


typedef struct{
	Swapchain swapchain;	
}LinuxWindowSwapchain;

void* create_window_swapchain_linux(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena)
{
	LinuxWindowSwapchain *wf = arena_alloc(sizeof(LinuxWindowSwapchain),0,0, arena);
	RUNTIME_ABORT("create_window_frameburffer_linux() is EMPTY!");
	return &wf->swapchain;
}

void destroy_window_swapchain_linux(void *swapchain)
{
	LinuxWindowSwapchain *ws = ((Swapchain*)swapchain)->data;
	RUNTIME_ABORT("destroy_window_frameburffer_linux() is EMPTY!");
}

void present_window_swapchain_linux(void *swapchain)
{
	LinuxWindowSwapchain *ws = ((Swapchain*)swapchain)->data;
	RUNTIME_ABORT("present_window_frameburffer_linux() is EMPTY!");
}


typedef struct{
	void *driver_data;
    GC gc;
	u64 line_size;
	u32 width,height;
	u32 image_count;

    XImage **ximages;
	Pixmap *pixmaps;
	XShmSegmentInfo *shm_infos;

	Swapchain swapchain;
}X11WindowSwapchain;

void* create_window_swapchain_x11(WindowSwapchainCreateInfo info, void *driver_data, Arena *arena)
{
	X11WindowDriver *wd = driver_data;

	s32 screen = DefaultScreen(wd->display);
    Display *display = wd->display;
    Window window = wd->window;

    GC gc = XCreateGC(display, window, 0, NULL);
	u32 width = wd->width;
	u32 height = wd->height;

	u32 image_count = 2;
	if(info.desired_image_count)
	{
		image_count = info.desired_image_count;
	}

    s32 depth = DefaultDepth(display, screen);

	Visual *visual = DefaultVisual(display, screen); //  Wrong color mapping
	

	XImage **ximages = 0;
	XShmSegmentInfo *shm_infos = 0;
	if(wd->shared_memory_supported == false)
	{
		ximages = arena_alloc(sizeof(XImage*) * image_count,0,0, arena);
	}
	else
	{
		shm_infos = arena_alloc(sizeof(XShmSegmentInfo) * image_count,0,0, arena);	
	}

	Image *images = arena_alloc(sizeof(Image) * image_count, 0,0, arena);
	u32 line_size = 0;

	Pixmap *pixmaps = 0;
	if(wd->present_supported)
	{
		pixmaps = arena_alloc(sizeof(Pixmap) * image_count,0,0, arena);
	}

	for(u32 i = 0; i < image_count; i++)
	{
		XImage* ximage = XCreateImage(display, visual, depth, ZPixmap, 0, NULL, width, height, 32, 0);
		if(ximage == NULL)
		{
			DEBUG_ABORT("Failed to create XImage!");
		}

		line_size = ximage->bytes_per_line;

		void *buffer = 0;

		if(ximages)
		{
			buffer = arena_alloc(line_size * height,0,0, arena);
			if(pixmaps)
			{
				pixmaps[i] = XCreatePixmap(wd->display, wd->window, width, height, depth);
			}
		}
		else
		{
			XShmSegmentInfo shm_info = {0};
			shm_info.shmid = shmget(IPC_PRIVATE, line_size * height, IPC_CREAT | 0777);
			if(shm_info.shmid < 0)
			{
				RUNTIME_ABORT("Cannot get shared memory id for xlib pixmap\n");
			}
			shm_info.shmaddr = shmat(shm_info.shmid, 0,0);
			if(shm_info.shmaddr== (char *)-1)
			{
				RUNTIME_ABORT("Cannot get shared memory for xlib pixmap\n");
			}
			print("%u64 %s32\n", shm_info.shmaddr, shm_info.shmid);
			shm_info.readOnly = false;

			if(pixmaps)
			{
				buffer = shm_info.shmaddr;
				pixmaps[i] = XShmCreatePixmap(wd->display, wd->window, shm_info.shmaddr, &shm_info, width, height, depth);
				if(!XShmAttach(wd->display, &shm_info))
				{
					RUNTIME_ABORT("Cannot attach XShm memory!");
				}
			}
			else
			{
				RUNTIME_ABORT("XShm s not supported but Xpresent is.");
			}


			shm_infos[i] = shm_info;



		}


		images[i] = initialize_image(buffer, 1, PIXEL_FORMAT_B8G8R8A8, width, height);
		images[i].line_size = line_size;

		if(1)
		{
			clear_view(images[i].view, info.background_color);
		}
		else
		{
			if(i & 1)
			{
				clear_view(images[i].view, color32(0,0,0,0));
			}
			else
			{
				clear_view(images[i].view, color32(255, 255, 255, 255));
			}
		}
			

		if(info.old_swapchain)
		{
			Swapchain* old = info.old_swapchain;
			u32 index = defmin(i, old->count - 1);
			copy_view(images[i].view, old->images[index].view);
		}
		PixelFormat format = PIXEL_FORMAT_B8G8R8A8;
		u32 pixel_size = pixel_format_sizes[format];
		if(ximages)
		{
			ximages[i] = ximage;
			ximages[i]->data = (void*)buffer;
		}
		else
		{
			XDestroyImage(ximage);
		}
	}

	X11WindowSwapchain *ws = arena_alloc(sizeof(X11WindowSwapchain), 0,0,arena);

	Swapchain swapchain = {
		.data = ws,
		.old_data = 0,
		.images = images,
		.index = 0,
		.accum = 0,
		.count = image_count,
	};


	ws[0] = (X11WindowSwapchain){
		.driver_data = driver_data,
		.gc = gc,
		.ximages = ximages,
		.width = width,
		.height = height,
		.image_count = image_count,
		.pixmaps = pixmaps,
		.shm_infos = shm_infos,
		.swapchain = swapchain,

	};
	return &ws->swapchain;
}

void destroy_window_swapchain_x11(void *swapchain)
{
	X11WindowSwapchain *ws = ((Swapchain*)swapchain)->data;
	X11WindowDriver *wd = ws->driver_data;
	if(ws->pixmaps)
	{
		for(u32 i = 0; i < ws->image_count; i++)
		{
			if(ws->shm_infos)
			{
				XShmDetach(wd->display, &ws->shm_infos[i]);
				shmdt(ws->shm_infos[i].shmaddr);
				shmctl(ws->shm_infos[i].shmid, IPC_RMID, NULL);
			}
			XFreePixmap(wd->display, ws->pixmaps[i]);
		}
	}
	if(ws->ximages)
	{
		for(u32 i = 0; i < ws->image_count; i++)
		{
			ws->ximages[i]->data = 0;
			XDestroyImage(ws->ximages[i]);
		}
	}
	XFreeGC(wd->display, ws->gc);
}


void present_window_swapchain_x11(void *swapchain) 
{
	X11WindowSwapchain *ws = ((Swapchain*)swapchain)->data;
	X11WindowDriver *wd = ws->driver_data;


	u32 last_frame_index = ws->swapchain.index;
	ws->swapchain.accum ++;
	ws->swapchain.index = (ws->swapchain.index + 1) % ws->swapchain.count;


	if(ws->pixmaps)
	{
		Pixmap pixmap = ws->pixmaps[last_frame_index];
		if(ws->ximages && wd->shared_memory_supported == false)
		{
			XImage* ximage = ws->ximages[last_frame_index];
			XPutImage(wd->display, pixmap, ws->gc, ximage, 0,0,0,0, ximage->width, ximage->height);
		}
		XPresentPixmap(wd->display, wd->window, pixmap, 0, None, None, 
		0,0,	// Offsets
		None,
		None,	// Wait fence
		None,	// idle fence
		PresentOptionCopy, 
		0,1,0,NULL, 0);
	}
	else
	{
		XImage* ximage = ws->ximages[last_frame_index];
		XPutImage(wd->display, wd->window, ws->gc, ximage, 0,0,0,0, ximage->width, ximage->height);
		XSync(wd->display, False);
	}
}

typedef struct{
	
	Swapchain swapchain;
}X11SharedWindowSwapchain;

void* create_window_swapchain_x11_shared(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena)
{
	X11SharedWindowSwapchain *wf = arena_alloc(sizeof(X11SharedWindowSwapchain),0,0, arena);
	RUNTIME_ABORT("create_window_frameburffer_x11_shared() is EMPTY!");
	return &wf->swapchain;
}

void destroy_window_swapchain_x11_shared(void *swapchain)
{
	X11SharedWindowSwapchain *wd = ((Swapchain*)swapchain)->data;
	RUNTIME_ABORT("destroy_window_frameburffer_x11_shared() is EMPTY!");
}


void present_window_swapchain_x11_shared(void *swapchain)
{
	X11SharedWindowSwapchain *ws = ((Swapchain*)swapchain)->data;
	RUNTIME_ABORT("present_window_frameburffer_x11_shared() is EMPTY!");
}

//===================================================================================================================================================================================
#else //===================================================================================================================================================================================
//===================================================================================================================================================================================



#endif // OS_LINUX

