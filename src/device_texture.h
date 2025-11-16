#pragma once

#include "vk.h"
#include "device_geometry.h"

typedef struct{
	Device* device;
	u32 image_capacity;
	u32 image_count;
	VkFormat image_format;
	DeviceImageView2D* views;
	u32 default_view_index;
	u8* free_indices;
}TextureMap;



TextureMap create_texture_map(Device* device, VkFormat format, u32 capacity, Arena* arena);
void destroy_texture_map(TextureMap texture);

void texture_map_remove_image(TextureMap* map, u32 index);
TextureCoordinate texture_map_allocate_image(TextureMap* texture_map, DeviceArena* device_arena, uvec2 size, Arena* arena);
TextureCoordinate texture_map_import_image(TextureMap* texture_map, DeviceImage2D image);

void cmd_blit_entire_texture_map(DeviceCommandBuffer cb, TextureMap texture_map, DeviceImage2D dst);
void cmd_transition_texture_map(DeviceCommandBuffer cb, TextureMap texture_map, VkImageLayout src_layout, VkImageLayout dst_layout, VkAccessFlags src_access, VkAccessFlags dst_access, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage);
