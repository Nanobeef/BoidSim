#include "image.h"

void clear_view(View view, Color32 color)
{
	for(u64 y = 0; y < view.height; y++)
	{
		Color32* line = view.buffer + (view.parent_line_size * (view.y + y));
		for(u32 x = 0; x < view.width; x++)
		{
			line[x + view.x] = color;	
		}
	}
}
	

void copy_view(View dst, View src)
{
	u32 width = defmin(dst.width, src.width);
	u32 height = defmin(dst.height, src.height);
	
	for(u64 y = 0; y < height; y++)
	{
		Color32* dst_line = view_line(dst, y);
		Color32* src_line = view_line(src, y);
		memcpy(dst_line, src_line, src.pixel_size * width);
	}
}

Image initialize_image(void *buffer, u32 line_alignment, PixelFormat format, u32 width, u32 height)
{
	RUNTIME_ASSERT(format == PIXEL_FORMAT_B8G8R8A8, "Only R8G8B8A8 is supported, for now.");

	u64 pixel_size = pixel_format_sizes[format];
	
	u64 line_size =  forward_align_uint(pixel_size * width, line_alignment);

	Image image = {
		.buffer = buffer,
		.line_size = line_size,
		.format = format,
		.pixel_size = pixel_size,
		.width = width,
		.height = height,
		.size = line_size * height,
	};
	image.view = initialize_view(image, 0,0, width, height);
	return image;
}

Image allocate_image(PixelFormat format, u32 width, u32 height, Arena *arena)
{
	Image image = initialize_image(0, 64, format, width, height);
	image.buffer = arena_alloc_aligned(image.size, arena->page_size, arena);
	return image;
}

View initialize_view(Image image, u32 x, u32 y, u32 width, u32 height)
{
	View view = {
		.buffer = image.buffer,
		.parent_width = image.width,
		.parent_height = image.height,
		.parent_line_size = image.line_size,
		.width = width,
		.height = height,
		.x = x,
		.y = y,
		.format = image.format,
		.pixel_size = image.pixel_size,
		.x_size = image.pixel_size * x,
	};
	return view;
}
