#pragma once
#include "alg.h"


typedef enum{
	EVENT_NONE,
	EVENT_WINDOW,	
	EVENT_PROCESS,
	EVENT_KEYBOARD,
	EVENT_MOUSE,
	EVENT_SET_CURSOR,
}EventTypeFlags;
typedef u32 EventType;

typedef enum{
	WINDOW_NONE = 0,
	WINDOW_RESIZE,
	WINDOW_MOVE,
}WindowEventTypeFlags;
typedef u32 WindowEventType;

typedef struct{
	WindowEventType type;
	struct{ s32 x,y; };
	struct{ u32 width, height;};
}WindowEvent;

typedef enum{
	KEYBOARD_NONE = 0,
	KEYBOARD_PRESS,	
	KEYBOARD_RELEASE,	
}KeyboardEventTypeFlags;
typedef u32 KeyboardEventType;


typedef enum{
	MOUSE_NONE,	
	MOUSE_PRESS,
	MOUSE_RELEASE,
	MOUSE_SCROLL,
	MOUSE_MOVE,
	MOUSE_ENTER,
	MOUSE_LEAVE,
}MouseEventTypeFlags;
typedef u32 MouseEventType;

typedef enum{
	MOUSE_BUTTON_NONE,
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_BACK,
	MOUSE_BUTTON_FORWARD,
}MouseButtonTypeFlags;
typedef u32 MouseButtonType;

typedef struct{
	MouseEventType type;
	union{
		struct {s32 x,y;};
		struct {s32 scroll;};
		MouseButtonType button;
	};
}MouseEvent;



typedef enum{
	KEY_NONE = 0,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_LEFT_CONTROL,
	KEY_RIGHT_CONTROL,
	KEY_LEFT_SHIFT,
	KEY_RIGHT_SHIFT,
	KEY_GRAVE,
	KEY_ESCAPE,
}KeyTypeFlags;
typedef u32 KeyType;

typedef struct{
	KeyboardEventType type;
	KeyType key;
}KeyboardEvent;

typedef enum{
	CURSOR_POINTER = 0,
	CURSOR_HAND,
	CURSOR_X11_GOBBLER,
	CURSOR_X11_GUMBY,
	CURSOR_X11_BOAT,
	CURSOR_FLEUR,
	CURSOR_CIRCLE,
	CURSOR_CROSS,
}CursorTypeFlags;
typedef u32 CursorType;

typedef struct{
	CursorType type;	
	fvec4 background_color;
	fvec4 foreground_color;
}CursorSetEvent;

typedef struct{
	EventType type;	
	union{
		WindowEvent	window;
		KeyboardEvent keyboard;
		MouseEvent mouse;
		CursorSetEvent set_cursor;
	};
	void *window_pointer;
	u64 time;
}Event;


Event init_event();
void post_event(Event *event_ring_buffer, Event event);
void print_event(Event event);

typedef struct{
	b32 pressed;
	u64 press_time;
	u64 release_time;
	u64 action_time;
}Button;

typedef struct{
	u64 mouse_move_time;
	union{
		svec2 mouse_position;
		struct {s32 mouse_x, mouse_y;};
	};
	union{
		uvec2 mouse_delta;
		struct {s32 mouse_dx, mouse_dy;};
	};
	union{
		uvec2 window_position;
		struct {s32 window_x, window_y;};
	};
	union{
		uvec2 window_size;
		struct {u32 window_width, window_height;};
	};

	fvec2 norm_mouse;
	fvec2 norm_mouse_delta;

	b32 window_resized;
	b32 window_moved;

	s32 mouse_scroll;
	b32 mouse_moved;

	Button first_button;

	Button mouse_left, mouse_right, mouse_middle;
	Button escape;
	Button grave;

	Button a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;
	Button n0,n1,n2,n3,n4,n5,n6,n7,n8,n9;
	Button f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12;
	Button left_control;
	Button left_shift;

	Button last_button;

	// TODO: left_control, left_shify, 'press_time'


	u64 dt;
	u64 time;

	f32 screen_refresh_rate;
	u32 dpi;

	Event *event_ring_buffer;
}FrameEvents;


// Window size update order:
// *.) Native Event Queue
// 1.) Window Driver
// *.) Event Resolve
// 2.) Window
// *.) (fe.window_resized == true)
// 3.) Window Framebuffer

// TODO: Support more than one window: Need to add support for more than one frame events output and more than one event ring buffer.
	
FrameEvents resolve_frame_events(FrameEvents last, Event *event_ring_buffer, Arena* frame_arena);











