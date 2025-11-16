#include "camera2.h"


fvec2 camera2_mouse_to_world(Camera2 camera, FrameEvents fe)
{
	fvec2 m = fvec2_make(fe.mouse_x, fe.mouse_y);
	m = fvec2_mul(m, camera.unit_pixel);
	m = fvec2_sub(m, fvec2_make(1.0, 1.0));
	m = camera2_inverse_transform(camera, m);
	return m;
}

fvec2 camera2_pixel_to_world(Camera2 camera, fvec2 m)
{
	m = fvec2_mul(m, camera.unit_pixel);
	m = fvec2_sub(m, fvec2_make(1.0, 1.0));
	m = camera2_inverse_transform(camera, m);
	return m;
}

Camera2 compute_camera2(Camera2 camera)
{
	camera.inverse_affine = fmat3_affine_inverse(camera.affine);

	{
		fvec2 a = fvec2_mul_fmat3(camera.aspect, camera.inverse_affine);
		fvec2 b = fvec2_mul_fmat3(fvec2_negate(camera.aspect), camera.inverse_affine);
		camera.center = fvec2_mul_fmat3(fvec2_zero(), camera.inverse_affine);
		camera.top_left = b;
		camera.top_right = fvec2_make(a.x, b.y);
		camera.bottom_left = fvec2_make(b.x, a.y);
		camera.bottom_right = a;
	}
	{
		fvec2 a = fvec2_inv(camera.aspect);
		fvec2 b = fvec2_inv(fvec2_negate(camera.aspect));
		camera.orig_center = fvec2_zero();
		camera.orig_top_left = b;
		camera.orig_top_right = fvec2_make(a.x, b.y);
		camera.orig_bottom_left = fvec2_make(b.x, a.y);
		camera.orig_bottom_right = a;
	}
	camera.mouse = camera2_pixel_to_world(camera, camera.pixel_mouse);

	return camera;
}


Camera2 make_camera2(uvec2 frame_size)
{
	Camera2 camera = {0};
	camera.frame_height = 1440.0; // just a constant.
	camera.dst_zoom = camera.src_zoom = camera.zoom = 1.0;
	camera.dst_pan = camera.src_pan = camera.pan = fvec2_make(0.0, 0.0);
	camera.transition_time = (1000000000 / 144) * 10;
	camera.t = 1.0;
	camera.reset_delay = 250000000;
	camera.unit_snorm = fvec2_make(2.0, 2.0);
	camera.unit_percent = fvec2_div(camera.unit_snorm, fvec2_make(100.0, 100.0));
	camera = resize_camera2(camera, frame_size);
	camera.pixel_mouse = fvec2_make(0.0, 0.0);
	camera.grid_color = fvec4_make(0.002, 0.002, 0.002, 1.0);
	camera.zoom_speed = 0.8;

	camera.dst_time = camera.src_time = get_time_ns();


	return camera;
}
Camera2 resize_camera2(Camera2 camera, uvec2 frame_size)
{
	camera.frame_size = fvec2_cast_uvec2(frame_size),	
	camera.frame_zoom = (camera.frame_height / camera.frame_size.y);
	camera.aspect_ratio = camera.frame_size.y / camera.frame_size.x;
	camera.unit_pixel = fvec2_div(camera.unit_snorm, camera.frame_size);

	camera.aspect = fvec2_scalar(1.0);
	camera.aspect.x *= camera.aspect_ratio;

	camera.affine = compute_affine_helper(camera.aspect, camera.zoom, camera.pan);
	camera.src_affine = compute_affine_helper(camera.aspect, camera.src_zoom, camera.src_pan);
	camera.dst_affine = compute_affine_helper(camera.aspect, camera.dst_zoom, camera.dst_pan);
	camera = compute_camera2(camera);

	return camera;
}

Camera2 update_camera2(Camera2 camera, FrameEvents fe, b32 pan_and_zoom)
{
	camera.pixel_mouse = fvec2_make(fe.mouse_x, fe.mouse_y);
	if(fe.w.pressed == true)
	{
		camera.dst_pan.y += 0.01;
	}
	if(pan_and_zoom == false)
	{
		camera = compute_camera2(camera);
		return camera;
	}
	b32 buttons_pressed = fe.w.pressed | fe.a.pressed | fe.s.pressed | fe.d.pressed;
	if(fe.mouse_left.pressed == true || buttons_pressed)
	{
		fvec2 mouse_delta = fvec2_make(fe.mouse_dx, fe.mouse_dy);
		fvec2 delta = mouse_delta;
		f64 dt = (f64)(fe.dt) / 1000000.0;
		f32 button_scale = 1.0 * (dt);
		if(fe.left_control.pressed == true)
		{
			button_scale *= 2.0;
		}
		delta.x += -((f32)fe.d.pressed)*button_scale;
		delta.x += ((f32)fe.a.pressed)*button_scale;
		delta.y += -((f32)fe.s.pressed)*button_scale;
		delta.y += ((f32)fe.w.pressed)*button_scale;
		{
			camera.last_move_time = fe.time;
			fvec2 pan;
			pan = fvec2_mul(delta, camera.unit_pixel);

			pan = fvec2_div(pan, fvec2_scalar_mul(camera.aspect, camera.dst_zoom));

			if(camera.t != 1.0 && fe.left_control.pressed == false){

				camera.dst_pan  = fvec2_add(camera.dst_pan, pan);

				{
					fvec2 affine_zoom = fvec2_scalar_mul(camera.aspect, camera.dst_zoom);
					fvec2 affine_pan = camera.dst_pan;
					camera.dst_affine = fmat3_mul(
						fmat3_affine_scale(affine_zoom.x, affine_zoom.y),
						fmat3_affine_translate(affine_pan.x, affine_pan.y)
					);
				}
			}

			else
			{

				camera.pan  = fvec2_add(camera.pan, pan);
				{
					fvec2 affine_zoom = fvec2_scalar_mul(camera.aspect, camera.zoom);
					fvec2 affine_pan = camera.pan;
					camera.affine = fmat3_mul(
						fmat3_affine_scale(affine_zoom.x, affine_zoom.y),
						fmat3_affine_translate(affine_pan.x, affine_pan.y)
					);
				}
				camera.src_pan = camera.dst_pan = camera.pan;
				camera.src_affine = camera.dst_affine = camera.affine;
			}
		}
	}
	if(fe.q.pressed || fe.e.pressed)
	{
		
		f32 dt = (f64)fe.dt / 1000000000.0;
		f32 speed = 1.0 - (dt * 2.0);
		f32 zoom = camera.zoom;
		if(fe.q.pressed)
		{
			zoom *= speed;
		}
		if(fe.e.pressed)
		{
			zoom = zoom / speed;
		}
		camera2_set_zoom(&camera, zoom);
	}

	s32 scroll_parity = fe.mouse_scroll;
	if(scroll_parity)
	{
		if(fe.left_shift.pressed == true)
		{
			camera.zoom_speed = 0.9;
		}
		else
		{
			camera.zoom_speed = 0.8;
		}
		f32 scroll = camera.zoom_speed;
		if(scroll_parity < 0)
		{
			scroll = 1.0 / scroll;		
		}
		fvec2 mouse = fvec2_make(fe.mouse_x, fe.mouse_y);
		mouse = fvec2_mul(mouse, camera.unit_pixel);
		mouse = fvec2_scalar_sub(mouse, 1.0);


		fvec2 before_mouse = fvec2_div(mouse, fvec2_scalar_mul(camera.aspect, camera.dst_zoom));
		camera.mouse_src_zoom = before_mouse;
		camera.dst_zoom = camera.dst_zoom * scroll;
		fvec2 after_mouse = fvec2_div(mouse, fvec2_scalar_mul(camera.aspect, camera.dst_zoom));
		fvec2 dm = fvec2_sub(after_mouse, before_mouse);
		if(fe.left_control.pressed == false)
		{
			camera.dst_pan = fvec2_add(camera.dst_pan, dm);
		}
	
		camera.src_time = fe.time;
		camera.dst_time = camera.src_time + camera.transition_time;

		if(camera.t != 1.0)
		{
			camera.src_pan = camera.pan;	
			camera.src_zoom = camera.zoom;	
			camera.src_affine = camera.affine;

		}
		else
		{
			camera.dst_affine = compute_affine_helper(camera.aspect, camera.dst_zoom, camera.dst_pan);
			camera.src_affine = compute_affine_helper(camera.aspect, camera.src_zoom, camera.src_pan);
		}
	}




	if(fe.time < camera.dst_time)
	{
		camera.last_move_time = fe.time;
		f32 t = 1.0 - (f64)(camera.dst_time - fe.time) / (f64)(camera.dst_time - camera.src_time);
		//print("%f32\n", t);

		t = lerp(t, 1.0, t);
		camera.t = t;

		
		fvec2 dmouse = fvec2_make(fe.mouse_dx, fe.mouse_dy);
		dmouse = fvec2_mul(dmouse, camera.unit_pixel);

		if(fe.mouse_left.pressed == false)
		{

			fvec2 before_dmouse = fvec2_div(dmouse, fvec2_scalar_mul(camera.aspect, camera.zoom));
			camera.zoom = lerp(camera.src_zoom, camera.dst_zoom, t);
			fvec2 after_dmouse = fvec2_div(dmouse, fvec2_scalar_mul(camera.aspect, camera.zoom));
			fvec2 dm = fvec2_add(after_dmouse, before_dmouse);
			dm = fvec2_scalar_mul(dm, (1.0 - t));
			camera.dst_pan = fvec2_sub(camera.dst_pan, dm);

		}

		camera.dst_affine = compute_affine_helper(camera.aspect, camera.dst_zoom, camera.dst_pan);

		camera.zoom = lerp(camera.src_zoom, camera.dst_zoom, camera.t);
		camera.pan = fvec2_lerp(camera.src_pan, camera.dst_pan, camera.t);
		camera.affine = fmat3_lerp(camera.src_affine, camera.dst_affine, camera.t);

	}
	else if(camera.t != 1.0)
	{
		camera.pan = camera.src_pan = camera.dst_pan;
		camera.zoom = camera.src_zoom = camera.dst_zoom;
		camera.affine = camera.src_affine = camera.dst_affine;
		camera.t = 1.0;
	}
	else
	{
		camera.affine = compute_affine_helper(camera.aspect, camera.zoom, camera.pan);
	}

	if(fe.f3.pressed == true)
	{
		if(fe.f3.press_time != camera.reset_time)
		{
			if(camera.reset == false)
			{
				camera.pan_before_reset = camera.pan;
				camera2_set_pan(&camera, fvec2_make(0.0, 0.0));
				camera.zoom_before_reset = camera.zoom;
				camera2_set_zoom(&camera, 1.0);
				camera.reset = true;
				camera.reset_time = fe.f3.press_time;
			}
			else
			{
				camera2_set_zoom(&camera, camera.zoom_before_reset);
				camera.reset = false;
				camera2_set_pan(&camera, camera.pan_before_reset);
				camera.reset = false;
			}
			camera.reset_time = fe.f3.press_time;
		}
	}

	camera = compute_camera2(camera);

	return camera;
}



fvec2 camera2_transform(Camera2 camera, fvec2 in)
{
	in = fvec2_mul_fmat3(in, camera.affine);	
	return in;
}

fvec2 camera2_inverse_transform(Camera2 camera, fvec2 in)
{
	in = fvec2_mul_fmat3(in, camera.inverse_affine);	
	return in;
}

fmat3 compute_affine_helper(fvec2 zoom_aspect, f32 zoom, fvec2 pan)
{
	fvec2 affine_zoom = fvec2_scalar_mul(zoom_aspect, zoom);
	fvec2 affine_pan = pan;
	fmat3 affine = fmat3_mul(
		fmat3_affine_scale(affine_zoom.x, affine_zoom.y),
		fmat3_affine_translate(affine_pan.x, affine_pan.y)
	);
	return affine;
}

f32 camera2_set_zoom(Camera2* camera, f32 zoom)
{
	camera->zoom = zoom;
	camera->dst_zoom = zoom;
	camera->src_zoom = zoom;

	camera->affine = compute_affine_helper(camera->aspect, camera->zoom, camera->pan);
	camera->src_affine = compute_affine_helper(camera->aspect, camera->src_zoom, camera->src_pan);
	camera->dst_affine = compute_affine_helper(camera->aspect, camera->dst_zoom, camera->dst_pan);

	return zoom;
}

fvec2 camera2_set_pan(Camera2* camera, fvec2 pan)
{
	camera->pan = pan;
	camera->dst_pan = pan;
	camera->src_pan = pan;

	camera->affine = compute_affine_helper(camera->aspect, camera->zoom, camera->pan);
	camera->src_affine = compute_affine_helper(camera->aspect, camera->src_zoom, camera->src_pan);
	camera->dst_affine = compute_affine_helper(camera->aspect, camera->dst_zoom, camera->dst_pan);


	return pan;

}

