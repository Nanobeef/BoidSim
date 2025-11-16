#include "window.h"

#include "window_properties.h"
#include "vk.h"


Window *create_window(WindowProperties properties, u32 width, u32 height, Arena* resize_arena, Arena *arena)
{
	Window *window = arena_alloc(sizeof(Window), 0,0,arena);
	{
		Window temp = {.driver_type = properties.driver, .swapchain_type = properties.swapchain};
		memcpy(window, &temp, sizeof(Window));
	}


	WindowDriverCreateInfo driver_create_info = {
		.width = width,
		.height = height,
		.x = 0,
		.y = 0,
		.window_pointer = window,
	};
	window->driver_data = create_window_driver_ppfn[window->driver_type](driver_create_info, arena);
	WindowSwapchainCreateInfo swapchain_create_info = {
		.background_color = color32(0,0,0,0),
		.desired_image_count = 2,
	};
	if(properties.device)
	{
		window->vulkan_surface = arena_alloc(sizeof(DeviceSurface),0,0, arena);	
		Device *device = properties.device;
		DeviceSurface surface = create_device_surface(device->instance,window->driver_type, window->driver_data);
		memcpy(window->vulkan_surface, &surface, sizeof(DeviceSurface));

	}
	else
	{
		window->swapchain_data = create_window_swapchain_ppfn[window->swapchain_type](swapchain_create_info, window->driver_data, resize_arena);
	}

	window->loop_time = loop_time_init(0);
	window->width = width;
	window->height = height;

	return window;
}

void destroy_window(Window *window)
{
	if(window->vulkan_surface == 0)
	{
		destroy_window_swapchain_ppfn[window->swapchain_type](window->swapchain_data);
	}
	destroy_window_driver_ppfn[window->driver_type](window->driver_data);
}
void poll_window(Window *window, Event *event_ring_buffer)
{
	poll_window_driver_ppfn[window->driver_type](window->driver_data, event_ring_buffer);	
}
void present_swapchain(Window *window, u64 target_frame_time)
{
	u64 present_time = get_time_ns();
	present_window_swapchain_ppfn[window->swapchain_type](window->swapchain_data);
	present_time = get_time_ns() - present_time;
	window->lifetime_frame_accum++;
	window->loop_time.target = target_frame_time;
	window->loop_time = loop_time_end(window->loop_time);
	window->loop_time = loop_time_start(window->loop_time);

	window->present_time = present_time;
	window->elapsed_time = window->loop_time.elapsed;
	if(window->elapsed_time > window->present_time)
	{
		window->not_present_time = window->elapsed_time - window->present_time;
	}
}
void *recreate_swapchain(Window *window, Arena* arena)
{
	WindowSwapchainCreateInfo swapchain_create_info = {
		.background_color = color32(0,0,0,0),
		.desired_image_count = 2,
		.old_swapchain = window->swapchain_data,
	};

	window->swapchain_data = create_window_swapchain_ppfn[window->swapchain_type](swapchain_create_info, window->driver_data, arena);
	destroy_window_swapchain_ppfn[window->swapchain_type](swapchain_create_info.old_swapchain);
	return window->swapchain_data;
}

