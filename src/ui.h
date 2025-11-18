#pragma once

#include "device_graphics.h"

typedef enum{
	UI_ELEMENT_NULL = 0,
	UI_ELEMENT_WARM_NULL,
	UI_ELEMENT_BOX,
	UI_ELEMENT_BUTTON,
	UI_ELEMENT_SLIDER,
	UI_ELEMENT_COUNT,
}UI_ElementTypeFlags;
typedef u32 UI_ElementType;


typedef struct{
	fvec4 color;
	b32 enabled;
}UI_Color;

UI_Color ui_color(f32 r, f32 g, f32 b, f32 a);
UI_Color ui_color_disable();

struct UI_Element;
typedef struct UI_ElementColorTheme{
	UI_Color text;
	UI_Color foreground;
	UI_Color background;
	UI_Color outline;
	UI_Color hover_tint;
	UI_Color states[2];


	f32 corner_radius;
	f32 outline_thickness;
	f32 slider_width;
}UI_ElementTheme;

typedef struct{
	struct UI_Element *root;
	u64 time;
	u32 frame_index;
	u32 last_frame_index;
	u64 frame_accum;
	u32 max_element_count;
	u32 element_counts[2];
	b32 focused;
	struct UI_Element *element_maps[2];	
	Arena frame_arenas[2];
	PRNG prng;

	fvec2 frame_pixel_size;
	fvec2 frame_norm_size;
	fvec2 frame_norm_pixel_size;
	fvec2 frame_norm_top_left;
	fvec2 frame_norm_bottom_right;

	fmat3 affine_matrix;


	DeviceVertexBuffer *vb;
	const SimpleFont *simple_font;
	FrameEvents fe;

	UI_ElementTheme element_themes[UI_ELEMENT_COUNT];

}UI;


typedef enum{
	UI_BUTTON_MONOSTABLE,
	UI_BUTTON_ASTABLE,
}UI_ButtonTypeFlags;
typedef u32 UI_ButtonType;

typedef struct UI_ElementState{
	b32 hovering;
	b32 pressed;
	f32 norm;	

	b32 first_pressed;
	b32 first_hovering;

	fvec2 pressed_position;

	b32 *hovering_ptr;
	b32 *pressed_ptr;
	f32 *norm_ptr;
}UI_ElementState;

typedef struct UI_Element{
	UI_ElementType type;
	u64 frame_accum;
	String8 local_name;
	String8 global_name;

	UI *ui;

	struct UI_Element *children_tail;
	struct UI_Element *children;
	struct UI_Element *peer;
	struct UI_Element *parent;

	fvec2 top_left,bottom_right;

	UI_ElementTheme theme;
	String8 text;		

	UI_ElementState state;

	union{
		struct UI_ElementBox{
		}box;
		struct UI_ElementButton{
		}button;
		struct UI_ElementSlider{
			struct UI_Element *button;
		}slider;
	};
}UI_Element;

typedef UI_Element UI_Box;
typedef UI_Element UI_Button;
typedef UI_Element UI_Slider;

UI *ui_test(DeviceVertexBuffer *vb, const SimpleFont *simple_font, FrameEvents fe, uvec2 frame_size);
