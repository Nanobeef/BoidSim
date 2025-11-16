







// Example of software rasterization




#if 0
#include "window.h"

extern b32 global_close_window;
extern b32 global_keep_terminal_open;

#include "scanline.h"
#include <simde/x86/avx512.h>
	

#include "string.h"
#include "draw.h"

#include "riacc.h"
#include "math.h"


s32 main()
{
	main_init(GB * 64lu);

	const u32 frame_count = 2;
	const u32 resize_count = 2;
	u32 frame_index = 0;
	u64 frame_accum = 0;
	u64 target_time = 0;

	Arena *resize_arena_ring = allocate_ring(Arena, resize_count, &main_arena);
	Arena *frame_arena_ring = allocate_ring(Arena, frame_count, &main_arena);
	itterate_ring(resize_arena_ring, resize_arena_ring[i] = allocate_sub_arena(GB * 4lu, &main_arena));
	itterate_ring(frame_arena_ring, frame_arena_ring[i] = allocate_sub_arena(MB * 64, &main_arena));
	WindowProperties window_properties = query_best_window_properties(WINDOW_DRIVER_X11,WINDOW_SWAPCHAIN_X11,WINDOW_ACCELERATION_SOFTWARE);
	Event *event_ring_buffer = allocate_ring_buffer(Event, 1024, &main_arena);

	FrameEvents fe = {0};


	Window *window = create_window(window_properties, 1024, 1024, ring_current(resize_arena_ring), &main_arena);
	Swapchain *swapchain = window->swapchain_data;
	Font *font = create_font(0, 1024, 1024 * 1024, &main_arena);


	u64 loop_epoch_time = get_time_ns();
	u64 loop_epoch_accum = loop_epoch_time;

	while(fe.escape.pressed == false)
	{
		Arena *frame_arena = ring_current(frame_arena_ring);
		reset_arena(frame_arena);
		poll_window(window, event_ring_buffer);
		fe = resolve_frame_events(fe, event_ring_buffer, ring_current(resize_arena_ring));
		if(fe.window_resized)
		{
			Arena *last_resize_arena = ring_current(resize_arena_ring);
			Arena *resize_arena = ring_next(resize_arena_ring);
			swapchain = recreate_swapchain(window, resize_arena);
			reset_arena(last_resize_arena);
			print("resize\n");
		}

		View view = swapchain->images[swapchain->index].view;
		clear_view(view, color32(0, 0,0,0));

		triangle_test(window, font);

		u64 time = get_time_ns();
		if(time - loop_epoch_accum > 1000000000)
		{
			loop_epoch_accum = time;
		}



		ring_next(frame_arena_ring);
		frame_index = ring_index(frame_arena_ring);
		frame_accum++;
		present_swapchain(window, target_time);
		//print("Present: %u64 us\n", window->present_time / 1000);
		//print("Render:  %u64 ms\n", window->not_present_time / 1000);
	}

	main_cleanup();
	return 0;
}
#endif
