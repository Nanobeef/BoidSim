#pragma once

#include "color.h"

typedef struct{
	void* buffer;

	u32 parent_width, parent_height;
	u64 parent_line_size;
	
	u32 width, height;	

	u32 x, y;	


	PixelFormat format;
	u32 pixel_size;
	u32 x_size;

	u64 size;
}View;

#define view_line( VIEW , Y) ((VIEW.buffer + VIEW.parent_line_size * (VIEW.y + (Y))) + (VIEW.x * VIEW.pixel_size))
#define view_pixel( VIEW, LINE, X) ((LINE) + (X) * (VIEW.pixel_size))

void clear_view(View view, Color32 color);
void copy_view(View dst, View src);

typedef struct{
	void *buffer;
	
	u32 width, height;	

	u32 pixel_size;
	u32 line_size;
	PixelFormat format;


	u64 size;

	View view; 
}Image;

Image initialize_image(void *buffer, u32 line_alignment, PixelFormat format, u32 width, u32 height);

Image allocate_image(PixelFormat format, u32 width, u32 height, Arena *arena);

View initialize_view(Image image, u32 x, u32 y, u32 width, u32 height); 



typedef struct{
	void *data;
	void *old_data;
	Image *images;
	u32 index;
	u64 accum;
	u32 count;
	b32 can_sync;
}Swapchain;















