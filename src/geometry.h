#pragma once

#include "math.h"

typedef struct{
	u32 x : 15;				
	u32 y : 15;			
	u32 i : 2; 				
}TextureCoordinate;

typedef struct{ 
	u16 x0;
	u16 y0;
	u16 x1;
	u16 y1;
	u32 i;
}TextureRectangle;

typedef struct{
	fvec4 color;
	fvec2 position;
	TextureCoordinate texture;
}Vertex2;

static const u32 texture_index_disable = 3;
static const u32 texture_index_not_ready = 2;
static const u32 texture_index_mono = 1;
static const u32 texture_index_color = 0;

b32 point_vs_rectangle(fvec2 pos, fvec2 a, fvec2 b);
b32 point_vs_ellipse(fvec2 p, fvec2 c, fvec2 r);
b32 point_vs_rounded_rectangle(fvec2 pos, fvec2 a, fvec2 b, fvec2 radius);

TextureCoordinate texcord_disable();
TextureCoordinate texcord_not_ready();

typedef struct{
	u32 n,s,e,w;
}Rectangle;

static uvec2 rectangle_position(Rectangle rectangle)
{
	return uvec2_make(rectangle.w, rectangle.n);
}
static uvec2 rectangle_size(Rectangle rectangle)
{
	return uvec2_make(rectangle.e - rectangle.w, rectangle.s - rectangle.n);
}

typedef struct{
	u32 width, height;
	u32 horizontal_position;
	u32 vertical_position;
	u32 row_max_height;
	u32 x,y; // Just the offset
}LinmaxRectanglePack;

LinmaxRectanglePack linmax_rectangle_pack_init(u32 width, u32 height);
Rectangle linmax_rectangle_pack(LinmaxRectanglePack* p, u32 width, u32 height);
