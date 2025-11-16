#pragma once
#include "event.h"

typedef struct{
	fvec2 frame_size;
	f32 aspect_ratio; // rise over run
	fvec2 aspect;
	f32 frame_zoom;
	f32 frame_height;

	u64 transition_time;
	u64 src_time;
	u64 dst_time;
	f32 zoom_speed;

	u64 reset_delay;
	b32 reset;
	f32 zoom_before_reset;
	fvec2 pan_before_reset;
	u64 reset_time;
	u64 last_move_time;


	fvec2 mouse_src_zoom;
	f32 t;

	f32 zoom;
	f32 src_zoom;
	f32 dst_zoom;

	fvec2 pan;	
	fvec2 dst_pan;
	fvec2 src_pan;

	fvec4 grid_color;

	fmat3 src_affine;
	fmat3 dst_affine;

	fmat3 affine;
	fmat3 inverse_affine;

	fvec2 orig_top_left;
	fvec2 orig_top_right;
	fvec2 orig_bottom_left;
	fvec2 orig_bottom_right;
	fvec2 orig_center;

	fvec2 top_left;
	fvec2 top_right;
	fvec2 bottom_left;
	fvec2 bottom_right;
	fvec2 center;

	fvec2 pixel_mouse;
	fvec2 mouse;

	fvec2 unit_snorm; // (2,2)
	fvec2 unit_pixel; 
	fvec2 unit_percent;
}Camera2;

fvec2 camera2_mouse_to_world(Camera2 camera, FrameEvents fe);
fvec2 camera2_pixel_to_world(Camera2 camera, fvec2 pos);

Camera2 make_camera2(uvec2 frame_size);
Camera2 resize_camera2(Camera2 camera, uvec2 frame_size);
Camera2 update_camera2(Camera2 camera, FrameEvents fe, b32 pan_and_zoom);

fvec2 camera2_transform(Camera2 camera, fvec2 in);
fvec2 camera2_inverse_transform(Camera2 camera, fvec2 in);

fmat3 compute_affine_helper(fvec2 zoom_aspect, f32 zoom, fvec2 pan);

f32 camera2_set_zoom(Camera2* camera, f32 zoom);
fvec2 camera2_set_pan(Camera2* camera, fvec2 pan);

