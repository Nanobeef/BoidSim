#pragma once

#include "basic.h"


typedef struct Glyph{
	u8* data;
	u32 width;
	u32 height;
	u32 reuse_count;
	u32 code;

	u32 font_index; 
	u32 requested_height;

	u32 horizontal_advance, vertical_advance;
	u32 x_bearing, y_bearing;
	u32 ascender, descender;

	u32 global_line_gap;

	struct Glyph *next;
}Glyph;

typedef struct{
	void *handle;	
	u32 requested_height;
	u32 width, height;
	s32 line_gap;
	s32 advance;
	s32 ascender;
	s32 descender;
}GlyphGenerator;

typedef struct Font{
	GlyphGenerator gen;			

	Arena *arena_ring;

	u64 hash_map_capacity;
	u64 hash_map_count;
	u64 hash_map_mask;
	Glyph *glyph_hash_map;	

}Font;

GlyphGenerator create_glyph_generator(const char* path, Arena *arena);
void destroy_glyph_generator(GlyphGenerator gen);
void size_glyph_generator(GlyphGenerator *gen, u32 height);
Glyph load_glyph(GlyphGenerator gen, u32 code, Arena * arena);
void print_glyph(Glyph glyph);
void font_test(Arena *arena);

Font* create_font(const char *path, u32 max_glyph_count, u64 arena_size, Arena *arena);
void destroy_font(Font* font);
Glyph* load_glyph_to_font(Font* font, u32 code, u32 height);
