#pragma once
#include "window_properties.h"

#include "image.h"




	
typedef struct Window{
	const WindowDriverType driver_type;
	const WindowSwapchainType swapchain_type;
	void *driver_data;

	void *swapchain_data;
	void *vulkan_surface;

	LoopTime loop_time;
	u64 lifetime_frame_accum;

	u32 width, height;
	s32 x, y;

	u64 not_present_time;
	u64 present_time;
	u64 elapsed_time;
}Window;



Window *create_window(WindowProperties properties, u32 width, u32 height, Arena* resize_arena, Arena *arena);
void destroy_window(Window *window);
void poll_window(Window *window, Event *event_ring_buffer);
void present_swapchain(Window *window, u64 target_frame_time);
void* recreate_swapchain(Window *window, Arena* arena);




