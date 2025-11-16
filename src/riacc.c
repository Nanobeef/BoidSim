#include "riacc.h"




Riacc create_riacc(u32 width, u32 height, Arena *arena)
{
	u32 starting_bucket_capacity = KB * 8;
	width = defmax(256, width);
	height= defmax(256, height);
	u32 buckets_width = uint_next_power_of_two(width) >> 8;
	u32 buckets_height = uint_next_power_of_two(height) >> 8;

	u32 bucket_count = buckets_width * buckets_height;


	RiaccBucket *buckets = arena_alloc(sizeof(RiaccBucket) * bucket_count, 0,0,arena);


	Riacc riacc = {
		.buckets_width = buckets_width,
		.buckets_height = buckets_height,
		.width_shift = uint_trailing_zeros(buckets_width),
		.height_shift = uint_trailing_zeros(buckets_height),
		.new_bucket_capacity = starting_bucket_capacity,
		.buckets = buckets,
	};

		for(u32 i = 0; i < bucket_count; i++)
		{
			RiaccLine *lines = arena_alloc(sizeof(RiaccLine) * riacc.new_bucket_capacity,0,0, arena);
			riacc.buckets[i] = (RiaccBucket){
				.count = 0,
				.lines = lines,
			};
		}
	return riacc;
}

#include <math.h>


// Amanatides Woo : http://www.cse.yorku.ca/~amana/research/grid.pdf

void riacc_insert_line(Riacc riacc, u32 x, u32 y, u32 x0, u32 x1, u32 y0, u32 y1)
{
	u32 index = y *riacc.buckets_width+x;
	RiaccLine line = {x0,x1,y0,y1};
	riacc.buckets[index].lines[riacc.buckets[index].count] = line;
	riacc.buckets[index].count++;
}


void riacc_ray(Riacc riacc, u32 x0, u32 x1, u32 y0, u32 y1)
{
	f32 ox = (f32)x0/256;
	f32 oy = (f32)y0/256;

	s32 fx = riacc.buckets_width;
	s32 fy = riacc.buckets_height;

	f32 dx = ((f32)x1-(f32)x0)/256;
	f32 dy = ((f32)y1-(f32)y0)/256;

	u32 nx = riacc.buckets_width;
	u32 ny = riacc.buckets_height;

	f32 tmx = INFINITY;
	f32 tmy = INFINITY;

	f32 tdx = 0;
	f32 tdy = 0;

	s32 sx = 0;
	s32 sy = 0;

	s32 x = (s32)floor(ox);
	s32 y = (s32)floor(oy);

	if(dx != 0){
		
		f32 inv_dx = 1.0 / dx;

		float bound;
		if(dx>0)
		{
			sx = 1;
			bound = x+1;
			tdx = inv_dx;
			fx = (x1>>8)+1;
		}
		else
		{
			sx = -1;
			bound = x;
			tdx = -inv_dx;
			fx = (x1>>8)-1;
		}

		tmx = (bound-ox)*inv_dx;
	}

	if(dy != 0){
		
		f32 inv_dy = 1.0 / dy;

		float bound;
		if(dy>0)
		{
			sy = 1;
			bound = y+1;
			tdy = inv_dy;
			fy = (y1>>8)+1;

		}
		else
		{
			fy = (y1>>8)-1;
			sy = -1;
			bound = y;
			tdy = -inv_dy;
		}

		tmy = (bound-oy)*inv_dy;
	}

	f32 current = 0.0;

	while(true)
	{
		f32 exit = fmin(tmx, tmy);

		u32 xentry = (u32)((ox + current * dx)*256)&255;
		u32 yentry = (u32)((oy + current * dy)*256)&255;
		u32 xexit = (u32)((ox + exit * dx)*256)&255;
		u32 yexit = (u32)((oy + exit * dy)*256)&255;

		print("%u32,\t%,\t%,\t%\n", xentry, yentry, xexit, yexit);
		current = exit;
		riacc_insert_line(riacc, x,y, xentry,xexit,yentry,yexit);
		
		if(tmx < tmy)
		{
			x+=sx;
			if(x == fx)
			{
				break;
			}
			tmx += tdx;
		}
		else
		{
			y+=sy;
			if(y == fy)
			{
				break;
			}
			tmy += tdy;
		}
	}

}

// EFLA: http://www.edepot.com/lined.html

void myLine(u32* acc, int x, int y, int x2, int y2) {
	b32 yLonger=false;
	int incrementVal, endVal;
	int shortLen=y2-y;
	int longLen=x2-x;
	if (abs(shortLen)>abs(longLen)) {
		int swap=shortLen;
		shortLen=longLen;
		longLen=swap;
		yLonger=true;
	}
	endVal=longLen;
	if (longLen<0) {
		incrementVal=-1;
		longLen=-longLen;
	} else incrementVal=1;
	int decInc;
	if (longLen==0) decInc=0;
	else decInc = (shortLen << 16) / longLen;
	int j=0;
	if (yLonger) {	
		for (int i=0;i!=endVal;i+=incrementVal) {
			acc[(x+(j >> 16))+(y+i)*256] = U32_MAX;
			j+=decInc;
		}
	} else {
		for (int i=0;i!=endVal;i+=incrementVal) {
			acc[(x+i) + (y+(j >> 16))*256] = U32_MAX;
			j+=decInc;
		}
	}


}

u32 *resolve_riacc_bucket(RiaccBucket bucket, Arena *arena)
{
	u32 *ret = arena_alloc(sizeof(u32) * 256 * 256,0,0, arena);
	memset(ret, 0, sizeof(u32) * 256 * 256);

	for(u32 i = 0; i < bucket.count; i++)
	{
		//resolve_line(ret, bucket.lines[i]);
		RiaccLine l = bucket.lines[i];
		myLine(ret,l.x0,l.y0, l.x1,l.y1);
	}
	return ret;
}

void reset_riacc(Riacc riacc)
{
	for(u32 i = 0; i < riacc.buckets_width * riacc.buckets_height ; i++)
	{
		riacc.buckets[i].count = 0;
	}
}

void draw_riacc(Riacc riacc, View view)
{

	Color32 color = color32(255,255,255,255);

	for(u32 y = 0; y < riacc.buckets_height; y++)
	{
		for(u32 x = 0; x < riacc.buckets_width; x++)
		{
			Temp temp = begin_temp(0);
			u32 *accumulator = resolve_riacc_bucket(riacc.buckets[x + y*riacc.buckets_width],temp.arena);

			for(u32 yy = 0; yy < 256; yy++)
			{
				void *line = view_line(view, y*256+yy);
				for(u32 xx = 0; xx < 256; xx++)
				{
					if(((x * 256 + xx) < view.width) && ((y * 256  + yy) < view.height))
					{
						u32 s = accumulator[yy * 256 + xx];
						s >>=24;
						Color32 dst_color = color32(
							(s * color.r) >> 8,
							(s * color.g) >> 8,
							(s * color.b) >> 8,
							(s * color.a) >> 8
						);
						Color32 *dst = view_pixel(view, line, x*256+xx);
						*dst = dst_color;
					}
				}
			}
			end_temp(temp);
		}
	}
}

