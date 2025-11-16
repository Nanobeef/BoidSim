#include "liacc.h"


Liacc alloc_liacc(u32 requested_width, u32 requested_height, Arena *arena)
{
	
	u32 width = forward_align_uint(requested_width, 256);
	u32 height = forward_align_uint(requested_height, 256);

	u32 section_count_x = width >> 8;
	u32 section_count_y = height >> 8;

	LiaccSection *sections = arena_alloc(sizeof(LiaccSection) * section_count_x * section_count_y, 0,0,arena);

	PRNG rg = init_prng(323);
	if(0)
	{
	for(u32 y = 0; y < section_count_y; y++)
	{
		for(u32 x = 0; x < section_count_x; x++)
		{
			u32 i = x + y * section_count_x;
			{
				if((i & 1))
				{
					prng_memset(&rg, sections[i].buffer, sizeof(sections[i].buffer));	
				}
			}
		}
	}
	}
	
	Liacc liacc = {
		.width = width,
		.height = height,
		.requested_width = requested_width,	
		.requested_height = requested_height,	
		.section_count_x = section_count_x,
		.section_count_y = section_count_y,
		.sections = sections,
		.color = color32(255, 190, 100, 255),
		.rg = init_prng(12312),
	};
	return liacc;
}


u32 test_liacc(Liacc *liacc)
{
	PRNG *rg = &liacc->rg;
	*rg = init_prng(get_time_ns());
	u32 line_count = 1024 * 1;
	u32 total_line_count = line_count * liacc->section_count_y * liacc->section_count_x;

	const u32 const_brightness = bit28;

	u32 t = get_time_ns() * 10;
	for(u32 j = 0; j < liacc->section_count_x * liacc->section_count_y; j++)
	{
		if(true)
		{
			for(u32 k = 0; k < line_count; k++)
			{
				u16 y0 = random_u32(rg);
				u16 y1 = random_u32(rg);
				u16 x0 = 0;
				u16 x1 = U16_MAX;




				// I need weather a line is horizontal or vertical.
				// Some greater/less branch or complex math


				///////////////////////
				// Also could start with the outside bounds and the starting point along the major axis.

				// I have this ^ now
				///////////////////////



				const u32 brightness = bitmask18;
				u32 max_brightness = U32_MAX - brightness;;


					
				u16 yd;
				if(y1 > y0) 
					yd = y1-y0;
				else
					yd = y0-y1;

				u16 xd;
				if(x1 > x0) 
					xd = x1-x0;
				else
					xd = x0-x1;

				// Below is just a change of the image addressing pattern from row major (yd > xd) to column major (yd < xd).


				if(xd < yd)
				{
					// Y is major
					u16 dx = (x1>>8) - (x0>>8);
					u16 xm = x0 + ((y0 >> 8) * dx);
					for(u16 i = 0; i < 256; i++)
					{
						u16 x = xm>>8;		
						xm+=dx;
						u32 *val = &liacc->sections[j].buffer[x + (i << 8)];
						*val += brightness;
						*val = defmin(max_brightness, *val);
					}
				}
				else
				{
					// X is major
					u16 dy = (y1>>8) - (y0>>8);
					u16 ym = y0 + ((x0 >> 8) * dy);
					for(u16 i = 0; i < 256; i++)
					{
						u16 y = ym>>8;		
						ym+=dy;
						u32 *val = &liacc->sections[j].buffer[i + (y << 8)];
						*val += brightness;
						*val = defmin(max_brightness, *val);
					}
				}
			}
		}
	}
	return total_line_count;
}



void draw_liacc(View view, Liacc liacc)
{
	for(u32 y = 0; y < view.height; y++)
	{
		void *line = view_line(view, y);
		for(u32 x = 0; x < view.width; x++)
		{
			Color32 *dst = view_pixel(view, line, x);
			u32 pixel_index = (x & 255) + ((y & 255) * 256);
			u32 section_index = (x >> 8) + (y>>8) * liacc.section_count_x;
			u32 *src = &liacc.sections[section_index].buffer[pixel_index];

			{
				u32 s = *src;
				s = s >> 24;
				Color32 color = color32(
					(s * liacc.color.r) >> 8,
					(s * liacc.color.g) >> 8,
					(s * liacc.color.b) >> 8,
					(s * liacc.color.a) >> 8
				);
				*dst = color;
			}
		}
	}
		
}
