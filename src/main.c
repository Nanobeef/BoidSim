#include "window.h"

extern b32 global_close_window;
extern b32 global_keep_terminal_open;

#include "ui.h"

#include "prefix_sum.h"

s32 main()
{
	main_init(GB * 64lu);
	const u32 frame_count = 2;
	const u32 resize_count = 2;
	u32 frame_index = 0;
	u64 frame_accum = 0;
	u64 target_time = 0;

	Arena *resize_arena_ring = allocate_ring(Arena, resize_count * 2, &main_arena);
	Arena *frame_arena_ring = allocate_ring(Arena, frame_count * 2, &main_arena);
	itterate_ring(resize_arena_ring, resize_arena_ring[i] = allocate_sub_arena(MB * 64lu, &main_arena));
	itterate_ring(frame_arena_ring, frame_arena_ring[i] = allocate_sub_arena(MB * 64lu, &main_arena));
	Arena *frame_arena = ring_current(frame_arena_ring);
	Arena *resize_arena = ring_current(resize_arena_ring);

	WindowProperties window_properties = query_best_window_properties(WINDOW_DRIVER_X11,WINDOW_SWAPCHAIN_X11,WINDOW_ACCELERATION_SOFTWARE);
	Event *event_ring_buffer = allocate_ring_buffer(Event, 1024, &main_arena);
	FrameEvents fe = {.event_ring_buffer = event_ring_buffer};
	Instance *instance = create_instance(&main_arena);
//	Device* device = create_device_by_type(instance, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, &main_arena);
	Device* device = create_device_by_type(instance, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, &main_arena);
	window_properties.device = device;
	Window *window = create_window(window_properties, 1024, 1024, ring_current(resize_arena_ring), &main_arena);
	DeviceSurface surface; memcpy(&surface, window->vulkan_surface, sizeof(DeviceSurface));


	DeviceSwapchain swapchain = create_device_swapchain(device, *(DeviceSurface*)window->vulkan_surface, 0, ring_current(resize_arena_ring));
	DeviceRenderer dr = create_device_renderer(device, 0, frame_count, swapchain.size, resize_arena, resize_arena);

	u64 loop_epoch_time = get_time_ns();
	u64 loop_epoch_accum = loop_epoch_time;
	LoopTime loop_time = loop_time_init(1000000000/144);


	DeviceFence *swapchain_fences = allocate_device_fences(device, frame_count, true, &main_arena);
	DeviceFence *graphics_fences = allocate_device_fences(device, frame_count, true, &main_arena);
	DeviceSemaphore *swapchain_semaphores = allocate_device_semaphores(device, frame_count, &main_arena);
	DeviceSemaphore *graphics_semaphores = allocate_device_semaphores(device, frame_count,  &main_arena);
	DeviceQueue graphics_queue = device->main_queue_family->queues[0];
	DeviceCommandPool **graphics_command_pools = arena_alloc(sizeof(DeviceCommandPool) * frame_count,0,0, &main_arena);
	for(u32 i = 0; i < frame_count; i++)
	{
		graphics_command_pools[i] = allocate_device_command_pool(device->main_queue_family, frame_arena, &main_arena);
	}
		
	BoidSim* boid_sim = create_boid_sim(device, 1024u * 1024u * 1u, 32, &main_arena);

	UI *boid_ui = init_ui_test();

	while(fe.escape.pressed == false)
	{
		loop_time = loop_time_end(loop_time);
		loop_time = loop_time_start(loop_time);
		THREAD->global.frame_time = loop_time.real_elapsed;
		poll_window(window, event_ring_buffer);
		fe = resolve_frame_events(fe, event_ring_buffer, ring_current(resize_arena_ring));

		if(fe.r.press_time == fe.time)
		{
			reset_boid_sim(boid_sim);
		}
		if(fe.mouse_right.pressed)
		{
			THREAD->global.boid.attractor_position = 
				fvec2_add(
					camera2_pixel_to_world(
						dr.world_camera, 
						fvec2_make(fe.mouse_x, fe.mouse_y)
					),
					fvec2_make(0.5, 0.5)
				);
			THREAD->global.boid.attractor_enable = true;
		}
		else
		{
			THREAD->global.boid.attractor_enable = false;
		}

		poll_ui_test(boid_ui, fe, swapchain.size, boid_sim);

		b32 should_recreate_renderer = poll_device_renderer(&dr, fe, !boid_ui->focused);


		if(fe.window_resized || should_recreate_renderer) 
		{
			Arena *last_resize_arena = ring_current(resize_arena_ring);
			resize_arena = ring_next(resize_arena_ring);

			wait_for_device_fences(frame_count, graphics_fences);

			recreate_device_swapchain(&swapchain, resize_arena);			
			recreate_device_renderer(&dr, frame_count, swapchain.size, resize_arena);
			reset_arena(last_resize_arena);
		}

		wait_and_reset_device_fence(swapchain_fences[frame_index]);
		{
			VkResult result = vkAcquireNextImageKHR(device->handle, swapchain.handle, U64_MAX, swapchain_semaphores[frame_index].handle, swapchain_fences[frame_index].handle, &swapchain.image_index);
			swapchain.next_image = swapchain.images[swapchain.image_index];
		}

		wait_and_reset_device_fence(graphics_fences[frame_index]);
		DeviceCommandPool* pool = graphics_command_pools[frame_index];
		reset_device_command_pool(pool, frame_arena);

		DeviceCommandBuffer cb = allocate_device_command_buffer(pool, false, frame_arena);
		begin_device_command_buffer(cb, 0);
		cmd_begin_render_pass_clear(cb, dr.framebuffers[frame_index], fvec4_make(0.0, 0.0, 0.0, 0.0));



		{
			vkCmdBindDescriptorSets(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, dr.pipeline_layout, 0, 1, &dr.descriptor_sets[frame_index], 0,0);
			cmd_set_pipeline_size(cb, swapchain.size);
			{
				DeviceRendererPushRange push_range = {
					.affine = fmat3_padding(dr.world_camera.affine),
					.color = fvec4_scalar_div(fvec4_make(255.0, 253.0, 208.0, 255.0) , 255.0),
					.time = get_time_ms(),
					.index = dr.debug_index,
					.scale = THREAD->global.boid.boid_size_norm,
				};
				vkCmdPushConstants(cb.handle, dr.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DeviceRendererPushRange), &push_range);
			}
			{
				// World
				VkPipeline boid_pipeline = dr.boid_pipeline;
				if(dr.show_wireframe)
				{
					boid_pipeline = dr.boid_wireframe_pipeline;
				}
				vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, boid_pipeline);

				cmd_draw_boid_sim_boids(cb, boid_sim);

			}
			if(fe.z.pressed){
				DeviceVertexBuffer *vb = &dr.world_vertex_buffers[frame_index];
				clear_vertex_buffer(vb);
				draw_boid_sim_grid(vb, dr.world_camera, dr.simple_font, boid_sim);
				VkPipeline vertex2_pipeline = dr.vertex2_pipeline;
				if(dr.show_wireframe)
				{
					vertex2_pipeline = dr.vertex2_wireframe_pipeline;
				}
				vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vertex2_pipeline);
				cmd_draw_vertex_buffer(cb, vb);
			}
			{
				DeviceRendererPushRange push_range = {
					.affine = fmat3_padding(dr.overlay_camera.affine),
					.color = fvec4_make(1.0, 0.0, 0.0, 1.0),
					.time = get_time_ms(),
					.index = dr.debug_index,
					.scale = 1.0f,
				};
				vkCmdPushConstants(cb.handle, dr.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DeviceRendererPushRange), &push_range);
			}
			{
				// Overlay
				DeviceVertexBuffer *vb = &dr.overlay_vertex_buffers[frame_index];
				clear_vertex_buffer(vb);

				//draw_boid_sim_overlay(vb, dr.overlay_camera, dr.simple_font, boid_sim);

				VkPipeline vertex2_pipeline = dr.vertex2_pipeline;
				if(dr.show_wireframe)
				{
					vertex2_pipeline = dr.vertex2_wireframe_pipeline;
				}
				vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vertex2_pipeline);
				cmd_draw_vertex_buffer(cb, vb);

			}
			{
				DeviceVertexBuffer *vb = &dr.ui_vertex_buffers[frame_index];
				clear_vertex_buffer(vb);
				draw_ui_test(boid_ui, vb, &dr.simple_font);
				{
					DeviceRendererPushRange push_range = {
						.affine = fmat3_padding(boid_ui->affine_matrix),
						.color = fvec4_make(1.0, 0.0, 0.0, 1.0),
						.time = get_time_ms(),
						.index = dr.debug_index,
						.scale = 1.0f,
					};
					vkCmdPushConstants(cb.handle, dr.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DeviceRendererPushRange), &push_range);
				}

				VkPipeline vertex2_pipeline = dr.vertex2_pipeline;
				if(dr.show_wireframe)
				{
					vertex2_pipeline = dr.vertex2_wireframe_pipeline;
				}
				vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vertex2_pipeline);
				cmd_draw_vertex_buffer(cb, vb);
			}

			cmd_end_render_pass(cb);
		}

		cmd_transition_device_image2d(cb, swapchain.next_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		cmd_device_renderer_blit(cb, dr, frame_index, swapchain.next_image);

		cmd_transition_device_image2d(cb, swapchain.next_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		end_device_command_buffer(cb);

		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
		submit_device_command_buffers(graphics_queue, 1, &cb, wait_stages, 1, &swapchain_semaphores[frame_index], 1, &graphics_semaphores[frame_index], graphics_fences[frame_index]);
		present_device_swapchain(graphics_queue, &swapchain, graphics_semaphores[frame_index]);

		ring_next(frame_arena_ring);
		frame_accum++;
		frame_index = frame_accum % frame_count;
		frame_arena = ring_current(frame_arena_ring);
		reset_arena(frame_arena);
	}

	device_wait_idle(device);

	destroy_boid_sim(boid_sim);

	destroy_device_renderer(dr);
	destroy_device_semaphores(frame_count, graphics_semaphores);
	destroy_device_semaphores(frame_count, swapchain_semaphores);
	destroy_device_fences(frame_count, graphics_fences);
	destroy_device_fences(frame_count, swapchain_fences);
	for(u32 i = 0; i < frame_count; i++)
	{
		free_device_command_pool(graphics_command_pools[i]);
	}
	destroy_device_swapchain(swapchain);
	destroy_device_surface(surface);
	destroy_window(window);
	destroy_device(device);
	destroy_instance(instance);
	main_cleanup();
	return 0;
}
