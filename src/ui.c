#include "ui.h"
#include "embed.h"



static fvec4 gold_color = { 0.94, 0.62, 0.054, 1.0};

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

UI_Element *ui_translate(UI_Element *e, f32 x, f32 y)
{
	e->top_left.x += x;
	e->bottom_right.x += x;
	e->top_left.y += x;
	e->bottom_right.y += x;
	return e;
}
UI_Element *ui_resize(UI_Element *e, f32 x, f32 y)
{
	e->bottom_right.x = e->top_left.x + x;
	e->bottom_right.y = e->top_left.y + y;
	return e;
}

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
		.text = ui_color(255.0/256.0, 253.0/256.0, 208.0/256.0, 1.0),
		.foreground = ui_color(0.6, 0.6, 0.6, 0.5),
		.background = ui_color(0.0, 0.0, 0.0, 0.9),
		.outline = ui_color(0.6, 0.6, 0.6, 1.0),
		.hover_tint = ui_color(0.05, 0.05, 0.05, 0.05),
		.states[0] = ui_color(0.01, 0.01, 0.01, 0.0),
		.states[1].enabled = true,
		.corner_radius = 8,
		.outline_thickness = 0.75,
		.text_height = 14.0,
	};
	ui->element_themes[UI_ELEMENT_BUTTON] = ui->element_themes[UI_ELEMENT_BOX];
	ui->element_themes[UI_ELEMENT_BUTTON].corner_radius = 2;
	ui->element_themes[UI_ELEMENT_BUTTON].outline_thickness = 0.5;
	ui->element_themes[UI_ELEMENT_BUTTON].states[1].color = fvec4_sub(gold_color, fvec4_make(0.2, 0.2, 0.2, 0.0));

	ui->element_themes[UI_ELEMENT_SLIDER] = ui->element_themes[UI_ELEMENT_BUTTON];
	ui->element_themes[UI_ELEMENT_SLIDER].background = ui_color(0.0, 0.0, 0.0, 0.0);
	ui->element_themes[UI_ELEMENT_SLIDER].slider_width = 12;

	return ui;
}

UI* ui_next_frame(UI *ui, uvec2 frame_size, FrameEvents fe)
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
	ui->time = get_time_ns();

	if(0)
	{
		memzero(ui->element_maps[ui->frame_index], sizeof(UI_Element) * ui->max_element_count);
	}
	else
	{
		for(u32 i = 0; i < ui->max_element_count; i++){ui->element_maps[ui->frame_index][i].type = UI_ELEMENT_NULL;}
	}


	ui->fe = fe;

	reset_arena(&ui->frame_arenas[ui->frame_index]);
	ui->arena = &ui->frame_arenas[ui->frame_index];

	ui->prng = init_prng(1232543);

	ui->root->children_tail = 0;
	ui->root->peer = 0;
	ui->root->children = 0;
	ui->root->state.hovering = true;

	ui->focused = false;
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

#define ui_button(PE, NAME, X0, Y0, X1, Y1, ...)\
({\
	UI_Element *e = ui_element(PE, str8_lit(NAME), (fvec2){X0,Y0}, (fvec2){X1,Y1}, UI_ELEMENT_BUTTON);\
	if(e)\
	{\
		UI *__ui = (PE)->ui;\
		_Pragma("GCC diagnostic push");\
		_Pragma("GCC diagnostic ignored \"-Woverride-init-side-effects\"");\
		e->button = (struct UI_ElementButton){\
			__VA_ARGS__\
		};\
		_Pragma("GCC diagnostic pop");\
	}\
	e;\
})

#define ui_slider(PE, NAME, X0, Y0, X1, Y1, ...)\
({\
	UI_Element *e = ui_element(PE, str8_lit(NAME), (fvec2){X0,Y0}, (fvec2){X1,Y1}, UI_ELEMENT_SLIDER);\
	UI_Element *button = ui_button(PE, 0, X0, Y0, X1, Y1);\
	if(e)\
	{\
		UI *__ui = (PE)->ui;\
		_Pragma("GCC diagnostic push");\
		_Pragma("GCC diagnostic ignored \"-Woverride-init-side-effects\"");\
		e->slider = (struct UI_ElementSlider){\
			__VA_ARGS__\
			.button = button,\
		};\
		_Pragma("GCC diagnostic pop");\
	}\
	ui_resize(button, e->theme.slider_width, (Y1-Y0));\
	ui_compute_slider(e);\
	e;\
})

UI_Element *ui_element(UI_Element *pe, String8 name, fvec2 top_left, fvec2 bottom_right, UI_ElementType type)
{
	if(pe == 0)
	{
		print("Every UI element must have a parent!");
		return 0;
	}

	top_left = fvec2_add(pe->top_left, top_left);
	bottom_right = fvec2_add(pe->top_left, bottom_right);

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
		state.norm_ptr = 0;
	}
	UI_Element element = {
		.type = type,
		.ui = ui,
		.frame_accum = ui->frame_accum,
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

	if(dst->parent->children)
	{
		dst->parent->children_tail->peer = dst;
		dst->parent->children_tail = dst;
	}
	else
	{
		dst->parent->children_tail = dst;
		dst->parent->children = dst;
	}
	return dst;
}

void ui_draw_box(UI_Box *e)
{
	UI *ui = e->ui;
	f32 pixel_size = ui->frame_norm_pixel_size.y;
	fvec2 a = ui_pixel_to_screen(ui, e->top_left);
	fvec2 b = ui_pixel_to_screen(ui, e->bottom_right);
	f32 ir = e->theme.corner_radius * pixel_size;
	f32 or = (e->theme.outline_thickness) * pixel_size;
	u32 q = 32;

	gdraw_rounded_rectangle_outline(ui->vb, q,ir,or, a,b, e->theme.outline.color);
	gdraw_rounded_rectangle(ui->vb, q, ir, a,b, e->theme.background.color);

	if(e->text.str)
	{
		gdraw_simple_text_box(ui->vb, *ui->simple_font, e->text.str, e->theme.text.color, a, b, a, (e->theme.text_height * pixel_size));
	}
}

void ui_draw_button(UI_Box *e)
{
	UI *ui = e->ui;
	f32 pixel_size = ui->frame_norm_pixel_size.y;
	fvec2 a = ui_pixel_to_screen(ui, e->top_left);
	fvec2 b = ui_pixel_to_screen(ui, e->bottom_right);
	f32 ir = e->theme.corner_radius * pixel_size;
	f32 or = (e->theme.outline_thickness) * pixel_size;
	u32 q = 32;



	if(e->theme.outline.enabled)
	{
		gdraw_rounded_rectangle_outline(ui->vb, q,ir,or, a,b, e->theme.outline.color);
	}
	if(e->theme.background.enabled)
	{
		fvec4 background = e->theme.background.color;
		if(e->state.hovering)
		{
			background = fvec4_add(background, e->theme.hover_tint.color);
		}
		background = fvec4_add(background, e->theme.states[e->state.pressed].color);
		gdraw_rounded_rectangle(ui->vb, q, ir, a,b, background);
	}
	if(e->text.str)
	{
		gdraw_simple_text_box(ui->vb, *ui->simple_font, e->text.str, e->theme.text.color, a, b, a, (e->theme.text_height * pixel_size));
	}

}

f32 inv_lerp_fvec2(fvec2 a, fvec2 b, fvec2 p)
{
	f32 major = fvec2_distance(a,b);	
	f32 minor = fvec2_distance(a,p);	
	return minor / major;
}

f32 project_point_to_line(fvec2 a, fvec2 b, fvec2 p)
{
	fvec2 d = fvec2_sub(b,a);	
	f32 denom = fvec2_magnitude_squared(d);
	if(denom == 0.0f)
	{
		return 0.0f;
	}
	f32 t = ((p.x - a.x) * d.x + (p.y - a.y) * d.y) / denom;
	t = fmax(fmin(t, 1.0), 0.0);
	return t;
}


void ui_draw_slider(UI_Slider*e)
{
	UI *ui = e->ui;
	f32 pixel_size = ui->frame_norm_pixel_size.y;
	f32 ir = e->theme.corner_radius * pixel_size;
	f32 or = (e->theme.outline_thickness) * pixel_size;
	u32 q = 32;

	{
		fvec2 a = ui_pixel_to_screen(ui, e->top_left);
		fvec2 b = ui_pixel_to_screen(ui, e->bottom_right);
		gdraw_rounded_rectangle_outline(ui->vb, q,ir,or, a,b, e->theme.outline.color);
		gdraw_rounded_rectangle(ui->vb, q, ir, a,b, e->theme.background.color);

		if(e->text.str)
		{
			gdraw_simple_text_box(ui->vb, *ui->simple_font, e->text.str, e->theme.text.color, a, b, a, (e->theme.text_height * pixel_size));
		}
	}
}


void ui_draw_element(UI_Element *e)
{
	switch(e->type)
	{
		case UI_ELEMENT_BOX: ui_draw_box(e); break;
		case UI_ELEMENT_BUTTON: ui_draw_button(e); break;
		case UI_ELEMENT_SLIDER: ui_draw_slider(e); break;
		default:break;
	}
	
	if(e->children)
	{
		ui_draw_element(e->children);
	}
	if(e->peer)
	{
		ui_draw_element(e->peer);
	}
}

void ui_draw(UI *ui, const SimpleFont *simple_font, DeviceVertexBuffer *vb)
{
	ui->vb = vb;
	ui->simple_font = simple_font;
	// The maximum number of elements is fixed and small, a stack overflow is unlikely.
	ui_draw_element(ui->root);
}

void ui_poll_box(UI_Box *e, b32 check_bounds)
{
	UI *ui = e->ui;
	if(e->state.pressed)
	{
		ui->focused = true;
		if(ui->fe.mouse_left.pressed == false)
		{
			e->state.pressed = false;
		}
		e->state.first_pressed = false;
	}
	if(e->state.hovering)
	{
		e->state.first_hovering = false;
	}
	if(check_bounds)
	{
		fvec2 mouse = fvec2_make(ui->fe.mouse_x, ui->fe.mouse_y);
		if(point_vs_rounded_rectangle(mouse,e->top_left, e->bottom_right, fvec2_scalar(e->theme.corner_radius)))
		{
			if(e->state.hovering == false)
			{
				e->state.first_hovering = true;
			}
			e->state.hovering = true;
			if(ui->fe.mouse_left.pressed)
			{
				if(e->state.pressed == false)
				{
					e->state.pressed = true;	
					e->state.pressed_position = mouse;
					e->state.first_pressed = true;
				}
			}
			else
			{
				e->state.pressed = false;
			}
		}
		else
		{
			e->state.hovering = false;
		}
	}
	else
	{
		e->state.hovering = false;
	}
}

void ui_poll_button(UI_Box *e, b32 check_bounds)
{
	UI *ui = e->ui;
	if(e->state.pressed_ptr == 0)
	{
		e->state.pressed_ptr = &e->state.pressed;
	}
	if(e->state.first_pressed_ptr == 0)
	{
		e->state.first_pressed_ptr = &e->state.first_pressed;
	}
	if(e->state.first_hovering_ptr == 0)
	{
		e->state.first_hovering_ptr = &e->state.first_hovering;
	}
	if(e->state.hovering_ptr == 0)
	{
		e->state.hovering_ptr = &e->state.hovering;
	}
	if(e->state.pressed)
	{
		ui->focused = true;
		if(ui->fe.mouse_left.pressed == false)
		{
			e->state.pressed = e->state.pressed_ptr[0] = false;
		}
		e->state.first_pressed_ptr[0] = e->state.first_pressed = false;
	}
	if(e->state.hovering)
	{
		e->state.first_hovering_ptr[0] = e->state.first_hovering = false;
	}
	if(check_bounds)
	{
		fvec2 mouse = fvec2_make(ui->fe.mouse_x, ui->fe.mouse_y);
		if(point_vs_rounded_rectangle(mouse,e->top_left, e->bottom_right, fvec2_scalar(e->theme.corner_radius)))
		{
			if(e->state.hovering == false)
			{
				e->state.first_hovering_ptr[0] = e->state.first_hovering = true;
			}
			e->state.hovering = true;
			if(ui->fe.mouse_left.pressed)
			{
				if(e->state.pressed == false)
				{
					e->state.pressed = e->state.pressed_ptr[0] = true;
					e->state.pressed_position = mouse;
					e->state.first_pressed = e->state.first_pressed_ptr[0] = true;
				}
			}
			else
			{
				e->state.pressed = false;
				e->state.pressed_ptr = false;
			}
		}
		else
		{
			e->state.hovering = e->state.hovering_ptr[0] = false;
		}
	}
	else
	{
		e->state.hovering = e->state.hovering_ptr[0] = false;
	}
}


void ui_compute_slider(UI_Slider *e)
{
	UI_Button *button = e->slider.button;
	fvec2 center = fvec2_scalar_div(fvec2_add(button->top_left, button->bottom_right), 2.0);
	fvec2 size = fvec2_sub(button->bottom_right, button->top_left);
	f32 end_padding = e->theme.slider_width/2.0;
	fvec2 line_a = fvec2_make(e->top_left.x + end_padding, center.y);
	fvec2 line_b = fvec2_make(e->bottom_right.x - end_padding, center.y);
	fvec2 tp = fvec2_lerp(line_a,  line_b, e->state.norm);
	fvec2 half_size = fvec2_scalar_div(size, 2.0);
	button->top_left = fvec2_sub(tp, half_size);
	button->bottom_right = fvec2_add(tp, half_size);
}

void ui_poll_slider(UI_Slider *e, b32 check_bounds)
{
	UI *ui = e->ui;
	b32 value_changed = false;
	if(e->state.norm_ptr == 0)
	{
		e->state.norm_ptr = &e->state.norm;
	}
	else
	{
		value_changed = true;
		e->state.norm = e->state.norm_ptr[0];
	}

	UI_Button *button = e->slider.button;
	e->state.first_pressed = e->slider.button->state.first_pressed;
	e->state.pressed = button->state.pressed;

	fvec2 center = fvec2_scalar_div(fvec2_add(button->top_left, button->bottom_right), 2.0);
	if(value_changed || ui->fe.mouse_left.pressed)
	{
		if(e->state.first_pressed)
		{
			e->state.pressed_position = button->state.pressed_position;
			e->state.pressed_position = fvec2_sub(center, e->state.pressed_position);
		}
		fvec2 size = fvec2_sub(button->bottom_right, button->top_left);
		f32 end_padding = e->theme.slider_width/2.0;
		fvec2 line_a = fvec2_make(e->top_left.x + end_padding, center.y);
		fvec2 line_b = fvec2_make(e->bottom_right.x - end_padding, center.y);

		// Offset position of press to center
		fvec2 pressed_spot;
		if(e->state.pressed)
		{
			fvec2 mouse = fvec2_make(ui->fe.mouse_x, ui->fe.mouse_y);
			pressed_spot = fvec2_add(mouse, e->state.pressed_position);
			e->state.norm_ptr[0] = e->state.norm = project_point_to_line(line_a, line_b, pressed_spot);
		}
		else
		{
			pressed_spot = fvec2_lerp(line_a, line_b, e->state.norm);
		}
		ui_compute_slider(e);
	}

}

void ui_poll_element(UI_Element *e, b32 check_bounds)
{
	switch(e->type)
	{
		case UI_ELEMENT_BOX: ui_poll_box(e, check_bounds); 
		break;
		case UI_ELEMENT_BUTTON: ui_poll_button(e, check_bounds); 
		break;
		case UI_ELEMENT_SLIDER: ui_poll_slider(e, check_bounds); 
		break;
		default:break;
	}

	//print("%str8\n", e->global_name);

	if(e->children)
	{
		ui_poll_element(e->children, e->state.hovering);
	}
	if(e->peer)
	{
		ui_poll_element(e->peer, e->parent->state.hovering);
	}
}

void ui_poll(UI *ui)
{
	ui_poll_element(ui->root, true);
}



UI *init_ui_test()
{
	UI *ui = allocate_ui(1024, &main_arena);	
	return ui;
}


UI *poll_ui_test(UI *ui, FrameEvents fe, uvec2 frame_size, BoidSim *sim)
{
	ui_next_frame(ui, frame_size, fe);
	
	UI_Box *box = 0;




	UI_Slider *range_sliders[3];
	UI_Slider *strength_sliders[3];
	UI_Slider *speed_sliders[3];
	UI_Slider *misc_sliders[3];

	struct GlobalBoidParams *bp = &THREAD->global.boid;
	{
		f32 box_height = 96;
		f32 box_width = 800;
		f32 vertical_padding = 16;
		f32 margin = (((f32)frame_size.x - box_width) / 2.0);
		//f32 margin = 24;
		fvec2 top_left = 		fvec2_make(margin, (f32)frame_size.y - box_height - vertical_padding);
		fvec2 bottom_right = 	fvec2_make(margin + box_width, (f32)frame_size.y - vertical_padding);
		f32 horizontal_center = (top_left.x + bottom_right.x) / 2.0;

		box = ui_box(ui->root, 0, top_left.x, top_left.y, bottom_right.x, bottom_right.y);
		UI_Button *button = 0;

		fvec2 box_size = fvec2_sub(box->bottom_right, box->top_left);
		f32 left_x = box_size.x;


		{
			f32 size = 16.0;
			f32 xpad = 8, ypad = 8;
			f32 width = 160;
			left_x -= xpad+width;
			f32 ypos = ypad;
			for(u32 i = 0; i < arrlen(range_sliders); i++)
			{
				f32 x0 = left_x;
				f32 y0 = ypos;
				f32 x1 = x0 + width;
				f32 y1 = y0 + size;
				range_sliders[i] = ui_slider(box,0, x0,y0,x1,y1);
				ypos += ypad *2+ size;
			}
		}


		{
			f32 size = 16.0;
			f32 xpad = 8, ypad = 8;
			f32 width = 160;
			left_x -= xpad+width;
			f32 ypos = ypad;
			for(u32 i = 0; i < arrlen(strength_sliders); i++)
			{
				f32 x0 = left_x;
				f32 y0 = ypos;
				f32 x1 = x0 + width;
				f32 y1 = y0 + size;
				strength_sliders[i] = ui_slider(box,0, x0,y0,x1,y1);
				ypos += ypad *2+ size;
			}
		}

		left_x = 0;
		{
			f32 size = 16.0;
			f32 xpad = 8, ypad = 8;
			f32 width = 100;
			f32 ypos = ypad;
			left_x += xpad;
			for(u32 i = 0; i < arrlen(speed_sliders); i++)
			{
				f32 x0 = left_x;
				f32 y0 = ypos;
				f32 x1 = x0 + width;
				f32 y1 = y0 + size;
				speed_sliders[i] = ui_slider(box,0, x0,y0,x1,y1);
				ypos += ypad *2+ size;
			}
			left_x += width;
		}

		{
			f32 size = 16.0;
			f32 xpad = 8, ypad = 8;
			f32 width = 100;
			f32 ypos = ypad;
			left_x += xpad;
			for(u32 i = 0; i < arrlen(misc_sliders); i++)
			{
				f32 x0 = left_x;
				f32 y0 = ypos;
				f32 x1 = x0 + width;
				f32 y1 = y0 + size;
				misc_sliders[i] = ui_slider(box,0, x0,y0,x1,y1);
				ypos += ypad *2+ size;
			}
			left_x += width;
		}


		u32 info_left_x = left_x;

		u32 top_y = 0;
		{
			f32 size = 16.0;
			f32 xpad = 12, ypad = 8;
			f32 width = 64;
			f32 ypos = ypad;
			left_x += xpad;
			info_left_x += 8;
			f32 x0 = left_x;
			f32 y0 = ypos;
			f32 x1 = x0 + width;
			f32 y1 = y0 + size;
			UI_Button *bump_button = ui_button(box,0, x0,y0,x1,y1);
			bump_button->text = str8_lit("  Bump");
			bump_button->state.pressed_ptr = &bp->bump_enable;
			left_x += width;

		}
		{
			f32 size = 16.0;
			f32 xpad = 12, ypad = 8;
			f32 width = 64;
			f32 ypos = ypad;
			left_x += xpad;
			f32 x0 = left_x;
			f32 y0 = ypos;
			f32 x1 = x0 + width;
			f32 y1 = y0 + size;
			UI_Button *time_button = ui_button(box,0, x0,y0,x1,y1);
			time_button->text = str8_lit("  Time");
			if(time_button->state.first_pressed)
			{
				bp->show_time = !bp->show_time;		
			}
			left_x += width;

			if(time_button->state.hovering || bp->show_time){

				u32 thread_time_divisor = (sim->boid_count) / sim->thread_count;
				UI_Box *time_box = ui_box(ui->root, 0, 8, 8, 256, 520);
				time_box->theme.text_height = 18;
				time_box->text = string_print(ui->arena, 
					"Boid Count %u32K\n"
					"FPS: %f32\n"
					"UPS: %f32\n"
					"Update:     %u64 ns\n"
					" Count:     %u64 ns\n"
					" Alloc:     %u64 ns\n"
					" Fill:      %u64 ns\n"
					" Construct: %u64 ns\n"
					" Resolve:   %u64 ns\n"
					"Reset:      %u64 ns\n"
					"\n"
					"Thread Time per Boid (%u64)\n"
					"Update:     %u64 ns\n"
					" Count:     %u64 ns\n"
					" Alloc:     %u64 ns\n"
					" Fill:      %u64 ns\n"
					" Construct: %u64 ns\n"
					" Resolve:   %u64 ns\n"
					"Reset:      %u64 ns\n"
					"\n"
					"Goal:(1M at 144ups) (7ms)\n"
					"Update:     %u32 ns\n"
					" Count:     %u32 ns (zero)\n"
					" Alloc:     %u32 ns\n"
					" Fill:      %u32 ns\n"
					" Construct: %u32 ns\n"
					" Resolve:   %u32 ns\n"
					"Reset:      %u32 ns\n"
					"\n"
					,
					(u64)sim->boid_count / (1024),

					1000000000.0f / (f32)MAIN_THREAD->global.frame_time,
					1000000000.0f / (f32)sim->elapsed_time,
					
					sim->elapsed_time / 1000,
					sim->stage_times[BOID_SIM_STAGE_COUNT] / 1000,
					sim->stage_times[BOID_SIM_STAGE_ALLOCATE] / 1000,
					sim->stage_times[BOID_SIM_STAGE_FILL] / 1000,
					sim->stage_times[BOID_SIM_STAGE_CONSTRUCT] / 1000,
					sim->stage_times[BOID_SIM_STAGE_RESOLVE] / 1000,
					sim->stage_times[BOID_SIM_STAGE_RESET] / 1000,

					sim->thread_count,

					sim->elapsed_time / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_COUNT] / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_ALLOCATE] / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_FILL] / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_CONSTRUCT] / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_RESOLVE] / thread_time_divisor,
					sim->stage_times[BOID_SIM_STAGE_RESET] / thread_time_divisor,

					224,
					0,
					10,
					10,
					30,
					150,
					50
				);
			}
		}
		{
			f32 size = 16.0;
			f32 xpad = 12, ypad = 8;
			f32 width = 64;
			f32 ypos = ypad;
			left_x += xpad;
			f32 x0 = left_x;
			f32 y0 = ypos;
			f32 x1 = x0 + width;
			f32 y1 = y0 + size;
			UI_Button *controls_button = ui_button(box,0, x0,y0,x1,y1);

			if(controls_button->state.first_pressed)
			{
				bp->show_controls = !bp->show_controls;
			}


			controls_button->text = str8_lit("Controls");
			if(controls_button->state.hovering || bp->show_controls)
			{
				u32 offset = box_height + 8;
				u32 width = 400;
				u32 height = 600 - (bottom_right.y - top_left.y);

				UI_Box *controls_box = ui_box(ui->root, 0, horizontal_center - (width/2), top_left.y - offset - height, horizontal_center + (width/2), bottom_right.y - offset);
				controls_box->theme.background.color.a = 1.0;
				controls_box->text = string_print(ui->arena, 
				"Exit: (Escape)\n"
				"\n"
				"Camera\n"
				"  Pan: (Left Click)\n"
				"  Zoom: (Wheel)\n"
				"  Move: (W A S D)\n"
				"  Zoom: (Q E)\n"
				"  Fast Move: (Left Control)\n"
				"  Slow Move: (Left Shift)\n"
				"  Toggle Reset: (F3)\n"
				" Enable Pixel Zoom: (M)\n"
				"\n"
				"Graphics\n"
				" Triangles:\n"
				"  Normal: (O + 1) or (O + 0)\n"
				"  Debug: (O + 2)\n"
				"  Opaque: (O + 3)\n"
				" Toggle Lines: (I)\n"
				" Multisample Count:\n"
				"  Max: (P + 0)\n"
				"  Other: [(P + 1) (P + 2) (P + 3) ...     ]\n"
				"         [   1       2       4    8,16,32 ]\n"
				"\n"
				"Boid Simulation\n"
				" Reset: (R)\n"
				" Clear Area: (Right Click)\n"
				" Most sliders are non-linear.\n"
				"\n"
				"-----------------------------------\n"
				"\n"
				"Info\n"
				" Release: PROTOTYPE\n"
				" Renderer: Vulkan 1.0\n"
				" OS: Linux\n"
				" Window: X11\n"
				" Libraries:\n"
				"  FreeType 2\n"
				"  Simd Everywhere\n"
				" Font: Liberation Mono\n"
				"\n"
				"Made in U.S.A.\n"
				);
			}
			top_y += size;
			left_x = info_left_x;

		}

		{
			f32 size = 58.0;
			f32 xpad = 8, ypad = 8;
			f32 width = 232;
			f32 ypos = ypad;
			left_x += xpad;
			f32 x0 = left_x - xpad;
			f32 y0 = ypos + top_y + ypad;
			f32 x1 = x0 + width;
			f32 y1 = y0 + size;
			UI_Button *count_box  = ui_box(box,0, x0,y0,x1,y1);

			count_box->text = string_print(ui->frame_arenas, 
				"Boid Count:   %u32 (static)\n"
				"Thread Count: 32     (static)\n"
				"\n"	
				"  Eli Eichner | November 2025"
				,bp->boid_count_uint);
			count_box->state.pressed_ptr = &bp->bump_enable;
			left_x += width;
			count_box->theme.corner_radius = 2.0;
		}

		range_sliders[0]->text = str8_lit("Seperation Radius");
		range_sliders[0]->state.norm_ptr = &bp->seperation_range_norm;
		range_sliders[1]->text = str8_lit("Cohesion Radius");
		range_sliders[1]->state.norm_ptr = &bp->cohesion_range_norm;
		range_sliders[2]->text = str8_lit("Alignment Radius");
		range_sliders[2]->state.norm_ptr = &bp->alignment_range_norm;

		strength_sliders[0]->text = str8_lit("Seperation Strength");
		strength_sliders[0]->state.norm_ptr = &bp->seperation_strength_norm;
		strength_sliders[1]->text = str8_lit("Cohesion Strength");
		strength_sliders[1]->state.norm_ptr = &bp->cohesion_strength_norm;
		strength_sliders[2]->text = str8_lit("Alignment Strength");
		strength_sliders[2]->state.norm_ptr = &bp->alignment_strength_norm;

		speed_sliders[0]->text = str8_lit("Min Speed");
		speed_sliders[0]->state.norm_ptr = &bp->min_speed_norm;
		speed_sliders[1]->text = str8_lit("Max Speed");
		speed_sliders[1]->state.norm_ptr = &bp->max_speed_norm;
		speed_sliders[2]->text = str8_lit("Acceleration");
		speed_sliders[2]->state.norm_ptr = &bp->acceleration_norm;

		misc_sliders[0]->text = str8_lit("Boid Size");
		misc_sliders[0]->state.norm_ptr = &bp->boid_size_norm;
		misc_sliders[1]->text = str8_lit("Randomness");
		misc_sliders[1]->state.norm_ptr = &bp->randomness_norm;
		misc_sliders[2]->text = str8_lit("Pen Radius");
		misc_sliders[2]->state.norm_ptr = &bp->attractor_range_norm;

		ui_poll(ui);
	}
	{
		CursorType cursor = CURSOR_POINTER;
		if((ui->focused == false) && (box->state.hovering == false))
		{
			if(fe.mouse_left.pressed)
			{
				cursor = CURSOR_HAND;
			}
			else
			{
				cursor = CURSOR_X11_GOBBLER;
			}
		}
		Event e = {
			.type = EVENT_SET_CURSOR,
			.set_cursor.type = cursor,
			.set_cursor.foreground_color = gold_color,
		};
		post_event(fe.event_ring_buffer, e);
	}
	return ui;
}

UI *draw_ui_test(UI *ui, DeviceVertexBuffer *vb, const SimpleFont *simple_font)
{
	ui_draw(ui, simple_font, vb);
	return ui;
}




