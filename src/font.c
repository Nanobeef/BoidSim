#include "font.h"
#include "alg.h"

#include <ft2build.h>
#include FT_FREETYPE_H


typedef struct{
	u32 width, height;
	u32 horizontal_advance, vertical_advance;
	u32 x_bearing, y_bearing;
	u64 size;
}GlyphSize;


GlyphSize calc_glyph_size(FT_GlyphSlot g)
{
	u64 size = (g->metrics.height >> 6) * (g->metrics.width >> 6) * sizeof(u8);
	GlyphSize ret = {
		.size = size,
		.width = (g->metrics.width >> 6),
		.height = (g->metrics.height >> 6),
		.horizontal_advance = (g->metrics.horiAdvance >> 6),
		.vertical_advance = (g->metrics.vertAdvance>> 6),
		.x_bearing = (g->metrics.horiBearingX >> 6),
		.y_bearing = (g->metrics.horiBearingY >> 6),
	};
	return ret;
}

typedef struct{
	FT_Library lib;
	FT_Face face;
}Freetype;

#define BERKELEY_MONO_PATH "bin/berkeley-mono/BerkeleyMono-SemiBold-Condensed.ttf"

GlyphGenerator create_glyph_generator(const char* path, Arena *arena)
{
	if(path == 0)
	{
		path = BERKELEY_MONO_PATH;
	}

	FT_Error error = {0};
	FT_Library lib = {0};
	error = FT_Init_FreeType(&lib);
	if(error)
	{
		DEBUG_ABORT("Failed to initialize freetype2\n");
	}
	FT_Face face = {0};
	error = FT_New_Face(lib, path, 0, &face);
	if(error)
	{
		DEBUG_ABORT("Failed to initialize face\n");
	}
	Freetype *freetype = arena_alloc(sizeof(Freetype), 0,0, arena);
	*freetype = (Freetype){
		.lib = lib,		
		.face = face,
	};
	GlyphGenerator gen = {
		.handle = freetype,
		.width = (face->bbox.xMax - face->bbox.xMin)>>6,
		.height = (face->bbox.yMax - face->bbox.yMin)>>6,
	};
	return gen;
}

void destroy_glyph_generator(GlyphGenerator gen)
{
	Freetype *freetype = gen.handle;
	FT_Done_Face(freetype->face);
	FT_Done_FreeType(freetype->lib);
}

void size_glyph_generator(GlyphGenerator *gen, u32 requested_height)
{
	if(gen->requested_height != requested_height)
	{
		Freetype *freetype = gen->handle;
		FT_Face face = freetype->face;
		FT_Set_Pixel_Sizes(face, 0, requested_height);
		gen->requested_height = requested_height;
		gen->ascender = face->ascender>>6;
		gen->descender = face->descender>>6;
		gen->advance = face->max_advance_width;
	}
}


Glyph load_glyph(GlyphGenerator gen, u32 index, Arena * arena)
{
	Freetype *f = gen.handle;
	FT_Load_Glyph(f->face, index, FT_LOAD_RENDER);
	FT_GlyphSlot g= f->face->glyph;
	GlyphSize gs = calc_glyph_size(g);
	u32 width = gs.width;
	u32 height = gs.height;
	u8* data = arena_alloc(width * height, 0,0,arena);
	for(u32 y = 0; y < g->bitmap.rows; y++)
	{
		memcpy(data + y * width, g->bitmap.buffer + g->bitmap.pitch * y, width);
	}

	Glyph glyph = {
		.data = data,
		.width = gs.width,
		.height = gs.height,
		.font_index = index,
		.requested_height = gen.requested_height,
		.horizontal_advance = gs.horizontal_advance,
		.vertical_advance = gs.vertical_advance,
		.x_bearing = gs.x_bearing,
		.y_bearing = gs.y_bearing,
		.ascender = gen.ascender,
		.descender = gen.descender,
		.global_line_gap = gen.line_gap,
	};
	return glyph;
}

GlyphSize load_glyph_size(GlyphGenerator gen, u32 index)
{
	Freetype *f = gen.handle;
	FT_Load_Glyph(f->face, index, FT_LOAD_COMPUTE_METRICS);
	FT_GlyphSlot g = f->face->glyph;	
	GlyphSize ret = calc_glyph_size(g);
	return ret;
}


u32 load_glyph_index(GlyphGenerator gen, s32 code)
{
	Freetype *f = gen.handle;
	s32 index = FT_Get_Char_Index(f->face, code);
	return (u32)index;
}

void print_glyph(Glyph glyph)
{
	for(u32 y = 0; y < glyph.height; y++)
	{
		for(u32 x = 0; x < glyph.width; x++)
		{
			u8 b = glyph.data[x + y * glyph.width];
			if(b == 255){print("%%");}
			else if(b > 128){print(":");}
			else if(b > 0){print(".");}
			else{print(" ");}
		}
		print("\n");
	}
	print("\n");
}


Font* create_font(const char *path, u32 max_glyph_count, u64 arena_size, Arena *arena)
{
	Font *font = arena_alloc(sizeof(Font), 0,0,arena);
	*font = (Font){0};
	font->gen = create_glyph_generator(path, arena);
	font->hash_map_capacity = uint_next_power_of_two(max_glyph_count);
	font->hash_map_count = 0;
	font->hash_map_mask = font->hash_map_capacity-1;
	u64 map_size = sizeof(Glyph) * font->hash_map_capacity;
	font->glyph_hash_map = arena_alloc(map_size,0,0, arena);
	memset(font->glyph_hash_map, 0, map_size);

	font->arena_ring = allocate_ring(Arena, 2, arena);
	itterate_ring(font->arena_ring, font->arena_ring[i] = allocate_sub_arena(arena_size, arena));
	

	return font;
}

void destroy_font(Font *font)
{
	destroy_glyph_generator(font->gen);
}


Glyph* load_glyph_to_font(Font* font, u32 code, u32 height)
{
	u64 src_key = ((u64)height << 0) | ((u64)code << 32);
	u64 hash_index = splitmix64_hash(src_key) & font->hash_map_mask;

	s32 i = 0; Glyph *last_glyph = 0;
	{
		Glyph *g = &font->glyph_hash_map[hash_index];
		while(g)
		{
			u64 dst_key = ((u64)g->requested_height << 0) | ((u64)g->code << 32);
			if(src_key == dst_key)
			{
				g->reuse_count++;
				return g;		
			}
			last_glyph = g;	
			g = g->next;
		}

	}

	{
		size_glyph_generator(&font->gen, height);
		Arena *arena = ring_current(font->arena_ring);
		u32 glyph_index = load_glyph_index(font->gen, code);

		if(glyph_index)
		{
			GlyphSize glyph_size = load_glyph_size(font->gen, glyph_index);
			u64 alignment = 64;
			u64 required_size = glyph_size.size + sizeof(Glyph) + alignment;
			if((arena->end - forward_align_pointer(arena->pos, alignment)) < required_size)
			{
				DEBUG_PRINT("NEED TO MOVE GLYPH TO NEXT ARENA\n");		
			}
			
			Glyph glyph = load_glyph(font->gen, glyph_index, arena);
			glyph.reuse_count = 0;
			glyph.code = code;

			last_glyph->next = arena_alloc(sizeof(Glyph),0,0, arena);
			memcpy(last_glyph->next, &glyph, sizeof(Glyph));
		}
		else
		{
			GlyphSize gs = load_glyph_size(font->gen, glyph_index);
			Glyph glyph = {
				.data = 0,
				.width = gs.horizontal_advance,
				.height = gs.vertical_advance,
				.requested_height = height,
				.font_index = glyph_index,
				.reuse_count = 0,
				.code = code,
				.horizontal_advance = gs.horizontal_advance,
				.vertical_advance = gs.vertical_advance,
				.global_line_gap = font->gen.line_gap,
				.next = 0,
				.ascender = font->gen.ascender,
				.descender = font->gen.descender,
			};
			last_glyph->next = arena_alloc(sizeof(Glyph),0,0, arena);
			memcpy(last_glyph->next, &glyph, sizeof(Glyph));
		}
	}
	return last_glyph->next;
}

