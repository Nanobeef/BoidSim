#pragma once
#include "basic.h"

static u32 pixel_format_sizes[] = {
	0, 4,4,
};

typedef enum{
	PIXEL_FORMAT_NONE = 0,
	PIXEL_FORMAT_R8G8B8A8,
	PIXEL_FORMAT_B8G8R8A8,
}PixelFormatFlags;
typedef u32 PixelFormat;

typedef union{
	struct{
		u32 all;
	};
	struct{
		u8 b,g,r,a;
	};
	u8 arr[4];
}Color32 align(4);


static Color32 random_color32(PRNG *rg)
{
	u32 r = (u32)random_u64(rg);
	Color32 c;
	memcpy(&c, &r, sizeof(u32));
	return c;
}


static Color32 color32(u8 r, u8 g, u8 b, u8 a)
{
	return (Color32){.r = r, .g = g, .b = b,.a = a};
}

static Color32* set_pixel(Color32* dst, const Color32 *src)
{
	// Software remapping.
	*dst = *src;
	return dst;
}

static Color32 color32_add(Color32 ca, Color32 cb)
{
	u16 r,g,b,a;	
	r = (u32)ca.r + (u32)cb.r;
	g = (u32)ca.g + (u32)cb.g;
	b = (u32)ca.b + (u32)cb.b;
	a = (u32)ca.a + (u32)cb.a;
	if(r > 255){r=255;}
	if(g > 255){g=255;}
	if(b > 255){b=255;}
	if(a > 255){a=255;}
	return color32(r,g,b,a);
}




