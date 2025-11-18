#pragma once

#include "camera2.h"
#include "device_vertex_buffer.h"
#include "device_font.h"

// MISC GRAPHICS

void gdraw_camera2(Camera2 camera, DeviceVertexBuffer* vb);
typedef enum{
	CAMERA2_GRID_BOXES,
	CAMERA2_GRID_LINES,
}Camera2GridType;

void camera2_gdraw_grid(Camera2 camera, DeviceVertexBuffer* vb, Camera2GridType type);

// Immediate mode gdrawing ... appends all vertices to buffer. NO pipeline state change.
// These functions all use the Vertex2 triangle list pipeline, for simplicity.

/*
	TODO:
		struct RoundedTriangleMaker;
		gdraw_triangle();
		gdraw_triangle_outline();
		gdraw_rounded_triangle();
		gdraw_rounded_triangle_outline();
		
		struct RoundedLineMaker;
		gdraw_line_outline();
		gdraw_rounded_line_outline();
		gdraw_rounded_line_center_outline();

		struct RoundedLineStripMaker;
		gdraw_line_strip_outline();
		gdraw_rounded_line_strip();
		gdraw_rounded_line_strip_outline();

		struct RoundedRectangleMaker;
		gdraw_rectangle_outline();
		gdraw_rounded_rectangle_outline();

		struct RoundedQuadMaker;
		gdraw_rounded_quad();
		gdraw_rounded_quad_outline();

		// Rounded Miter Joints
*/


// Triangles

typedef enum{
	TRIANGLE_MODE_LIST,
	TRIANGLE_MODE_STRIP,
	TRIANGLE_MODE_FAN,
}TriangleMode;

void gdraw_fvec2_triangle_list(DeviceVertexBuffer *vb, u32 count, fvec2 *p, fvec4 color);
void gdraw_fvec2_triangle_strip(DeviceVertexBuffer *vb, u32 count, fvec2 *p, fvec4 color);
void gdraw_fvec2_triangle_fan(DeviceVertexBuffer *vb, u32 count, fvec2 *p, fvec4 color);

// Line

void gdraw_line(DeviceVertexBuffer *vb, f32 thickness, fvec2 a, fvec2 b, fvec4 color);
void gdraw_rounded_line(DeviceVertexBuffer *vb, u32 quality, f32 thickness, fvec2 a, fvec2 b, fvec4 color);
void gdraw_rounded_line_center(DeviceVertexBuffer *vb, u32 quality, f32 radius, fvec2 a, fvec2 b, fvec4 color);

void gdraw_line_strip(DeviceVertexBuffer* vb, f32 thickness, u32 point_count, const fvec2* points, fvec4 color);

// Rectangle

void gdraw_rectangle(DeviceVertexBuffer *vb, fvec2 a, fvec2 b, fvec4 color);
void gdraw_texture_rectangle(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, TextureCoordinate ta, TextureCoordinate tb, fvec4 color);
void gdraw_rounded_rectangle(DeviceVertexBuffer *vb, u32 arc_quality, f32 radius, fvec2 a, fvec2 b, fvec4 color);
void gdraw_rounded_rectangle2(DeviceVertexBuffer *vb, u32 arc_quality, fvec2 radius, fvec2 a, fvec2 b, fvec4 color);

// Quad

void gdraw_quad(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, fvec2 c, fvec2 d, fvec4 color);
void gdraw_texture_quad(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, fvec2 c, fvec2 d, TextureCoordinate ta, TextureCoordinate tb, TextureCoordinate tc, TextureCoordinate td, fvec4 color);

typedef struct{
	f32 angle;
	f32 delta_angle;
	f32 radius;
	u32 quality;
	fvec2 center;
}CircleMaker;

CircleMaker circle_make(u32 quality, f32 radius, fvec2 position);
fvec2 circle_make_next(CircleMaker *m);
void gdraw_circle(DeviceVertexBuffer* vb, u32 quality, f32 radius, fvec2 p, fvec4 color);
void gdraw_circle_outline(DeviceVertexBuffer* vb, u32 quality, f32 inner_radius, f32 outer_radius, fvec2 center, fvec4 color);

typedef struct{
	f32 angle;
	f32 delta_angle;
	fvec2 radius;
	u32 quality;
	fvec2 center;
}EllipseMaker;

EllipseMaker ellipse_make(u32 quality, fvec2 radius, fvec2 position);
fvec2 ellipse_make_next(EllipseMaker *m);
void gdraw_ellipse(DeviceVertexBuffer* vb, u32 quality, fvec2 radius, fvec2 p, fvec4 color);
void gdraw_ellipse_outline(DeviceVertexBuffer* vb, u32 quality, fvec2 inner_radius, fvec2 outer_radius, fvec2 center, fvec4 color);

typedef struct{
	f32 angle;
	f32 delta_angle;
	f32 radius;
	u32 quality;
	u32 index, major_index, major_increment;
	u32 itterations;
	fvec2 p;
	fvec2 ptable[4];
}RoundedRectangleMaker;

RoundedRectangleMaker rounded_rectangle_make(u32 quality, f32 radius, fvec2 a, fvec2 b);
fvec2 rounded_rectangle_make_next(RoundedRectangleMaker *m);
void gdraw_rounded_rectangle(DeviceVertexBuffer* vb, u32 quality, f32 radius, fvec2 a, fvec2 b, fvec4 color);
void gdraw_rounded_rectangle_outline(DeviceVertexBuffer* vb, u32 quality, f32 inner_radius, f32 outer_radius, fvec2 a, fvec2 b, fvec4 color);


// Font 
fvec2 gdraw_simple_glyph(DeviceVertexBuffer* vb, SimpleFont font, u8 c, fvec4 color, fvec2 pos, f32 height);
fvec2 gdraw_simple_text_box(DeviceVertexBuffer* vb, SimpleFont font, const u8* text, fvec4 color, fvec2 a, fvec2 b, fvec2 pos, f32 glyph_height);
void ddraw_whole_texture(DeviceVertexBuffer* vb, TextureCoordinate coordinate, fvec2 a, fvec2 b, fvec4 color);

void gdraw_test(DeviceVertexBuffer* vb, SimpleFont simple_font, Camera2 camera);
