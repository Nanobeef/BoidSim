
#include "basic.h"

struct OldGlyphGenerator;

typedef struct OldGlyph{
	struct OldGlyphGenerator* generator;	
	u8* bitmap;
	u32 width, height;
	s32 horizontal_bearing_x, horizontal_bearing_y;
	s32 vertical_bearing_x, vertical_bearing_y;
	s32 horizontal_advance, vertical_advance;
	u32 font_id;

}OldGlyph;

// OldGlyph Generator 
typedef struct OldGlyphGenerator{
	// Copies of PS_FontInfoRec
	u32 id; // No duplicate fonts can be loaded: name, weight, angle, ...  // Will check the info header of font
	
	s64 angle;
	b32 is_fixed_pitch;
	b32 is_monospaced;
	s8 underline_position;
	u8 underline_thickness;
	s32 ascender;
	s32 descender;


	b32 is_vertical;


	OldGlyph glyph;
	uvec2 font_size;
	u32 pt;

	void* lib_data;

	// TODO: 
	//	Make this a font loader, not for any one font but for glyphs in general. 
	//  Keep track of multiple FT_Face objects.
	//

}OldGlyphGenerator;

OldGlyphGenerator* create_old_glyph_generator(const char* path, u64 memory_size, const void *memory, Arena* arena);
void destroy_old_glyph_generator(OldGlyphGenerator* gen);
b32 old_glyph_generator_set_size(OldGlyphGenerator* gen, u32 pt);
OldGlyph* load_old_glyph(OldGlyphGenerator* gen, s32 code, Arena* arena);
void print_old_glyph(OldGlyph g);
void test_font(const char *path, Arena* arena);
