
#include "device_graphics.h"


fvec2 simple_font_compute_box_size(SimpleFont font, f32 height)
{
	uvec2 u_box_size = font.max_box;
	f32 scale = height / (f32)u_box_size.y;
	fvec2 box_size = fvec2_scalar_mul(fvec2_cast_uvec2(u_box_size), scale);
	f32 line_gap = (f32)font.line_gap * scale;
	return box_size;
}


fvec2 gdraw_simple_glyph(DeviceVertexBuffer* vb, SimpleFont font, u8 c, fvec4 color, fvec2 pos, f32 height)
{
	if(!char_is_printable(c))
	{
		return (fvec2){0.0};
	}
	Vertex2 v[6];

	fvec2 a, b;
	fvec2 ret;

	u32 char_index = c - 33;

	uvec2 u_box_size = font.max_box;
	f32 scale = height / (f32)u_box_size.y;
	fvec2 box_size = fvec2_scalar_mul(fvec2_cast_uvec2(u_box_size), scale);
	f32 line_gap = (f32)font.line_gap * scale;
	ret = box_size;

	

	if(c == ' ')
	{
		return ret;
	}

	TextureRectangle r = font.ascii_glyphs[char_index].rectangle;
	uvec2 u_glyph_size = uvec2_make(r.x1 - r.x0, r.y1 - r.y0);
	fvec2 glyph_size = fvec2_scalar_mul(fvec2_cast_uvec2(u_glyph_size), scale);

	//u32 u_advance = font.ascii_glyphs[char_index].advance;
	svec2 s_bearing = font.ascii_glyphs[char_index].bearing;
	fvec2 bearing = fvec2_scalar_mul(fvec2_cast_svec2(s_bearing), scale);
	//f32 advance = (f32)u_advance * scale;

	a = pos;
	a.y -= bearing.y;
	a.y += box_size.y;
	//a.y += line_gap;
	a.x += bearing.x;
	b = fvec2_add(a, glyph_size);

	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;

	v[0].position = a;
	v[1].position = fvec2_make(b.x, a.y);
	v[2].position = b;
	v[3].position = fvec2_make(a.x, b.y);

	v[0].texture = (TextureCoordinate){r.x0,r.y0,r.i};
	v[1].texture = (TextureCoordinate){r.x1,r.y0,r.i};
	v[2].texture = (TextureCoordinate){r.x1,r.y1,r.i};
	v[3].texture = (TextureCoordinate){r.x0,r.y1,r.i};

	u16 indices[6] = {0,1,2,0,3,2};
	vertex_buffer_draw_indexed(vb, 6, indices, 4, v);


	return ret;
}

fvec2 gdraw_simple_text_box(DeviceVertexBuffer* vb, SimpleFont font, const u8* text, fvec4 color, fvec2 a, fvec2 b, fvec2 pos, f32 glyph_height)
{
	fvec2 box = fvec2_sub(b,a);
	fvec2 delta = simple_font_compute_box_size(font, glyph_height);
	fvec2 cell_count = fvec2_div(box, delta);
	while(*text)
	{
		if(char_is_printable(*text))
		{
			gdraw_simple_glyph(vb, font, *text, color, pos, glyph_height);

			pos.x += delta.x;
			if(pos.x + delta.x > b.x && pos.x)
			{
				pos.x = a.x ;
				pos.y += delta.y;
			}
		}
		else if(*text == '\n')
		{
			pos.y += glyph_height;
			pos.x = a.x;
		}
		else if(*text == '\t')
		{
			f32 m = fmod(pos.x - a.x, delta.x * 4);
			pos.x -= m;
			pos.x += 4 * delta.x;
			if(pos.x + delta.x > b.x && pos.x)
			{
				pos.x = a.x ;
				pos.y += delta.y;
			}
		}
		text++;
	}
	return pos;
}

void gdraw_fvec2_triangle_list(DeviceVertexBuffer *vb, u32 count, fvec2 *p, fvec4 color)
{
	Temp temp = begin_temp(0);
	Vertex2* vertices = arena_alloc(sizeof(Vertex2) * count,0,0, temp.arena);
	for(u32 i = 0; i < count; i++)
	{
		vertices[i] = (Vertex2){
			.color = color,
			.position = p[i],
			.texture = texcord_disable(),
		};
	}
	vertex_buffer_draw(vb, count, vertices);
	end_temp(temp);
}
void gdraw_fvec2_triangle_strip(DeviceVertexBuffer *vb, u32 vertex_count, fvec2 *p, fvec4 color)
{
	Temp temp = begin_temp(0);
	if(vertex_count < 3){return;}
	Vertex2* vertices = arena_alloc(sizeof(Vertex2) * vertex_count,0,0, temp.arena);
	for(u32 i = 0; i < vertex_count; i++)
	{
		vertices[i] = (Vertex2){
			.color = color,
			.position = p[i],
			.texture = texcord_disable(),
		};
	}
	u32 index_count = (vertex_count-2) * 3;
	u16* indices = arena_alloc(sizeof(u16) * index_count,0,0, temp.arena);
	u32 i = 0;
	for(u32 v = 0; v < vertex_count-2; v++)
	{
		indices[i++] = v;
		indices[i++] = v+1;
		indices[i++] = v+2;
	}
	vertex_buffer_draw_indexed(vb, index_count, indices, vertex_count, vertices);
	end_temp(temp);
	
}
void gdraw_fvec2_triangle_fan(DeviceVertexBuffer *vb, u32 count, fvec2 *p, fvec4 color)
{
	if(count < 3)
	{
		return;
	}
	Temp temp = begin_temp(0);
	Vertex2* vertices = arena_alloc(sizeof(Vertex2) * count, 0,0,temp.arena);
	for(u32 i = 0; i < count; i++)
	{
		vertices[i] = (Vertex2){
			.color = color,
			.position = p[i],
			.texture = texcord_disable(),
		};
	}
		
	u32 triangle_count = (count - 2);
	u32 index_count = triangle_count * 3;
	u16* indices = arena_alloc(index_count * sizeof(u16), 0,0,temp.arena);

	u32 index = 0;
	for(u32 i = 0; i < triangle_count; i++)
	{
		indices[index++] = 0;
		indices[index++] = i + 1;	
		indices[index++] = i + 2;	
	}

	vertex_buffer_draw_indexed(vb, index_count, indices, count, vertices);
	end_temp(temp);
}

void gdraw_line(DeviceVertexBuffer *vb, f32 thickness, fvec2 a, fvec2 b, fvec4 color)
{
	fvec2 n = fvec2_normal2(a, b);
	n = fvec2_scalar_mul(n, thickness);
	fvec2 p[4];
	p[0] = fvec2_add(a, fvec2_make(-n.x, -n.y));
	p[1] = fvec2_add(a, fvec2_make(n.x, n.y));
	p[2] = fvec2_add(b, fvec2_make(n.x, n.y));
	p[3] = fvec2_add(b, fvec2_make(-n.x, -n.y));
	gdraw_quad(vb, p[0], p[1], p[2], p[3], color);
}

void gdraw_rounded_line_center(DeviceVertexBuffer *vb, u32 quality, f32 radius, fvec2 a, fvec2 b, fvec4 color)
{
	fvec2 u = fvec2_unit2(a, b);
	u = fvec2_scalar_mul(u,radius);
	a = fvec2_sub(a,u);
	b = fvec2_add(b,u);
	gdraw_rounded_line(vb, quality, radius, a, b, color);
}

void gdraw_rounded_line(DeviceVertexBuffer *vb, u32 quality, f32 radius, fvec2 a, fvec2 b, fvec4 color)
{
	quality>>=1;
	if(fvec2_distance(a,b) < radius*2)
	{
		fvec2 p = fvec2_scalar_mul(fvec2_add(a,b), 0.5);
		gdraw_circle(vb, quality<<1, radius, p, color);
		return;
	}
	quality--;
	fvec2 n = fvec2_normal2(a, b);
	fvec2 u = fvec2_unit2(a, b);
	f32 ra = fvec2_atan(n);	
	f32 rb = ra + 3.141592;
	n = fvec2_scalar_mul(n, radius);
	f32 dr = (2 * 3.1415926) / ((f32)quality * 2);

	fvec2 c[2];
	c[0] = fvec2_add(a, fvec2_scalar_mul(u, radius));
	c[1] = fvec2_add(b, fvec2_scalar_mul(u, -radius));
	
	
	u32 point_count = quality * 2+2; // overkill
	Temp temp = begin_temp(0);
	fvec2* points = arena_alloc(point_count * sizeof(fvec2), 0,0,temp.arena);
	u32 ii = 0;

	f32 r = ra;
	for(u32 ci = 0; ci < 2; ci++)
	{
		fvec2 center = c[ci];
		for(u32 i = 0; i <= quality; i++)
		{
			fvec2 p;
			p = fvec2_make(cosf(r), sinf(r));
			r += dr;
			p = fvec2_scalar_mul(p, radius);
			p = fvec2_add(p, center);
			points[ii++] = p;
		}
		r -= dr;
	}

	gdraw_fvec2_triangle_fan(vb, point_count, points, color);
	end_temp(temp);
}

void gdraw_vertex2t_quad(DeviceVertexBuffer* vb, Vertex2 t[4])
{
	u16 indices[6] = {
		0,1,2,0,3,2	
	};
	vertex_buffer_draw_indexed(vb, 6, indices, 4, t);
}

void gdraw_quad(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, fvec2 c, fvec2 d, fvec4 color)
{
	gdraw_texture_quad(vb, 
		a, b, c, d,
		texcord_disable(),texcord_disable(),texcord_disable(),texcord_disable(),
		color
	);
}
void gdraw_texture_quad(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, fvec2 c, fvec2 d, TextureCoordinate ta, TextureCoordinate tb, TextureCoordinate tc, TextureCoordinate td, fvec4 color)
{
	Vertex2 t[4];		
	t[0].position = a;
	t[0].color = color;
	t[0].texture = ta;
	t[1].position = b;
	t[1].color = color;
	t[1].texture = tb;
	t[2].position = c;
	t[2].color = color;
	t[2].texture = tc;
	t[3].position = d;
	t[3].color = color;
	t[3].texture = td;
	gdraw_vertex2t_quad(vb, t);
}

void gdraw_rectangle(DeviceVertexBuffer *vb, fvec2 a, fvec2 b, fvec4 color)
{
	gdraw_texture_rectangle(vb, a, b, texcord_disable(), texcord_disable(), color);
}

void gdraw_whole_texture(DeviceVertexBuffer* vb, TextureCoordinate tc, fvec2 a, fvec2 b, fvec4 color)
{
	TextureCoordinate ta,tb;
	ta = tc;
	tb = tc;
	ta.x = 0; ta.y=0;
	gdraw_texture_rectangle(vb, a,b,ta,tb, color);
}

void gdraw_texture_rectangle(DeviceVertexBuffer* vb, fvec2 a, fvec2 b, TextureCoordinate ta, TextureCoordinate tb, fvec4 color)
{
	Vertex2 t[4];
	t[0].color = color;
	t[0].position = a;
	t[0].texture = ta;

	t[1].color = color;
	t[1].position = fvec2_make(b.x, a.y);
	t[1].texture = ta;
	t[1].texture.x = tb.x;

	t[2].color = color;
	t[2].position = b;
	t[2].texture = tb;

	t[3].color = color;
	t[3].position = fvec2_make(a.x, b.y);
	t[3].texture = tb;
	t[3].texture.x = ta.x;
	gdraw_vertex2t_quad(vb, t);
}
#include "math.h"





CircleMaker circle_make(u32 quality, f32 radius, fvec2 position)
{
	CircleMaker maker = {
		.angle = 0.0,
		.delta_angle = (2*3.141592)/(f32)quality,
		.radius = radius,
		.quality = quality,
		.center = position,
	};
	return maker;
}

fvec2 circle_make_next(CircleMaker *m)
{
	f32 x = m->radius * cosf(m->angle);
	f32 y = m->radius * sinf(m->angle);
	fvec2 p = fvec2_add(m->center, fvec2_make(x,y));
	m->angle += m->delta_angle;
	return p;
}

void gdraw_circle(DeviceVertexBuffer* vb, u32 quality, f32 radius, fvec2 p, fvec4 color)
{
	Temp temp = begin_temp(0);
	CircleMaker m = circle_make(quality, radius, p);

	fvec2* points = arena_alloc(sizeof(fvec2) * quality, 0,0,temp.arena);
	for(u32 i = 0; i < quality; i++)
	{
		points[i] = circle_make_next(&m);

	}
	gdraw_fvec2_triangle_fan(vb, quality, points, color);
	end_temp(temp);
}

void gdraw_circle_outline(DeviceVertexBuffer* vb, u32 quality, f32 inner_radius, f32 outer_radius, fvec2 center, fvec4 color)
{
	Temp temp = begin_temp(0);
	CircleMaker m_outer = circle_make(quality, outer_radius, center);
	CircleMaker m_inner = circle_make(quality, inner_radius, center);
	m_inner.angle += m_inner.delta_angle * 0.5;

	u32 point_count = quality * 2+2;
	fvec2* points = arena_alloc(sizeof(fvec2) * point_count, 0,0,temp.arena);

	for(u32 i = 0; i < point_count; i++)
	{
		if(i&1)
		{
			points[i] = circle_make_next(&m_inner);
		}
		else
		{
			points[i] = circle_make_next(&m_outer);
		}

	}
	gdraw_fvec2_triangle_strip(vb, point_count, points, color);
	end_temp(temp);
}


EllipseMaker ellipse_make(u32 quality, fvec2 radius, fvec2 position)
{
	EllipseMaker maker = {
		.angle = 0.0,
		.delta_angle = (2*3.141592)/(f32)quality,
		.radius = radius,
		.quality = quality,
		.center = position,
	};
	return maker;
}

fvec2 ellipse_make_next(EllipseMaker *m)
{
	f32 x = m->radius.x * cosf(m->angle);
	f32 y = m->radius.y * sinf(m->angle);
	fvec2 p = fvec2_add(m->center, fvec2_make(x,y));
	m->angle += m->delta_angle;
	return p;
}

void gdraw_ellipse(DeviceVertexBuffer* vb, u32 quality, fvec2 radius, fvec2 p, fvec4 color)
{
	Temp temp = begin_temp(0);
	EllipseMaker m = ellipse_make(quality, radius, p);

	fvec2* points = arena_alloc(sizeof(fvec2) * quality,0,0, temp.arena);
	for(u32 i = 0; i < quality; i++)
	{
		points[i] = ellipse_make_next(&m);

	}
	gdraw_fvec2_triangle_fan(vb, quality, points, color);
	end_temp(temp);
}

void gdraw_ellipse_outline(DeviceVertexBuffer* vb, u32 quality, fvec2 inner_radius, fvec2 outer_radius, fvec2 center, fvec4 color)
{
	Temp temp = begin_temp(0);
	EllipseMaker m_outer = ellipse_make(quality, outer_radius, center);
	EllipseMaker m_inner = ellipse_make(quality, inner_radius, center);
	m_inner.angle += m_inner.delta_angle * 0.5;

	u32 point_count = quality * 2+2;
	fvec2* points = arena_alloc(sizeof(fvec2) * point_count,0,0, temp.arena);

	for(u32 i = 0; i < point_count; i++)
	{
		if(i&1)
		{
			points[i] = ellipse_make_next(&m_inner);
		}
		else
		{
			points[i] = ellipse_make_next(&m_outer);
		}

	}
	gdraw_fvec2_triangle_strip(vb, point_count, points, color);
	end_temp(temp);
}


RoundedRectangleMaker rounded_rectangle_make(u32 quality, f32 radius, fvec2 a, fvec2 b)
{
	a = fvec2_scalar_add(a, radius);
	b = fvec2_scalar_add(b, -radius);
	quality *= 4;
	RoundedRectangleMaker m = {
		.angle = 0.0f,
		.delta_angle = (2 * 3.1415926) / (f32)(quality),
		.radius = radius, 
		.index = 0,
		.major_index = (quality) / 4, 
		.major_increment = quality / 4,
		.itterations = quality + 4,
		.p = b,
		.ptable[0] = fvec2_make(b.x, b.y),
		.ptable[1] = fvec2_make(a.x, b.y),
		.ptable[2] = fvec2_make(a.x, a.y),
		.ptable[3] = fvec2_make(b.x, a.y),

	};
	return m;
}
fvec2 rounded_rectangle_make_next(RoundedRectangleMaker *m)
{
	f32 x = cosf(m->angle) * m->radius;
	f32 y = sinf(m->angle) * m->radius;
	fvec2 p = fvec2_make(x,y);
	p = fvec2_add(p, m->p);
	if((m->index == m->major_index))
	{
		m->p = m->ptable[(m->major_index / m->major_increment) & 3];
		m->major_index += m->major_increment;
		m->angle -= m->delta_angle;
		m->index--;	
	}
	m->index++;	
	m->angle += m->delta_angle;
	return p;
}
void gdraw_rounded_rectangle(DeviceVertexBuffer *vb, u32 quality, f32 radius, fvec2 a, fvec2 b, fvec4 color)
{
	Temp temp = begin_temp(0);

	RoundedRectangleMaker m = rounded_rectangle_make(quality, radius, a, b);
	fvec2* points = arena_alloc(sizeof(fvec2) * m.itterations, 0,0,temp.arena);
	for(u32 i = 0; i < m.itterations; i++)
	{
		points[i] = rounded_rectangle_make_next(&m);	
	}
	gdraw_fvec2_triangle_fan(vb, m.itterations, points, color);
	end_temp(temp);
}

void gdraw_rounded_rectangle_outline(DeviceVertexBuffer* vb, u32 quality, f32 radius, f32 thickness, fvec2 a, fvec2 b, fvec4 color)
{
	Temp temp = begin_temp(0);

	fvec2 ta, tb;;
	ta.x = fmin(a.x,b.x);
	ta.y = fmin(a.y,b.y);
	tb.x = fmax(a.x,b.x);
	tb.y = fmax(a.y,b.y);
	a = ta;
	b = tb;



	RoundedRectangleMaker m_inner = rounded_rectangle_make(quality, radius, a, b);
	a = fvec2_scalar_add(a, -thickness);
	b = fvec2_scalar_add(b, thickness);
	RoundedRectangleMaker m_outer = rounded_rectangle_make(quality, radius + thickness/2, a,b);
	u32 itt = m_inner.itterations * 2 + 2;
	fvec2* points = arena_alloc(sizeof(fvec2) * itt, 0,0,temp.arena);
	for(u32 i = 0; i < itt; i++)
	{
		if(i & 1)
		{
			points[i] = rounded_rectangle_make_next(&m_outer);	
		}
		else
		{
			points[i] = rounded_rectangle_make_next(&m_inner);	
		}
	}
	gdraw_fvec2_triangle_strip(vb, itt, points, color);
	end_temp(temp);
}



void gdraw_line_strip(DeviceVertexBuffer* vb, f32 thickness, u32 point_count, const fvec2* points, fvec4 color)
{
	if(point_count == 0)
	{
		return;
	}
	else if(point_count == 1)
	{
		return;	
	}
	else if(point_count == 2)
	{
		gdraw_line(vb, thickness, points[0], points[1], color);
		return;
	}
	thickness = thickness * 0.5;
	Temp temp = begin_temp(0);
	u32 vertex_count = point_count * 2;
	fvec2* vertices = arena_alloc(sizeof(fvec2) * vertex_count, 0,0,temp.arena);

	u32 vi = 0;
	{
		fvec2 normal = fvec2_normal2(points[0], points[1]);
		vertices[vi++] = fvec2_add(points[0], fvec2_scalar_mul(normal, thickness));
		vertices[vi++] = fvec2_add(points[0], fvec2_scalar_mul(normal, -thickness));
	}
	for(u32 i = 1; i < point_count-1; i++)
	{
		fvec2 u1 = fvec2_unit2(points[i-1], points[i]);

		fvec2 n1 = fvec2_normal2(points[i-1], points[i]); 
		fvec2 n2 = fvec2_normal2(points[i], points[i+1]);

		fvec2 miter = fvec2_unit(fvec2_add(n1, n2));
		
		f32 miter_thickness = thickness / fabs(fvec2_dot(miter, n1)); // could be n1 or n2
		if(miter_thickness < thickness * 16)
		{
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(miter, miter_thickness));
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(miter, -miter_thickness));
		}
		else
		{
			vertex_count+=2;
			arena_alloc(sizeof(fvec2) * 2, 1,0,temp.arena);
			fvec2 normal;
			normal = n1;
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(normal, thickness));
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(normal, -thickness));

			normal = n2;
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(normal, thickness));
			vertices[vi++] = fvec2_add(points[i], fvec2_scalar_mul(normal, -thickness));
		}


	}
	{
		fvec2 normal = fvec2_normal2(points[point_count-2], points[point_count-1]);
		vertices[vi++] = fvec2_add(points[point_count-1], fvec2_scalar_mul(normal, thickness));
		vertices[vi++] = fvec2_add(points[point_count-1], fvec2_scalar_mul(normal, -thickness));
	}
	
	gdraw_fvec2_triangle_strip(vb, vertex_count, vertices, color);
	end_temp(temp);
}


void gdraw_test(DeviceVertexBuffer* vb, SimpleFont simple_font, Camera2 camera)
{
	PRNG rg = init_prng(12320);
	f32 mul = 1.0;
	f32 thickness = 0.1;
	u32 point_count = 1000;


	f32 t = (f32)(get_time_ms() % 1000) / 1000.0;
	t *= 6.28;
	
	u64 si = (get_time_ms()/1000) % (u64)point_count;
	for(u32 ii = 0; ii < 36; ii++)
	{
		fvec4 color;
		{
			color = fvec4_make(1.0, 0.1, 0.7, 1.0);
			color.a = 1.0;
		}
		fvec2 offset = fvec2_make(ii * 10000 % 60000, (ii / 6) * 10000);
		fvec2 pos = fvec2_make(0,0);
		f32 scale = 100.0;
		scale *= fabs(random_f32(&rg));
		fvec2 bias = fvec2_make(0.1 * cos(t), 0.1 * cos(t));

		rg = init_prng(2304 + ii * 12023);
		Temp temp = begin_temp(0);
		fvec2* points = arena_alloc(sizeof(fvec2) * point_count,0,0, temp.arena);

		for(u32 xi = 0; xi < 10; xi++)
		{
			for(u32 i = 0; i < point_count; i++)
			{
				u64 r = random_u64(&rg);
				fvec2 rv = fvec2_scalar_mul(fvec2_make(random_f32(&rg) ,random_f32(&rg)), mul);
				fvec2 p = rv;
				p = fvec2_scalar_mul(p, scale);
				p = fvec2_add(p, bias);
				points[i] = fvec2_add(offset, pos);
				pos = fvec2_add(pos, p);

			}
			gdraw_line_strip(vb, scale / 10, point_count, points, color);
		}
		end_temp(temp);
	}
}
/*

{
	if(radius == 0.f)
	{
		gdraw_rectangle(vb, a, b, color);
		return;
	}
	f32 r2 = radius * 2;
	if(r2 > (b.x - a.x) || r2 > (b.y - a.y))
	{
		// Too small for radius
		gdraw_rectangle(vb, a, b, color);
		return;
	}

	Temp temp = begin_temp(0);
	u32 total_quality = quality * 4; 
	f32 dr = (2 * 3.1415926) / (f32)total_quality;
	fvec2 c[4];
	c[0] = fvec2_make(a.x + radius, a.y + radius);
	c[1] = fvec2_make(b.x - radius, a.y + radius);
	c[2] = fvec2_make(b.x - radius, b.y - radius);
	c[3] = fvec2_make(a.x + radius, b.y - radius);
	u32 point_count = total_quality + 4;
	fvec2* points = arena_alloc(sizeof(fvec2) * point_count,0,0, temp.arena);
	u32 pi = 0;
	u32 ii = 0;
	f32 r = -3.1415926;
	for(u32 ci = 0; ci < 4; ci++)
	{
		fvec2 center = c[ci];
		for(u32 i = 0; i <= quality; i++)
		{
			fvec2 p;
			p = fvec2_make(cosf(r), sinf(r));
			r += dr;
			p = fvec2_scalar_mul(p, radius);
			p = fvec2_add(p, center);
			points[ii++] = p;
		}
		r -= dr;
	}
	gdraw_fvec2_triangle_fan(vb, point_count, points, color);
	end_temp(temp);
}
*/

// CAMERA2

void camera2_gdraw_grid(Camera2 camera, DeviceVertexBuffer* vb, Camera2GridType type)
{
	f64 zoom = camera.zoom;
	u32 count = 0;
	f64 ds[3];
	ds[0] = powf(10, floorf(log10(1.0 / camera.zoom)) - 1.0);
	ds[1] = powf(10, floorf(log10(1.0 / camera.zoom)));
	ds[2] = powf(10, ceilf(log10(1.0 / camera.zoom)));
	u32 ds_count = arrlen(ds);

	for(u32 i = 0; i < ds_count; i++)
	{
		f64 d = ds[i] * zoom;
		fvec2 a = camera.orig_top_left;
		fvec2 b = camera.orig_bottom_right;
		fvec2 c = camera.center;
		c= fvec2_scalar_mul(c, zoom);
		
		fvec4 color = camera.grid_color;
		f64 alpha = 1.0;
		color.a = alpha;

		a.x -= fmod(a.x + c.x, d)+d;
		a.y -= fmod(a.y + c.y, d)+d;
		b.x -= fmod(b.x - c.x, d)-d;
		b.y -= fmod(b.y - c.y, d)-d;

		switch(type)
		{
			default:
			case CAMERA2_GRID_LINES:
			{
				f64 thickness = 0.001;
				for(f64 x = a.x; x < b.x; x+=d)
				{
					fvec2 aa = fvec2_make(x, a.y);
					fvec2 bb = fvec2_make(x, b.y);
					gdraw_line(vb, thickness, aa, bb, color);
				}
				for(f64 y = a.y; y < b.y; y+=d)
				{
					fvec2 aa = fvec2_make(a.x, y);
					fvec2 bb = fvec2_make(b.x, y);
					gdraw_line(vb, thickness, aa, bb, color);
				}
			}
			break;
			case CAMERA2_GRID_BOXES:
			{
				for(f64 x = a.x; x < b.x; x+=d)
				{
					for(f64 y = a.y; y < b.y; y+=d)
					{
						count++;
						fvec2 p = fvec2_make(x,y);
						f64 padding = d / 10;
						fvec2 aa = fvec2_scalar_add(p, padding);
						fvec2 bb = fvec2_scalar_add(p, d - padding);
						//gdraw_rounded_rectangle(vb,10, padding / 10, aa, bb, color);
						gdraw_rectangle(vb, aa, bb, color);
					}
				}
			}
			break;
		} // switch
	} // for
}

/*
void gdraw_camera2(Camera2 camera, DeviceVertexBuffer* vb)
{
	fvec2 pos = fvec2_make(0.0, 0.0);
	{
		fvec2 p1 = camera2_transform(camera, fvec2_make(0.0, 0.0));
		fvec2 p2 = camera2_inverse_transform(camera, fvec2_make(0.0, 0.0));
		p1 = camera.orig_center;
		p2 = camera.center;
		gdraw_circle(vb, 16, 0.005 / camera.zoom, p1, fvec4_make(0.0, 1.0, 0.0, 1.0));

		gdraw_circle(vb, 16, 0.005, camera.orig_top_left, fvec4_make(1.0, 1.0, 1.0, 1.0));
		gdraw_circle(vb, 16, 0.005, camera.orig_top_right, fvec4_make(1.0, 1.0, 1.0, 1.0));
		gdraw_circle(vb, 16, 0.005, camera.orig_bottom_left, fvec4_make(1.0, 1.0, 1.0, 1.0));
		gdraw_circle(vb, 16, 0.005, camera.orig_bottom_right, fvec4_make(1.0, 1.0, 1.0, 1.0));

		gdraw_circle(vb, 16, 0.005 / camera.zoom, p2, fvec4_make(1.0, 0.0, 0.0, 1.0));
		if(fvec2_distance(p1, p2) > 1.0 / camera.zoom / 2)
		{
			fvec2 p3 = fvec2_unit(fvec2_sub(p1, p2));
			p3 = fvec2_scalar_mul(p3, 0.02 / camera.zoom);
			p3 = fvec2_add(p3, p2);
			gdraw_circle(vb, 16, 0.001 / camera.zoom, p3, fvec4_make(1.0, 0.0, 0.0, 1.0));
		}
	}
}
*/

