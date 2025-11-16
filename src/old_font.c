#include "old_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library ft_library = 0;


typedef struct{
	FT_Face face;
}LibData;


OldGlyphGenerator* create_old_glyph_generator(const char* path, u64 memory_size, const void *memory, Arena* arena)
{

	FT_Error error = {0};
	if(ft_library == 0)
	{
		error = FT_Init_FreeType(&ft_library);	
		if(error)
		{
			const char* str = FT_Error_String(error);
			print("FT2 init Error\n");
			if(str)
			{
				print("Freetype2 Error: %cs\n", str);
			}
			return 0;
		}
	}


	FT_Face face = {0};
	b32 face_initialized = false;
	if(memory && (face_initialized == false))
	{
		FT_Open_Args args = {
			.flags = FT_OPEN_MEMORY,
			.memory_base = memory,
			.memory_size = memory_size,
		};
		face_initialized = true;
		error = FT_Open_Face(ft_library, &args, 0, &face);
		if(error)
		{ 
			const char* str = FT_Error_String(error);
			print("Freetype2 Error: %cs\n", str);
			return 0;
		}
		face_initialized = true;
	}
	if(path && (face_initialized == false))
	{
		RUNTIME_ABORT("Don't use this fallback");
		print("Fallback to %cstr\n", path);
		face_initialized = true;
		error = FT_New_Face(ft_library, path, 0, &face);
		if(error)
		{ 
			const char* str = FT_Error_String(error);
			print("Freetype2 Error: %cs\n", str);
			return 0;
		}
		face_initialized = true;

	}
	if(face_initialized == false)
	{

	}



	if(error)
	{
		const char* str = FT_Error_String(error);
		print("Freetype2 Error: %cs\n", str);
		return 0;
	}
	OldGlyphGenerator* gen = arena_alloc(sizeof(OldGlyphGenerator),0,0, arena);
		
	LibData* lib_data = arena_alloc(sizeof(LibData),0,0, arena);
	lib_data->face = face;

	//u32 table_capacity = 512;

	*gen = (OldGlyphGenerator){
		.lib_data = lib_data,
		.pt = 0,
	};
	return gen;
}

void destroy_old_glyph_generator(OldGlyphGenerator* gen)
{
	LibData* lib_data = gen->lib_data;
	FT_Done_Face(lib_data->face);
}

b32 old_glyph_generator_set_size(OldGlyphGenerator* gen, u32 pt)
{
	LibData* lib_data = gen->lib_data;
//	FT_Error error = FT_Set_Char_Size(lib_data->face, 0, pt * 64, 300, 300);
	FT_Error error = FT_Set_Pixel_Sizes(lib_data->face, 0, pt);

	if(error)
	{
		const char* str = FT_Error_String(error); print("Freetype2 Error: %cs\n", str);
		return true;	
	}
	gen->pt = pt;
	gen->font_size = uvec2_make(
		lib_data->face->size->metrics.max_advance >> 6,
		lib_data->face->size->metrics.height >> 6
//		lib_data->face->size->metrics.ascender >> 6
		);
	gen->ascender = lib_data->face->ascender >> 6;
	gen->descender = lib_data->face->descender >> 6;

	return 0;
}
OldGlyph* load_old_glyph(OldGlyphGenerator* gen, s32 code, Arena* arena)
{
	LibData* lib_data = gen->lib_data;
	FT_Face face = lib_data->face;
//	u32 pt = gen->pt;
	FT_UInt glyph_index = FT_Get_Char_Index(face, code);
	FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING);
	if(error)
	{
		const char* str = FT_Error_String(error);
		print("Freetype2 Error: %cs\n", str);
		return 0;
	}

	FT_GlyphSlot slot = face->glyph;

	error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
	if(error)
	{
		const char* str = FT_Error_String(error);
		print("Freetype2 Error: %cs\n", str);
		return 0;
	}
	u64 bitmap_size = slot->bitmap.width * slot->bitmap.rows;
	u8* bitmap = arena_alloc(bitmap_size * sizeof(u8), 0,0,arena);
	u64 src_i = 0;
	u64 dst_i = 0;
	for(u32 y = 0; y < slot->bitmap.rows; y++)
	{
		for(u32 x = 0; x < slot->bitmap.width; x++)
		{
			bitmap[dst_i++] = ((u8*)slot->bitmap.buffer)[src_i++];
		}
		src_i += slot->bitmap.pitch - slot->bitmap.width;
	}
//	s32 horizontal_bearing_x, horizontal_bearing_y;
//	s32 vertical_bearing_x, vertical_bearing_y;
//	s32 horizontal_advance, vertical_advance;
	OldGlyph glyph = {
		.generator = gen,	
		.bitmap = bitmap,
		.width = slot->bitmap.width,
		.height = slot->bitmap.rows,
		.horizontal_bearing_x = slot->metrics.horiBearingX >> 6,
		.horizontal_bearing_y = slot->metrics.horiBearingY >> 6,
		.vertical_bearing_x = slot->metrics.vertBearingX >> 6,
		.vertical_bearing_y = slot->metrics.vertBearingY >> 6,
		.horizontal_advance = slot->metrics.horiAdvance >> 6,
		.vertical_advance = slot->metrics.vertAdvance >> 6,

	};
	gen->glyph = glyph;
	return &gen->glyph;
}

void print_old_glyph(OldGlyph g)
{
	u64 i = 0;
	for(u32 y = 0; y < g.height; y++)
	{
		for(u32 x = 0; x < g.width; x++)
		{
			u8 v = g.bitmap[i++];
			if(v > 100)
			{
				print("@");
			}
			else if(v > 0)
			{
				print("-");
			}
			else
			{
				print(".");
			}
		}
		print("\n");
	}
	print("\n");
}

void test_font(const char *path, Arena* arena)
{
	OldGlyphGenerator* gen = create_old_glyph_generator(path,0,0, arena);
	
	old_glyph_generator_set_size(gen, 10);
	for(char i = ' '; i < '~'; i++)
	{
		u8* mark = arena->pos;
		load_old_glyph(gen, i, arena);
		print_old_glyph(gen->glyph);
		arena->pos = mark;
	}

	destroy_old_glyph_generator(gen);

}


