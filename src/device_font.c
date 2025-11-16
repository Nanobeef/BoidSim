#include "device_font.h"
#include "old_font.h"
#include "embed.h"



SimpleFont create_simple_font(DeviceArena* device_arena, DeviceImage2D image, u32 font_size, Arena* arena)
{
	SimpleFont ret = {
		.image = image,
	};

	Temp temp = begin_temp(arena);
	const void *face_data_start = _binary_bin_default_mono_ttf_start;
	u64 face_data_size = (u64)(_binary_bin_default_mono_ttf_end - _binary_bin_default_mono_ttf_start);
	OldGlyphGenerator* gen = create_old_glyph_generator(DEFAULT_FONT_PATH,face_data_size, face_data_start, temp.arena);
	LinmaxRectanglePack pack = linmax_rectangle_pack_init(image.size.x, image.size.y);
	const u32 glyph_count = 126 - 33;
	OldGlyph glyphs[glyph_count]; 
	old_glyph_generator_set_size(gen, font_size);
	ret.max_box = gen->font_size;
	ret.line_gap = gen->ascender - gen->descender;

	u64 total_size = 0;
	for(u32 i = 0; i < glyph_count; i++)
	{
		load_old_glyph(gen, (s32)i + 33, temp.arena);
		glyphs[i] = gen->glyph;
		total_size += glyphs[i].width * glyphs[i].height;
	}
	total_size *= 4;
	Device* device = device_arena->device;
	{
		VkBufferImageCopy regions[glyph_count];
		DeviceBuffer buffer = allocate_device_buffer(device_arena, total_size, DEVICE_MEMORY_TYPE_HOST_CACHED, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, temp.arena);
		u64 pos = 0;
		for(u32 i = 0; i < glyph_count; i++)
		{
			u64 size = glyphs[i].width * glyphs[i].height;
			u8* dst = buffer.memory.mapping;
			for(u64 j = 0; j < size; j++)
			{
				u8* d = dst + pos;
				u8 color = glyphs[i].bitmap[j];

				d[j] = color;
			}
			uvec2 bsize = uvec2_make(glyphs[i].width, glyphs[i].height);
			u32 padding = 8;
			Rectangle rect = linmax_rectangle_pack(&pack, bsize.x+padding, bsize.y+padding);
			rect.e -= padding;
			rect.s -= padding;
			ret.ascii_glyphs[i].rectangle = (TextureRectangle){
				.i = texture_index_mono,
				.x0 = rect.w,
				.x1 = rect.e,
				.y0 = rect.n,
				.y1 = rect.s,
			};
			ret.ascii_glyphs[i].advance = (glyphs[i].horizontal_advance);
			ret.ascii_glyphs[i].bearing = svec2_make(glyphs[i].horizontal_bearing_x, glyphs[i].horizontal_bearing_y);
			uvec2 bpos = uvec2_make(rect.w, rect.n);
			regions[i] = (VkBufferImageCopy){
				.bufferOffset = pos,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
				.imageOffset = {bpos.x, bpos.y, 0},
				.imageExtent = {bsize.x, bsize.y, 1},
			};
			pos += size;
		}


		TransientDeviceCommandBuffer tcb = begin_transient_device_command_buffer(device->main_queue_family->queues[0]);
		cmd_transition_device_image2d(tcb.cb, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		cmd_clear_color_device_image2d(tcb.cb, image, fvec4_zero());
		vkCmdCopyBufferToImage(tcb.cb.handle, buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, glyph_count, regions);
		end_transient_device_command_buffer(tcb);
		destroy_old_glyph_generator(gen);
		wait_transient_device_command_buffer(tcb);
		free_device_buffer(buffer);
	}
	{

		TransientDeviceCommandBuffer tcb = begin_transient_device_command_buffer(device->main_queue_family->queues[0]);
		cmd_transition_device_image2d(tcb.cb, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		end_transient_device_command_buffer(tcb);
		wait_transient_device_command_buffer(tcb);

	}
	end_temp(temp);
	return ret;
}

