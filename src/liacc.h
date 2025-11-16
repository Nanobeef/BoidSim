#pragma once

#include "image.h"

// Light accumulatoror

typedef struct{
	u32 buffer[256 * 256];
}LiaccSection;


typedef struct{
	u32 section_count_x, section_count_y;
	LiaccSection *sections;	

	u32 width, height;
	u32 requested_width, requested_height;

	PRNG rg;
	Color32 color;
}Liacc;

typedef struct{
	u16 x0,y0,x1,y1;		
}LightSegment;

Liacc alloc_liacc(u32 width, u32 height, Arena *arena);

u32 test_liacc(Liacc *liacc);

void draw_liacc(View view, Liacc liacc);


