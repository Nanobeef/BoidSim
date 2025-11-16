#include "device_geometry.h"


b32 point_vs_rectangle(fvec2 pos, fvec2 a, fvec2 b)
{
	return ((pos.x > a.x && pos.y > a.y) && (pos.x < b.x && pos.y < b.y));
}

b32 point_vs_ellipse(fvec2 p, fvec2 c, fvec2 r)
{
	f32 x = powf(((p.x - c.x) / r.x), 2.0);
	f32 y = powf(((p.y - c.y) / r.y), 2.0);
	if(x + y < 1.0)
	{
		return true;
	}
	return false;
}

b32 point_vs_rounded_rectangle(fvec2 pos, fvec2 a, fvec2 b, fvec2 radius)
{
	if(point_vs_rectangle(pos, a,b))
	{
		if(((b.x - a.x) < (radius.x * 2)) || ((b.y - a.y) < (radius.y * 2)))
		{
			// Too small for radius
			return true;
		}

		fvec2 c = fvec2_scalar_mul(fvec2_add(a,b), 0.5);
		if(pos.x < c.x)
		{
			if(pos.y < c.y)
			{ 
				// top_left
				fvec2 pp = fvec2_add(a, radius);
				if(pos.x < pp.x && pos.y < pp.y)
				{
					return point_vs_ellipse(pos, pp, radius);
				}
				return true;
			}
			else
			{ 
				// bottom_left 
				fvec2 pp = fvec2_add(fvec2_make(a.x, b.y), fvec2_make(radius.x, -radius.y));
				if(pos.x < pp.x && pos.y > pp.y)
				{
					return point_vs_ellipse(pos, pp, radius);
				}
				return true;
			}
		}
		else
		{
			if(pos.y < c.y)
			{ 
				// top_right
				fvec2 pp = fvec2_add(fvec2_make(b.x, a.y), fvec2_make(-radius.x, radius.y));
				if(pos.x > pp.x && pos.y < pp.y)
				{
					return point_vs_ellipse(pos, pp, radius);
				}
				return true;
					
			}
			else
			{ 
				// bottom_right
				fvec2 pp = fvec2_add(fvec2_make(b.x, b.y), fvec2_make(-radius.x, -radius.y));
				if(pos.x > pp.x && pos.y > pp.y)
				{
					return point_vs_ellipse(pos, pp, radius);
				}
				return true;

			}

		}
		return true;
	}
	return false;
}

TextureCoordinate texcord_disable()
{
	return (TextureCoordinate){0,0,texture_index_disable};
}

TextureCoordinate texcord_not_ready()
{
	return (TextureCoordinate){0,0,texture_index_not_ready};
}


LinmaxRectanglePack linmax_rectangle_pack_init(u32 width, u32 height)
{
	LinmaxRectanglePack ret = {
		.width = width,
		.height = height,
	};
	return ret;
}

Rectangle linmax_rectangle_pack(LinmaxRectanglePack* p, u32 width, u32 height)
{
	Rectangle rect = {0};
	if(p->horizontal_position + width > p->width)
	{
		if(p->vertical_position + height > p->height)
		{
			return rect;
		}
		else
		{
			p->vertical_position += p->row_max_height;
			p->horizontal_position = 0;
			p->row_max_height = 0;
		}
	}
	rect.n = p->vertical_position + p->y;
	rect.s = p->vertical_position + height + p->y;
	rect.w = p->horizontal_position + p->x;
	rect.e = p->horizontal_position + width + p->x;
	p->horizontal_position += width;
	p->row_max_height = defmax(p->row_max_height, height);
	return rect;
}

