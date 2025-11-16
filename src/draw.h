#pragma once

#include "font.h"
#include "image.h"
#include "window.h"

// Predecessor to scanline.h
// Regular drawing to an image.



void draw_point(View view, u32 xa, u32 ya, u32 radius, Color32 color);
void draw_line(View view, u32 xa, u32 ya, u32 xb, u32 yb, Color32 color);
void draw_rectangle(View view, u32 xa, u32 ya, u32 xb, u32 yb, Color32 color);
void draw_triangle(View view, u32 xa, u32 ya, u32 ab, u32 yb, u32 xc, u32 yc, Color32 color);
void draw_circle(View view, u32 xa, u32 ya, u32 radius, Color32 color);





u32* bresenham_store_x_buffer_low(s32 x0, s32 x1, s32 y0, s32 y1, Arena *arena);
u32* bresenham_store_x_buffer_high(s32 x0, s32 x1, s32 y0, s32 y1, Arena *arena);
u32* bresenham_store_x_buffer(View view, u32 xa, u32 ya, u32 xb, u32 yb, Arena *arena);

void draw_x_buffer(View view, u32* x_buffer, u32 y0, Color32 color);
void draw_scanline(View view, u32 x0, u32 x1, u32 y, Color32 color);





void draw_glyph(View view, Glyph glyph, u32 xo, u32 yo, Color32 color);
void draw_glyph_cstring(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *c);
void draw_glyph_print_va(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *format, va_list l); 
void draw_glyph_print(View view, Font *font, u32 height, u32 x0, u32 x1, u32 y0, Color32 color, const char *format, ...);

void draw_scanline_triangle(View view, u32 xa, u32 ya, u32 xb, u32 yb, u32 xc, u32 yc, Color32 color);

typedef struct{
	u32 width;
	u32 height;
	u32 *values;
}AccumulatorImage;
AccumulatorImage allocate_accumulator_image(u32 width, u32 height, Arena *arena);
void copy_accumulator_to_view(View view, AccumulatorImage image);
void draw_line_fast(AccumulatorImage acc, s32 x0, s32 y0, s32 x1, s32 y1, u32 value);

void triangle_test(Window *window, Font *font);
