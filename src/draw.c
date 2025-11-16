#include "draw.h"
#include "alg.h"





void draw_point(View view, u32 xa, u32 ya, u32 radius, Color32 color)
{
	u32 x0 = defmin(view.width, xa-radius);
	u32 y0 = defmin(view.height, ya-radius);
	u32 x1 = defmin(view.width, xa+radius);
	u32 y1 = defmin(view.height, ya+radius);

	for(u32 y = y0; y < y1; y++)
	{
		void *line = view_line(view, y);
		for(u32 x = x0; x < x1; x++)
		{
			Color32* p = view_pixel(view, line, x);	
			p[0] = color;
		}
	}
}

void draw_line_low(View view, s32 x0, s32 x1, s32 y0, s32 y1, Color32 color)
{
	s32 dx = x1-x0;
	s32 dy = y1-y0;

	s32 yi = 1;
	if(dy < 0)
	{
		dy = -dy;
		yi = -1;
	}
	s32 x = x0;
	s32 y = y0;

	s32 p = 2*dy-dx;
	for(x = x0; x < x1; x++)
	{
		{
			void* line = view_line(view, y);
			Color32 *c = view_pixel(view, line, x);
			c[0] = color32_add(c[0], color);
		}
		if(p > 0)
		{
			y = y+yi;
			p = p + 2 * (dy - dx);
		}
		else
		{
			p = p + 2 * dy;
		}
	}
}

void draw_line_high(View view, s32 x0, s32 x1, s32 y0, s32 y1, Color32 color)
{
	s32 dx = x1-x0;
	s32 dy = y1-y0;

	s32 xi = 1;
	if(dx < 0)
	{
		dx = -dx;
		xi = -1;
	}
	s32 x = x0;
	s32 y = y0;

	s32 p = 2*dx-dy;
	for(y = y0; y < y1; y++)
	{
		{
			void* line = view_line(view, y);
			Color32 *c = view_pixel(view, line, x);
			c[0] = color32_add(c[0], color);
		}
		if(p > 0)
		{
			x = x+xi;
			p = p + 2 * (dx - dy);
		}
		else
		{
			p = p + 2 * dx;
		}
	}
}

void draw_line(View view, u32 xa, u32 ya, u32 xb, u32 yb, Color32 color)
{
	s32 x0 = defmin(view.width, xa);
	s32 x1 = defmin(view.width, xb);
	s32 y0 = defmin(view.height, ya);
	s32 y1 = defmin(view.height, yb);
	if(abs(y1-y0) < abs(x1-x0))
	{
		if(x0 > x1)
		{
			draw_line_low(view,x1,x0,y1,y0,color);
		}
		else
		{
			draw_line_low(view,x0,x1,y0,y1,color);
		}
	}
	else
	{
		if(y0 > y1)
		{
			draw_line_high(view,x1,x0,y1,y0,color);
		}
		else
		{
			draw_line_high(view,x0,x1,y0,y1,color);
		}
	}
}


void draw_rectangle(View view, u32 xa, u32 ya, u32 xb, u32 yb, Color32 color)
{
	u32 x0 = defmin(view.width, xa);
	u32 x1 = defmin(view.width, xb);
	if(x1 < x0){swap(u32, x0, x1);}
	u32 y0 = defmin(view.height, ya);
	u32 y1 = defmin(view.height, yb);
	if(y1 < y0){swap(u32, y0, y1);}
	for(u32 y = y0; y < y1; y++)
	{
		void *line = view_line(view, y);
		for(u32 x = x0; x < x1; x++)
		{
			Color32* p = view_pixel(view, line, x);	
			p[0] = color;
		}
	}
}

// Bounds are already checked
void draw_scanline(View view, u32 x0, u32 x1, u32 y, Color32 color)
{
	Color32* line = view_line(view, y);
	for(u32 x = x0; x < x1; x++)
	{
		line[x] = color;
	}
}


void draw_triangle(View view, u32 xa, u32 ya, u32 ab, u32 yb, u32 xc, u32 yc, Color32 color)
{
		
}
void draw_circle(View view, u32 xa, u32 ya, u32 radius, Color32 color)
{
}

u32* bresenham_store_x_buffer_low(s32 x0, s32 y0, s32 x1, s32 y1, Arena *arena)
{
	s32 dx = x1-x0;
	s32 dy = y1-y0;

	u32 *x_buffer = 0;

	u32 bi = 0;
	s32 yi = 1;
	if(dy < 0)
	{
		dy = -dy;
		yi = -1;
		x_buffer = allocate_buffer(u32, dy, arena);
		bi = buffer_count(x_buffer) - 1;
	}
	else
	{
		x_buffer = allocate_buffer(u32, dy, arena);
		bi = 0;
	}

	s32 x = x0;
	s32 y = y0;

	s32 p = 2*dy-dx;
	for(x = x0; x < x1; x++)
	{
		if(p > 0)
		{
			y = y+yi;
			x_buffer[bi] = x;
			bi = bi + yi;

			p = p + 2 * (dy - dx);
		}
		else
		{
			p = p + 2 * dy;
		}
	}
	return x_buffer;
}

u32* bresenham_store_x_buffer_high(s32 x0, s32 y0, s32 x1, s32 y1, Arena *arena)
{
	s32 dx = x1-x0;
	s32 dy = y1-y0;

	u32 *x_buffer = allocate_buffer(u32, dy, arena);
	u32 bi = 0;

	s32 xi = 1;
	if(dx < 0)
	{
		dx = -dx;
		xi = -1;
	}
	s32 x = x0;
	s32 y = y0;

	s32 p = 2*dx-dy;
	for(y = y0; y < y1; y++)
	{
		if(p > 0)
		{
			x = x+xi;
			p = p + 2 * (dx - dy);
		}
		else
		{
			p = p + 2 * dx;
		}
		x_buffer[bi] = x;
		bi++;
	}
	return x_buffer;
}

u32* bresenham_store_x_buffer(View view, u32 xa, u32 ya, u32 xb, u32 yb, Arena *arena)
{
	s32 x0 = defmin(view.width, xa);
	s32 x1 = defmin(view.width, xb);
	s32 y0 = defmin(view.height, ya);
	s32 y1 = defmin(view.height, yb);
	u32 *x_buffer = 0;
	if(abs(y1-y0) < abs(x1-x0))
	{
		if(x0 > x1)
		{
			x_buffer = bresenham_store_x_buffer_low(x1,y1,x0,y0, arena);
		}
		else
		{
			x_buffer = bresenham_store_x_buffer_low(x0,y0,x1,y1, arena);
		}
	}
	else
	{
		if(y0 > y1)
		{
			x_buffer = bresenham_store_x_buffer_high(x1,y1,x0,y0, arena);
		}
		else
		{
			x_buffer = bresenham_store_x_buffer_high(x0,y0,x1,y1, arena);
		}
	}
	return x_buffer;
}


void draw_x_buffer(View view, u32* x_buffer, u32 y0, Color32 color)
{
	for(u32 i = 0; i < buffer_count(x_buffer); i++)
	{
		u32 y = y0 + i;
		void* line = view_line(view, y);
		Color32* p = view_pixel(view, line, x_buffer[i]);
		p[0] = color;
	}
}

void draw_scanline_unsafe(View view,  u32 xa, u32 xb, u32 y, Color32 color)
{
	Color32 *line = view_line(view, y);
	for(u32 x = xa; x < xb; x++)
	{
		line[x] = color;	
	}
}

void draw_scanline_triangle_flat_top(View view, u32 xa, u32 ya, u32 xb, u32 yb, u32 xc, u32 yc, Color32 color)
{
	Temp temp = begin_temp(0);
	if(xb < xa)
	{
		swap(u32, xa, xb);
		swap(u32, ya, yb);
	}
	u32* buffer_a = bresenham_store_x_buffer(view, xa, ya, xc, yc, temp.arena);
	u32* buffer_b = bresenham_store_x_buffer(view, xb, yb, xc, yc, temp.arena);
	for(u32 y = ya; y < yc; y++)
	{
		draw_scanline_unsafe(view, buffer_a[y-ya], buffer_b[y-ya], y, color);
	}
	end_temp(temp);
}

void draw_scanline_triangle_flat_bottom(View view, u32 xa, u32 ya, u32 xb, u32 yb, u32 xc, u32 yc, Color32 color)
{
	Temp temp = begin_temp(0);
	if(xc < xb)
	{
		swap(u32, xb, xc);
		swap(u32, yb, yc);
	}
	u32* buffer_a = bresenham_store_x_buffer(view, xb, yb, xa, ya, temp.arena);
	u32* buffer_b = bresenham_store_x_buffer(view, xc, yc, xa, ya, temp.arena);
	for(u32 y = ya; y < yc; y++)
	{
		draw_scanline_unsafe(view, buffer_a[y-ya], buffer_b[y-ya], y, color);
	}
	end_temp(temp);
}


void draw_scanline_triangle_general(View view, u32 xa, u32 ya, u32 xb, u32 yb, u32 xc, u32 yc, Color32 color)
{

	Temp temp = begin_temp(0);
	u32 *buffer_long = bresenham_store_x_buffer(view, xa, ya, xc, yc, temp.arena);	
	u32 x_split = buffer_long[yb-ya];
	draw_line(view, x_split, yb, xb, yb, color);
	if(xb == x_split)
	{
	}
	else
	{
		u32 *buffer_top = bresenham_store_x_buffer(view, xa, ya, xb, yb, temp.arena);
		u32 *buffer_bottom = bresenham_store_x_buffer(view, xb, yb, xc, yc, temp.arena);
		if(xb < x_split)
		{
			for(u32 y = ya; y < yb; y++)
			{
				draw_scanline_unsafe(view, buffer_top[y-ya], buffer_long[y-ya], y, color);
			}
			for(u32 y = yb; y < yc; y++)
			{
				draw_scanline_unsafe(view, buffer_bottom[y-yb], buffer_long[y-ya], y, color);
			}
		}
		else 
		{
			for(u32 y = ya; y < yb; y++)
			{
				draw_scanline_unsafe(view, buffer_long[y-ya], buffer_top[y-ya], y, color);
			}
			for(u32 y = yb; y < yc; y++)
			{
				draw_scanline_unsafe(view, buffer_long[y-ya], buffer_bottom[y-yb], y, color);
			}
		}
	}
	end_temp(temp);
}

void draw_scanline_triangle(View view, u32 xa, u32 ya, u32 xb, u32 yb, u32 xc, u32 yc, Color32 color)
{
	if(yc < ya)
	{
		swap(u32, ya, yc);
		swap(u32, xa, xc);
	}
	if(yb < ya)
	{
		swap(u32, ya, yb);
		swap(u32, xa, xb);
	}
	if(yc < yb)
	{
		swap(u32, yb, yc);
		swap(u32, xb, xc);
	}


	if(ya == yb)
	{
		draw_scanline_triangle_flat_top(view, xa, ya, xb, yb, xc, yc, color);
	}
	else if(yb == yc)
	{
		draw_scanline_triangle_flat_bottom(view, xa, ya, xb, yb, xc, yc, color);
	}
	else
	{
		draw_scanline_triangle_general(view, xa, ya, xb, yb, xc, yc, color);
	}



}


typedef struct {
	u32 x0,x1,y0,y1, xoff, yoff;	
}TextureBounds;

TextureBounds compute_texture_bounds(u32 target_width, u32 target_height, u32 texture_width, u32 texture_height, u32 texture_offset_x, u32 texture_offset_y)
{
	u32 x0=0;
	u32 x1=texture_width;
	u32 y0=0;
	u32 y1=texture_height;
	if((texture_offset_x > target_width) && (texture_offset_x + texture_width > target_width))
	{
		x0 = 0;
		x1 = 0;
	}
	else
	{
		if(texture_offset_x > target_width)
		{
			x0 = (0 - texture_offset_x);
		}
		if(texture_offset_x + texture_width > target_width)
		{
			x1 = texture_width - ((texture_offset_x + texture_width) - target_width);
		}
	}

	if((texture_offset_y > target_height) && (texture_offset_y + texture_height > target_height))
	{
		y0 = 0;
		y1 = 0;
	}
	else
	{
		if(texture_offset_y > target_height)
		{
			y0 = (0 - texture_offset_y);
		}
		if(texture_offset_y + texture_height > target_height)
		{
			y1 = texture_height - ((texture_offset_y + texture_height) - target_height);
		}
	}
	TextureBounds bounds = {
		.x0 = x0,
		.x1 = x1,
		.y0 = y0,
		.y1 = y1,
	};
	return bounds;
}


void draw_glyph(View view, Glyph glyph, u32 xo, u32 yo, Color32 color)
{
	if(glyph.data == 0)
	{
		xo += glyph.x_bearing;
		TextureBounds tb = compute_texture_bounds(view.width, view.height, glyph.width, glyph.height, xo, yo);
		for(u32 y = tb.y0; y < tb.y1; y++)
		{
			void *line = view_line(view, y + yo);
			for(u32 x = tb.x0; x < tb.x1; x++)
			{
				Color32* p = view_pixel(view, line, x + xo);
				p[0] = color32_add(p[0], color);
			}
		}
	}
	else
	{
		yo = yo - glyph.y_bearing + glyph.requested_height;
		xo += glyph.x_bearing;
		TextureBounds tb = compute_texture_bounds(view.width, view.height, glyph.width, glyph.height, xo, yo);
		for(u32 y = tb.y0; y < tb.y1; y++)
		{
			void *line = view_line(view, y + yo);
			for(u32 x = tb.x0; x < tb.x1; x++)
			{

				Color32* p = view_pixel(view, line, x + xo);
				u32 c = glyph.data[x + y * glyph.width];
				if(c)
				{
					p[0] = color32_add(p[0], color32((c * color.r) >> 8,(c * color.g) >> 8, (c * color.b) >> 8, (c * color.a) >> 8));
				}
			}
		}
	}
}

void draw_glyph_cstring(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *c)
{
	u32 x = x0;
	u32 y = y0;
	while(*c)
	{
		Glyph  *g = load_glyph_to_font(font, *c, height);	


		if((x + g->width > x1) || (*c == '\n'))
		{
			if((x < view.width) || (x0 < view.width))
			{
				y = y + g->vertical_advance;
				x = x0;
			}
		}
		if((*c > ' ') && (*c < 128))
		{
			draw_glyph(view, *g, x,y, color);
		}
		x += g->horizontal_advance;
		c++;

	}
}
void draw_glyph_print_va(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *format, va_list l)
{
	Temp temp = begin_temp(0);
	String8 buffer = va_string_print(temp.arena, format, l);
	draw_glyph_cstring(view, font, height, x0, x1, y0, color, (char*)buffer.str);
	end_temp(temp);
}
void draw_glyph_print(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *format, ...)
{
	va_list l;
	va_start(l, format);
	draw_glyph_print_va(view, font, height, x0, x1, y0, color, format, l);
	va_end(l);
}

void myLin_e(u32* acc, int x, int y, int x2, int y2) {
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

AccumulatorImage allocate_accumulator_image(u32 width, u32 height, Arena *arena)
{
	width = defmax(64,width);
	height = defmax(64,height);
	u64 size = width * height * sizeof(u32);
	AccumulatorImage image = {
		.values = arena_alloc(size, 0,0,arena),
		.width = width,
		.height = height,
	};
	memset(image.values, 0, size);
	return image;
}


void copy_accumulator_to_view(View view, AccumulatorImage image)
{

	u64 max = 0;
	for(u32 i = 0; i < image.width * image.height; i++)
	{
		max = defmax(image.values[i], max);
	}
	u32 mul = 1;
	if(max>1)
	{
		mul = U32_MAX / max;	
	}
	mul = defmax(1, mul);
	print("%u32\n", mul);

	u32 width = defmin(view.width, image.width);
	u32 height = defmin(view.height, image.height);
	for(u32 y = 0; y < height; y++)
	{
		void *line = view_line(view, y);
		for(u32 x = 0; x < width; x++)
		{
			Color32 *p = view_pixel(view, line, x);
			u32 v = image.values[x + y * image.width];
			v *= mul;
			*p = color32(
				v>>24,	
				v>>24,	
				v>>24,	
				v>>24
			);
		}
	}
}


void draw_line_fast(AccumulatorImage acc, s32 x0, s32 y0, s32 x1, s32 y1, u32 value)
{
	s32 small = x1-x0;
	s32 large = y1-y0;
	s32 inc, end;
	b32 xlarge = false;
	if(abs(small) > abs(large))
	{
		xlarge = true;
		swap(s32, small, large);	
	}
	end = large;
	if(large<0)
	{
		inc = -1;
		large = -large;
	}
	else
	{
		inc = 1;
	}
	s32 dec = 0;
	if(large != 0)
	{
		dec = (small << 16) / large;
	}
	s32 j = 0;
	if(xlarge)
	{
		for(s32 i = 0; i != end; i+=inc)
		{
			u32 index = (x0+i) + (y0+(j>>16)) * acc.width;
			acc.values[index] += value;
			j+=dec;
		}
	}
	else
	{
		for(s32 i = 0; i != end; i+=inc)
		{
			u32 index = (x0+(j>>16)) + (y0+i) * acc.width;
			acc.values[index] += value;
			j+=dec;
		}
	}
}

void triangle_test(Window *window, Font *font)
{
	Swapchain *swapchain = window->swapchain_data;
	View view = swapchain->images[swapchain->index].view;
	static u64 triangle_count = 30000;
	PRNG rg = init_prng(64);
	u32 target_time = 1000000000/10;

	for(u32 i = 0; i < triangle_count; i++)
	{
		LOOP_START:
		u32 x = random_u64(&rg) % (window->width);
		u32 y = random_u64(&rg) % (window->height);

		u32 scale = 10;
		
		u32 xs[3];
		for(u32 i = 0; i < arrlen(xs); i++)
		{
			xs[i] = random_u64(&rg) % scale;
			xs[i] -= scale / 2;
			xs[i] += x;
			if(xs[i] > window->width)
			{
				goto LOOP_START;
			}
		}
		u32 ys[3];
		for(u32 i = 0; i < arrlen(ys); i++)
		{
			ys[i] = random_u64(&rg) % scale;
			ys[i] -= scale / 2;
			ys[i] += y;
			if(ys[i] > window->height)
			{
				goto LOOP_START;
			}
		}
		Color32 color = random_color32(&rg);

		draw_scanline_triangle(view, xs[0], ys[0], xs[1], ys[1], xs[2], ys[2], color);
	}


	u64 target_triangle_count = triangle_count;
	f64 percent_error = 0.0;
	if(window->loop_time.elapsed)
	{
		f64 time_per_triangle = (f64)window->loop_time.elapsed / (f64)triangle_count;
		f64 target_count = (f64)target_time / time_per_triangle;
		percent_error = abs((f64)window->loop_time.elapsed - (f64)target_time) / (f64)target_time;
		target_triangle_count = target_count;
	}

	if(percent_error > 0.1)
	{
		
		if(target_triangle_count > triangle_count)
		{
			triangle_count += 1000;
		}
		else
		{
			triangle_count -= 1000;
		}
	}
	triangle_count = defmin(10000000, triangle_count);
	if(0)
	{
		draw_rectangle(view, 0,0, 160, 100, color32(0,0,0,0));
		draw_glyph_print(view, font, 32, 0, window->width, 0, color32(255, 240, 200, 255), "%u64us\n", window->not_present_time / 1000);
		draw_glyph_print(view, font, 16, 0, window->width, 32, color32(255, 240, 200, 255), "Triangles: %u64\n", triangle_count);
		draw_glyph_print(view, font, 16, 0, window->width, 48, color32(255, 240, 200, 255), "Target: %u64us\n", target_time / 1000);
	}	
}

