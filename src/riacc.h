#pragma once

#include "image.h"


typedef u32 RiaccAccumulator[256 * 256];


typedef struct{
	u8 x0,x1;
	u8 y0,y1;
}RiaccLine;

typedef struct RiaccBucket{
	RiaccLine *lines;
	u64 count;
}RiaccBucket;


typedef u32 RiaccBucketType;

typedef struct{
	u32 buckets_width, buckets_height;
	u32 width_shift, height_shift;
	u32 new_bucket_capacity;;
	RiaccBucket *buckets;
}Riacc;


Riacc create_riacc(u32 width, u32 height, Arena *arena);

void draw_riacc(Riacc riacc, View view);
void reset_riacc(Riacc riacc);
void riacc_ray(Riacc riacc, u32 x0, u32 x1, u32 y0, u32 y1);
