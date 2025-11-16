#pragma once

#include "device_texture.h"
#include "device_vertex_buffer.h"
#include "camera2.h"
#include "device_font.h"

typedef struct{
	fmat3p affine;
	fvec4 color;
	u32 time;
	u32 index; 
	f32 scale;
}DeviceRendererPushRange;

typedef struct{
	VkSampleCountFlags desired_sample_count;
}DeviceRendererCreateInfo;

typedef struct{

	Device *device;
	VkFormat target_format;
	VkSampleCountFlags target_sample_count;
	VkImageUsageFlags target_image_usage;
	VkSampler vertex2_color_sampler;
	VkSampler vertex2_mono_sampler;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;

	VkPipelineLayout pipeline_layout;
	RenderPass render_pass;

	VkPipeline vertex2_pipeline;
	VkPipeline vertex2_wireframe_pipeline;

	VkPipeline boid_pipeline;
	VkPipeline boid_wireframe_pipeline;


// Shared Data

	DeviceImage2D texture_image;
	DeviceImageView2D texture_image_view;

	DeviceImage2D glyph_image;
	DeviceImageView2D glyph_image_view;

	SimpleFont simple_font;

	VkDescriptorSet *descriptor_sets;

	DeviceVertexBuffer *ui_vertex_buffers;
	DeviceVertexBuffer *overlay_vertex_buffers;
	DeviceVertexBuffer *world_vertex_buffers;

	Camera2 world_camera;
	Camera2 overlay_camera; // Will not zoom
	Camera2 blit_camera;

// Frame Data

	u32 frame_count;
	uvec2 frame_size;
	u32 debug_index;
	b32 show_wireframe;

	DeviceImage2D *target_images;
	DeviceImageView2D *target_image_views;
	DeviceImage2D *target_msaa_images;
	DeviceImageView2D *target_msaa_image_views;
	Framebuffer *framebuffers;


	b32 blit_camera_active;

	DeviceRendererCreateInfo create_info;



}DeviceRenderer;


typedef struct{
	b32 initialized;
	VkSampleCountFlags sample_counts;	
}DeviceRendererLimits;


extern DeviceRendererLimits device_renderer_limits;


DeviceRenderer create_device_renderer(Device *device, DeviceRendererCreateInfo *create_info, u32 frame_count, uvec2 frame_size, Arena* resize_arena, Arena *arena);
void destroy_device_renderer(DeviceRenderer r);
void resize_renderer_create(DeviceRenderer *r, uvec2 frame_size,Arena *resize_arena);
void resize_renderer_destroy(DeviceRenderer *r);
void cmd_device_renderer_blit(DeviceCommandBuffer cb, DeviceRenderer dr, u32 frame_index, DeviceImage2D dst);
b32 poll_device_renderer(DeviceRenderer *dr, FrameEvents fe);
void recreate_device_renderer(DeviceRenderer *dr, u32 frame_count, uvec2 frame_size, Arena *resize_arena);
