#include "ui.h"
#include "embed.h"

fvec2 ui_pixel_to_screen(UI *ui, fvec2 pixel)
{
	return fvec2_add(fvec2_mul(pixel, ui->frame_norm_pixel_size), ui->frame_norm_top_left);
}

UI_Color ui_color(f32 r, f32 g, f32 b, f32 a)
{
	return (UI_Color){(fvec4){r,g,b,a}, true};
}

UI_Color ui_color_disable()
{
	return (UI_Color){(fvec4){1.0, 0.0, 1.0, 1.0}, false};
}


UI* allocate_ui(u32 max_element_count, Arena *arena)
{
	UI *ui = arena_alloc(sizeof(UI), 0,0, arena);
	*ui = (UI){
		.max_element_count = max_element_count,
	};
	ui->root = arena_alloc(sizeof(UI_Element), 0,0, arena);
	String8 name = str8_lit("ui");
	ui->root->local_name = str8_copy(name, arena);
	ui->root->global_name = str8_copy(name, arena);
	ui->root->ui = ui;
	for(u32 i = 0; i < 2; i++)
	{
		ui->element_maps[i] = arena_alloc(sizeof(UI_Element) * max_element_count, 0,1,arena);
	}


	for(u32 i = 0; i < 2; i++)
	{
		ui->frame_arenas[i] = allocate_sub_arena(1024 * 32, arena);
	}

	ui->element_themes[UI_ELEMENT_BOX] = (UI_ElementTheme){
		.foreground = ui_color(255.0/256.0, 253.0/256.0, 208.0/256.0, 1.0),
		.background = ui_color(0.0, 0.0, 0.0, 0.9),
		.outline = ui_color(1.0, 1.0, 1.0, 1.0),
		.hover_tint = ui_color(0.9, 0.9, 0.9, 0.05),
		.states[0] = ui_color(0.0, 0.0, 0.0, 0.4),
		.states[1] = ui_color(0.0, 0.1, 0.0, 0.4),
	};

	return ui;
}

UI* ui_next_frame(UI *ui, uvec2 frame_size, FrameEvents fe, DeviceVertexBuffer *vb)
{
	ui->frame_accum++;
	ui->frame_pixel_size = fvec2_cast_uvec2(frame_size);
	ui->frame_norm_size = fvec2_make(2.0 * (ui->frame_pixel_size.x / ui->frame_pixel_size.y), 2.0);
	fvec2 affine_scale = fvec2_scalar_div(ui->frame_norm_size, 2.0);
	ui->affine_matrix = fmat3_affine_scale(
		1.0 / affine_scale.x,
		affine_scale.y
	);
	ui->frame_norm_pixel_size = fvec2_div(ui->frame_norm_size, ui->frame_pixel_size);
	ui->frame_norm_top_left = fvec2_scalar_div(ui->frame_norm_size, -2.0f);
	ui->frame_norm_bottom_right = fvec2_add(ui->frame_norm_size, ui->frame_norm_top_left);
	ui->frame_accum++;
	ui->last_frame_index = ui->frame_index;
	ui->frame_index = (ui->frame_accum) & 1;

	if(0)
	{
		memzero(ui->element_maps[ui->frame_index], sizeof(UI_Element) * ui->max_element_count);
	}
	else
	{
		for(u32 i = 0; i < ui->max_element_count; i++){ui->element_maps[ui->frame_index][i].type = UI_ELEMENT_NULL;}
	}


	ui->fe = fe;
	ui->vb = vb;

	reset_arena(&ui->frame_arenas[ui->frame_index]);
	ui->prng = init_prng(1232543);
	return ui;
}


#define ui_box(PE, NAME, X0, Y0, X1, Y1, ...)\
({\
	UI_Element *e = ui_element(PE, str8_lit(NAME), (fvec2){X0,Y0}, (fvec2){X1,Y1}, UI_ELEMENT_BOX);\
	if(e)\
	{\
		UI *__ui = (PE)->ui;\
		_Pragma("GCC diagnostic push");\
		_Pragma("GCC diagnostic ignored \"-Woverride-init-side-effects\"");\
		e->box = (struct UI_ElementBox){\
			__VA_ARGS__\
		};\
		_Pragma("GCC diagnostic pop");\
	}\
	e;\
})

UI_Element *ui_hash_map_insert(u32 map_capacity, UI_Element *map, String8 global_name, UI_ElementType type)
{
	u64 seed = 5381;
	u64 hash = str8_hash(global_name, seed);
	u32 a = hash; 
	u32 b = map_capacity;
	UI_Element *dst = 0;
	for(u32 j = 0; j < 2; j++)
	{
		for(u32 i = a; i < b; i++)
		{
			// Don't want to stop search too early if an element was removed.
			if(map[i].type > UI_ELEMENT_WARM_NULL)
			{
				if(str8_equal(map[i].global_name, global_name) && map[i].type == type)
				{
					dst = &map[i];
					goto FOUND;		
				}
			}
			else if(map[i].type == UI_ELEMENT_NULL)
			{
				dst = &map[i];
				goto FOUND;
			}
			
		}
		a = 0;
		b = hash;
	}
FOUND:
	return dst;
}

UI_Element *ui_map_find_slot(UI *ui, String8 global_name, UI_ElementType type)
{
	UI_Element *previous_map = ui->element_maps[ui->last_frame_index];
	UI_Element *previous_slot = ui_hash_map_insert(ui->max_element_count, previous_map, global_name, type);

	UI_Element *map = ui->element_maps[ui->last_frame_index];
	UI_Element *slot = ui_hash_map_insert(ui->max_element_count, map, global_name, type);
	if(slot == 0)
	{
		return 0;
	}
	if(previous_slot)
	{
		if((slot->type != type) && (previous_slot->type == type))
		{
			*slot = *previous_slot;	
		}
	}
	else
	{
		slot->frame_accum = ui->frame_accum;	
	}
	return slot;
}

UI_Element *ui_element(UI_Element *pe, String8 name, fvec2 top_left, fvec2 bottom_right, UI_ElementType type)
{
	UI *ui = pe->ui;
	Arena *arena = &ui->frame_arenas[ui->frame_index];
	PRNG* rg = &ui->prng;

	
	String8 local_name;
	if(name.str)
		local_name = str8_copy(name, arena);
	else
		local_name = str8_random(4,rg, arena);

	String8 parent_name = pe->global_name;
	String8 global_name = str8_combine(arena, str8_lit("/"), 2, parent_name, local_name);

	UI_Element *map = ui->element_maps[ui->frame_index];
	UI_Element *dst = ui_map_find_slot(ui, global_name, type);
	if(dst == 0)
	{
		DEBUG_ABORT("UI map is full\n");
		return 0;
	}
	UI_ElementState state = {0};
	if(dst->frame_accum != ui->frame_accum)
	{
		state = dst->state;
	}
	UI_Element element = {
		.type = type,
		.ui = ui,
		.frame_accum = ui->frame_accum,
		.children_tail = 0,
		.parent = pe,
		.local_name = local_name,
		.global_name = global_name,
		.top_left = top_left,
		.bottom_right = bottom_right,
		.state = state,
	};

	if(pe->type == type)
		element.theme = pe->theme;
	else
		element.theme = ui->element_themes[type];
	*dst = element;

	if(dst->parent->children_tail)
	{
		dst->parent->children_tail->peer = dst;
	}
	dst->parent->children_tail = dst;



	return dst;
}


void ui_draw_box(UI_Box *e)
{
	UI *ui = e->ui;
	fvec2 a = ui_pixel_to_screen(ui, e->top_left);
	fvec2 b = ui_pixel_to_screen(ui, e->bottom_right);
	f32 ir = e->theme.corner_radius;
	f32 or = e->theme.corner_radius + e->theme.outline_thickness;
	u32 q = 32;
	gdraw_rounded_rectangle_outline(ui->vb, q,ir,or, a,b, e->theme.outline.color);
	gdraw_rounded_rectangle(ui->vb, q, ir, a,b, e->theme.background.color);
}

#include "collatz.h"

fmat3 ui_test(DeviceVertexBuffer *vb, const SimpleFont *simple_font, FrameEvents fe, uvec2 frame_size)
{
	static UI *ui = NULL;
	if(ui == NULL)
	{
		ui = allocate_ui(1024, &main_arena);	
		ui->simple_font = simple_font;
	}
	ui_next_frame(ui, frame_size, fe, vb);

	UI_Box *box = ui_box(ui->root, 0, 100, 100, 500, 500);
	//print("%str8\n", box->global_name);
	if(box)
	{
		ui_draw_box(box);
	}

	return ui->affine_matrix;
}


