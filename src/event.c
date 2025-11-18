#include "event.h"
#include "window.h"

extern b32 global_keep_terminal_open;

Event init_event()
{
	Event e = {
	};
	return e;
}

void post_event(Event *event_ring_buffer, Event e)
{
	e.time = get_time_ns();
	ring_buffer_push(event_ring_buffer, e);
}

void print_event(Event event)
{
	u64 time = get_time_ns() - event.time;
	switch(event.type)
	{
		case EVENT_WINDOW:
		{
			printf("Window ");
			if(event.window.type == WINDOW_RESIZE)
			{
				printf("Resize (%u, %u)", event.window.width, event.window.height);
			}
			if(event.window.type == WINDOW_MOVE)
			{
				printf("Move   (%d, %d)", event.window.x, event.window.y);
			}
		}
		break;
		case EVENT_KEYBOARD:
		{
			printf("Keyboard ");
			if(event.keyboard.type == KEYBOARD_PRESS)
			{
				printf("Press    ");
			}
			if(event.keyboard.type == KEYBOARD_RELEASE)
			{
				
				printf("Release  ");
			}
			printf("%u\t", event.keyboard.key);
		}
		break;
		case EVENT_MOUSE:
		{
			printf("Mouse ");	
			switch(event.mouse.type)
			{
				case MOUSE_ENTER: 
				{
					printf("Enter   "); 
				}
				break;
				case MOUSE_LEAVE: 
				{
					printf("Leave   ");
					printf(" (%d %d)", event.mouse.x, event.mouse.y);
				}
				break;
				case MOUSE_MOVE:  
				{
					printf("Move    ");
					printf(" (%d, %d)", event.mouse.x, event.mouse.y);
				}
				break;
				case MOUSE_PRESS: 
				{
					printf("Press   ");
					printf(" %u", event.mouse.button);
				}
				break;
				case MOUSE_RELEASE:
				{
					printf("Release ");
					printf(" %u", event.mouse.button);
				}
				break;
				case MOUSE_SCROLL:
				{
					printf("Scroll  ");
					printf(" %d\t", event.mouse.scroll);
				}
				break;
				default:
				break;
			}

		}
		break;
		default:
		printf("None");
	}
	printf(" (%lu us)\n", ns_to_us(time));
}

Button set_button(Button button, u64 time, b32 pressed)
{
	if(pressed && (button.pressed == false))
	{
		button.press_time = time;
	}
	else if(button.pressed)
	{
		button.release_time = time;	
	}
	button.action_time = time;
	button.pressed = pressed;
	return button;
}

FrameEvents resolve_frame_events(FrameEvents last, Event *event_ring_buffer, Arena* frame_arena)
{
	FrameEvents fe = last;
	fe.event_ring_buffer = event_ring_buffer;
	fe.mouse_scroll = 0;
	fe.mouse_dx = 0;
	fe.mouse_dy = 0;
	fe.mouse_moved = false;
	fe.window_resized = false;
	fe.dt = get_time_ns() - fe.time;
	fe.time = get_time_ns();
	Event e = {0};
	while(ring_buffer_pop(event_ring_buffer, &e))
	{
		switch(e.type)
		{
			case EVENT_WINDOW:
			{
				switch(e.window.type)
				{
					case WINDOW_RESIZE:
					{
						fe.window_resized = true;
						Window* window = e.window_pointer;
						window->width = e.window.width;
						window->height = e.window.height;
					}
					break;

					case WINDOW_MOVE:
					{
						fe.window_moved= true;
						Window* window = e.window_pointer;
						window->x = e.window.x;
						window->y = e.window.y;
					}
					break;
					default:
					break;
				}
			}
			break;
			case EVENT_KEYBOARD:
			{
				b32 pressed = (e.keyboard.type == KEYBOARD_PRESS);
				switch(e.keyboard.key)
				{
					case KEY_ESCAPE:
					{
						fe.escape = set_button(fe.escape, fe.time, pressed);
					}
					break;
					case KEY_GRAVE:
					{
						fe.grave = set_button(fe.grave, fe.time, pressed);
						global_keep_terminal_open = true;
					}
					case KEY_A: fe.a = set_button(fe.a, fe.time, pressed); break;
					case KEY_B: fe.b = set_button(fe.b, fe.time, pressed); break;
					case KEY_C: fe.c = set_button(fe.c, fe.time, pressed); break;
					case KEY_D: fe.d = set_button(fe.d, fe.time, pressed); break;
					case KEY_E: fe.e = set_button(fe.e, fe.time, pressed); break;
					case KEY_F: fe.f = set_button(fe.f, fe.time, pressed); break;
					case KEY_G: fe.g = set_button(fe.g, fe.time, pressed); break;
					case KEY_H: fe.h = set_button(fe.h, fe.time, pressed); break;
					case KEY_I: fe.i = set_button(fe.i, fe.time, pressed); break;
					case KEY_J: fe.j = set_button(fe.j, fe.time, pressed); break;
					case KEY_K: fe.k = set_button(fe.k, fe.time, pressed); break;
					case KEY_L: fe.l = set_button(fe.l, fe.time, pressed); break;
					case KEY_M: fe.m = set_button(fe.m, fe.time, pressed); break;
					case KEY_N: fe.n = set_button(fe.n, fe.time, pressed); break;
					case KEY_O: fe.o = set_button(fe.o, fe.time, pressed); break;
					case KEY_P: fe.p = set_button(fe.p, fe.time, pressed); break;
					case KEY_Q: fe.q = set_button(fe.q, fe.time, pressed); break;
					case KEY_R: fe.r = set_button(fe.r, fe.time, pressed); break;
					case KEY_S: fe.s = set_button(fe.s, fe.time, pressed); break;
					case KEY_T: fe.t = set_button(fe.t, fe.time, pressed); break;
					case KEY_U: fe.u = set_button(fe.u, fe.time, pressed); break;
					case KEY_V: fe.v = set_button(fe.v, fe.time, pressed); break;
					case KEY_W: fe.w = set_button(fe.w, fe.time, pressed); break;
					case KEY_X: fe.x = set_button(fe.x, fe.time, pressed); break;
					case KEY_Y: fe.y = set_button(fe.y, fe.time, pressed); break;
					case KEY_Z: fe.z = set_button(fe.z, fe.time, pressed); break;

					case KEY_0: fe.n0 = set_button(fe.n0, fe.time, pressed); break;
					case KEY_1: fe.n1 = set_button(fe.n1, fe.time, pressed); break;
					case KEY_2: fe.n2 = set_button(fe.n2, fe.time, pressed); break;
					case KEY_3: fe.n3 = set_button(fe.n3, fe.time, pressed); break;
					case KEY_4: fe.n4 = set_button(fe.n4, fe.time, pressed); break;
					case KEY_5: fe.n5 = set_button(fe.n5, fe.time, pressed); break;
					case KEY_6: fe.n6 = set_button(fe.n6, fe.time, pressed); break;
					case KEY_7: fe.n7 = set_button(fe.n7, fe.time, pressed); break;
					case KEY_8: fe.n8 = set_button(fe.n8, fe.time, pressed); break;
					case KEY_9: fe.n9 = set_button(fe.n9, fe.time, pressed); break;

					case KEY_F1: fe.f1 = set_button(fe.f1, fe.time, pressed); break;
					case KEY_F2: fe.f2 = set_button(fe.f2, fe.time, pressed); break;
					case KEY_F3: fe.f3 = set_button(fe.f3, fe.time, pressed); break;
					case KEY_F4: fe.f4 = set_button(fe.f4, fe.time, pressed); break;
					case KEY_F5: fe.f5 = set_button(fe.f5, fe.time, pressed); break;
					case KEY_F6: fe.f6 = set_button(fe.f6, fe.time, pressed); break;
					case KEY_F7: fe.f7 = set_button(fe.f7, fe.time, pressed); break;
					case KEY_F8: fe.f8 = set_button(fe.f8, fe.time, pressed); break;
					case KEY_F9: fe.f9 = set_button(fe.f9, fe.time, pressed); break;
					case KEY_F10: fe.f10 = set_button(fe.f10, fe.time, pressed); break;
					case KEY_F11: fe.f11 = set_button(fe.f11, fe.time, pressed); break;
					case KEY_F12: fe.f12 = set_button(fe.f12, fe.time, pressed); break;
					case KEY_LEFT_CONTROL: fe.left_control = set_button(fe.left_control, fe.time, pressed); break;
					default:
					break;
				}
			}
			case EVENT_MOUSE:
			{
				switch(e.mouse.type)
				{
					case MOUSE_SCROLL:
					{
						fe.mouse_scroll = e.mouse.scroll;
					}break;
					
					case MOUSE_MOVE:
					{
						fe.mouse_move_time = fe.time;
						fe.mouse_moved = true;
						if(fe.mouse_x || fe.mouse_y)
						{
							fe.mouse_dx += e.mouse.x - fe.mouse_x;
							fe.mouse_dy += e.mouse.y - fe.mouse_y;
						}
						fe.mouse_x = e.mouse.x;
						fe.mouse_y = e.mouse.y;

						fe.norm_mouse = fvec2_make(fe.mouse_x, fe.mouse_y);
						fe.norm_mouse = fvec2_div(fe.norm_mouse, fvec2_make(fe.window_width, fe.window_height));
						fe.norm_mouse = fvec2_make(fe.mouse_dx, fe.mouse_dy);
						fe.norm_mouse_delta = fvec2_div(fe.norm_mouse_delta, fvec2_make(fe.window_width, fe.window_height));

					}break;
					case MOUSE_PRESS:
					case MOUSE_RELEASE:
					{
						b32 pressed = (e.mouse.type == MOUSE_PRESS);
						switch(e.mouse.button)
						{
							case MOUSE_BUTTON_LEFT: fe.mouse_left = set_button(fe.mouse_left, fe.time, pressed); break;
							case MOUSE_BUTTON_RIGHT: fe.mouse_right = set_button(fe.mouse_right, fe.time, pressed); break;
							case MOUSE_BUTTON_MIDDLE: fe.mouse_middle = set_button(fe.mouse_middle, fe.time, pressed); break;

							default:
							break;
						}
					}break;

					default:
					break;
				}
			}
			break;
			default:
			break;
		}
	}
	return fe;
}
