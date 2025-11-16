#include "math.h"

#include "basic.h"

#include <math.h>

f32 lerp(f32 a, f32 b, f32 t)
{
	return (b-a) * t + a;
}

uvec2 uvec2_make(u32 a, u32 b)
{
	return (uvec2){a,b};
}
uvec2 uvec2_scalar(u32 a)
{
	return (uvec2){a,a};
}
uvec2 uvec2_zero()
{
	return (uvec2){0.0f, 0.0f};
}
uvec2 uvec2_one()
{
	return (uvec2){1.0f, 1.0f};
}

// vector and scalar operations
uvec2 uvec2_scalar_add(uvec2 a, u32 s)
{
	return uvec2_make(a.x + s, a.y + s);
}
uvec2 uvec2_scalar_sub(uvec2 a, u32 s)
{
	return uvec2_make(a.x - s, a.y - s);
}
uvec2 uvec2_scalar_mul(uvec2 a, u32 s)
{
	return uvec2_make(a.x * s, a.y * s);
}
uvec2 uvec2_scalar_div(uvec2 a, u32 s)
{
	return uvec2_make(a.x / s, a.y / s);
}
uvec2 uvec2_scalar_mod(uvec2 a, u32 s)
{
	return uvec2_make(fmod(a.x, s), fmod(a.y, s));
}
uvec2 uvec2_scalar_max(uvec2 a, u32 s)
{
	return uvec2_make(defmax(a.x, s), defmax(a.y, s));
}
uvec2 uvec2_scalar_min(uvec2 a, u32 s)
{
	return uvec2_make(defmin(a.x, s), defmin(a.y, s));
}

// component wise operations 
uvec2 uvec2_add(uvec2 a, uvec2 b)
{
	return uvec2_make(a.x + b.x, a.y + b.y);
}
uvec2 uvec2_sub(uvec2 a, uvec2 b)
{
	return uvec2_make(a.x - b.x, a.y - b.y);
}
uvec2 uvec2_mul(uvec2 a, uvec2 b)
{
	return uvec2_make(a.x * b.x, a.y * b.y);
}
uvec2 uvec2_div(uvec2 a, uvec2 b)
{
	return uvec2_make(a.x / b.x, a.y / b.y);
}
uvec2 uvec2_mod(uvec2 a, uvec2 b)
{
	return uvec2_make(fmod(a.x, b.x), fmod(a.y, b.y));
}
uvec2 uvec2_max(uvec2 a, uvec2 b)
{
	return uvec2_make(defmax(a.x, b.x), defmax(a.y, b.y));
}
uvec2 uvec2_min(uvec2 a, uvec2 b)
{
	return uvec2_make(defmin(a.x, b.x), defmin(a.y, b.y));
}

// signed

u32 uvec2_area(uvec2 a)
{
	return a.x * a.y;
}
uvec2 uvec2_flip(uvec2 a)
{
	f32 t = a.x;
	a.x = a.y;
	a.y = t;
	return a;
}

b32 uvec2_equal(uvec2 a, uvec2 b)
{
	return (a.x ==  b.x) && (a.y == b.y);
}

uvec2 uvec2_rsh(uvec2 v, u32 sh)
{
	v.x >>= sh;
	v.y >>= sh;
	return v;
}
uvec2 uvec2_lsh(uvec2 v, u32 sh)
{
	v.x <<= sh;
	v.y <<= sh;
	return v;
}

svec2 svec2_make(s32 a, s32 b)
{
	return (svec2){a,b};
}
svec2 svec2_scalar(s32 a)
{
	return (svec2){a,a};
}
svec2 svec2_zero()
{
	return (svec2){0.0f, 0.0f};
}
svec2 svec2_one()
{
	return (svec2){1.0f, 1.0f};
}

// vector and scalar operations
svec2 svec2_scalar_add(svec2 a, s32 s)
{
	return svec2_make(a.x + s, a.y + s);
}
svec2 svec2_scalar_sub(svec2 a, s32 s)
{
	return svec2_make(a.x - s, a.y - s);
}
svec2 svec2_scalar_mul(svec2 a, s32 s)
{
	return svec2_make(a.x * s, a.y * s);
}
svec2 svec2_scalar_div(svec2 a, s32 s)
{
	return svec2_make(a.x / s, a.y / s);
}
svec2 svec2_scalar_mod(svec2 a, s32 s)
{
	return svec2_make(fmod(a.x, s), fmod(a.y, s));
}
svec2 svec2_scalar_max(svec2 a, s32 s)
{
	return svec2_make(defmax(a.x, s), defmax(a.y, s));
}
svec2 svec2_scalar_min(svec2 a, s32 s)
{
	return svec2_make(defmin(a.x, s), defmin(a.y, s));
}

// component wise operations 
svec2 svec2_add(svec2 a, svec2 b)
{
	return svec2_make(a.x + b.x, a.y + b.y);
}
svec2 svec2_sub(svec2 a, svec2 b)
{
	return svec2_make(a.x - b.x, a.y - b.y);
}
svec2 svec2_mul(svec2 a, svec2 b)
{
	return svec2_make(a.x * b.x, a.y * b.y);
}
svec2 svec2_div(svec2 a, svec2 b)
{
	return svec2_make(a.x / b.x, a.y / b.y);
}
svec2 svec2_mod(svec2 a, svec2 b)
{
	return svec2_make(fmod(a.x, b.x), fmod(a.y, b.y));
}
svec2 svec2_max(svec2 a, svec2 b)
{
	return svec2_make(defmax(a.x, b.x), defmax(a.y, b.y));
}
svec2 svec2_min(svec2 a, svec2 b)
{
	return svec2_make(defmin(a.x, b.x), defmin(a.y, b.y));
}

// signed

s32 svec2_area(svec2 a)
{
	return a.x * a.y;
}
svec2 svec2_flip(svec2 a)
{
	f32 t = a.x;
	a.x = a.y;
	a.y = t;
	return a;
}

svec2 svec2_rsh(svec2 v, u32 sh)
{
	v.x >>= sh;
	v.y >>= sh;
	return v;
}
svec2 svec2_lsh(svec2 v, u32 sh)
{
	v.x <<= sh;
	v.y <<= sh;
	return v;
}
b32 svec2_equal(svec2 a, svec2 b)
{
	return (a.x ==  b.x) && (a.y == b.y);
}


inline fvec2 fvec2_make(f32 a, f32 b)
{
	return (fvec2){a,b};
}
fvec2 fvec2_scalar(f32 a)
{
	return (fvec2){a,a};
}
fvec2 fvec2_zero()
{
	return (fvec2){0.0f, 0.0f};
}
fvec2 fvec2_one()
{
	return (fvec2){1.0f, 1.0f};
}

// vector and scalar operations
fvec2 fvec2_scalar_add(fvec2 a, f32 s)
{
	return fvec2_make(a.x + s, a.y + s);
}
fvec2 fvec2_scalar_sub(fvec2 a, f32 s)
{
	return fvec2_make(a.x - s, a.y - s);
}
fvec2 fvec2_scalar_mul(fvec2 a, f32 s)
{
	return fvec2_make(a.x * s, a.y * s);
}
fvec2 fvec2_scalar_div(fvec2 a, f32 s)
{
	return fvec2_make(a.x / s, a.y / s);
}
fvec2 fvec2_scalar_mod(fvec2 a, f32 s)
{
	return fvec2_make(fmod(a.x, s), fmod(a.y, s));
}
fvec2 fvec2_scalar_max(fvec2 a, f32 s)
{
	return fvec2_make(defmax(a.x, s), defmax(a.y, s));
}
fvec2 fvec2_scalar_min(fvec2 a, f32 s)
{
	return fvec2_make(defmin(a.x, s), defmin(a.y, s));
}

// component wise operations 
fvec2 fvec2_add(fvec2 a, fvec2 b)
{
	return fvec2_make(a.x + b.x, a.y + b.y);
}
fvec2 fvec2_sub(fvec2 a, fvec2 b)
{
	return fvec2_make(a.x - b.x, a.y - b.y);
}
fvec2 fvec2_mul(fvec2 a, fvec2 b)
{
	return fvec2_make(a.x * b.x, a.y * b.y);
}
fvec2 fvec2_div(fvec2 a, fvec2 b)
{
	return fvec2_make(a.x / b.x, a.y / b.y);
}
fvec2 fvec2_mod(fvec2 a, fvec2 b)
{
	return fvec2_make(fmod(a.x, b.x), fmod(a.y, b.y));
}
fvec2 fvec2_max(fvec2 a, fvec2 b)
{
	return fvec2_make(defmax(a.x, b.x), defmax(a.y, b.y));
}
fvec2 fvec2_min(fvec2 a, fvec2 b)
{
	return fvec2_make(defmin(a.x, b.x), defmin(a.y, b.y));
}
fvec2 fvec2_floor(fvec2 a)
{
	a.x = floorf(a.x);
	a.y = floorf(a.y);
	return a;
}

// signed

f32 fvec2_area(fvec2 a)
{
	return a.x * a.y;
}
fvec2 fvec2_flip(fvec2 a)
{
	f32 t = a.x;
	a.x = a.y;
	a.y = t;
	return a;
}
fvec2 fvec2_perp(fvec2 a)
{
	a = fvec2_flip(a);
	a.x = -a.x;
	return a;
}
b32 fvec2_equal(fvec2 a, fvec2 b)
{
	return (a.x ==  b.x) && (a.y == b.y);
}


// floating point

fvec2 fvec2_inv(fvec2 a)
{
	return fvec2_make(1.0f / a.x, 1.0f / a.y);
}
f32 fvec2_magnitude_squared(fvec2 v)
{
	return (v.x * v.x + v.y * v.y);
}

f32 fvec2_magnitude(fvec2 v)
{
	return sqrtf(fvec2_magnitude_squared(v));
}

f32 fvec2_distance_squared(fvec2 a, fvec2 b)
{
	return fvec2_magnitude_squared(fvec2_sub(b,a));
}

f32 fvec2_distance(fvec2 a, fvec2 b)
{
	return fvec2_magnitude(fvec2_sub(b,a));
}
fvec2 fvec2_lerp(fvec2 a, fvec2 b, f32 t)
{
	return fvec2_make(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}
fvec2 fvec2_unit(fvec2 a)
{
	return fvec2_scalar_div(a, fvec2_magnitude(a));
}
fvec2 fvec2_unit2(fvec2 a, fvec2 b)
{
	fvec2 d = fvec2_sub(b,a);
	return fvec2_scalar_div(d, fvec2_magnitude(d));
}
fvec2 fvec2_normal(fvec2 a)
{
	a = fvec2_scalar_div(a, fvec2_magnitude(a));
	a = fvec2_perp(a);
	return a;
}
fvec2 fvec2_normal2(fvec2 a, fvec2 b)
{
	fvec2 d = fvec2_sub(b,a);
	d = fvec2_scalar_div(d, fvec2_magnitude(d));
	f32 t = d.x;
	d.x = -d.y;
	d.y = t;
	return d;
}
fvec2 fvec2_normal3(fvec2 a, fvec2 b, fvec2 c)
{
	a = fvec2_unit2(a,b);
	b = fvec2_unit2(b,c);
	c = fvec2_add(a,b);
	c = fvec2_normal(c);
	return c;
}



f32 fvec2_dot(fvec2 a, fvec2 b)
{
	return a.x * b.x + a.y * b.y;
}
f32 fvec2_cross(fvec2 a, fvec2 b)
{
	return a.x * b.y - a.y * b.x;
}

fvec2 fvec2_negate(fvec2 a)
{
	return fvec2_make(-a.x, -a.y);
}

// vector trig
f32 fvec2_atan(fvec2 v)
{
	return atan2f(v.y, v.x);
}

// VECTOR2 CASTING




fvec2 fvec2_cast_svec2(svec2 v)
{
	return fvec2_make(v.x, v.y);
}
fvec2 fvec2_cast_uvec2(uvec2 v)
{
	return fvec2_make(v.x, v.y);
}

svec2 svec2_cast_fvec2(fvec2 v)
{
	return svec2_make(v.x, v.y);
}
svec2 svec2_cast_uvec2(uvec2 v)
{
	return svec2_make(v.x, v.y);
}
	
uvec2 uvec2_cast_fvec2(fvec2 v)
{
	return uvec2_make(v.x, v.y);
}
uvec2 uvec2_cast_svec2(svec2 v)
{
	return uvec2_make(v.x, v.y);
}



fvec4 fvec4_make(f32 r, f32 g, f32 b, f32 a)
{
	return (fvec4){r,g,b,a};
}
fvec4 fvec4_zero()
{
	return fvec4_make(0,0,0,0);
}
fvec4 fvec4_scalar(f32 a)
{
	return (fvec4){a,a,a,a};
}
fvec4 fvec4_add(fvec4 a, fvec4 b)
{
	return fvec4_make(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
}
fvec4 fvec4_sub(fvec4 a, fvec4 b)
{
	return fvec4_make(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
}
fvec4 fvec4_mul(fvec4 a, fvec4 b)
{
	return fvec4_make(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
}
fvec4 fvec4_div(fvec4 a, fvec4 b)
{
	return fvec4_make(a.r / b.r, a.g / b.g, a.b / b.b, a.a / b.a);
}
u32 print_fvec4(fvec4 v)
{
	return print("fvec4(%f, %f, %f, %f)", v.r,v.g,v.b,v.a);
}
u32 println_fvec4(fvec4 v)
{
	return print("fvec4(%f, %f, %f, %f)\n", v.r,v.g,v.b,v.a);
}

fvec4 fvec4_scalar_add(fvec4 a, f32 b)
{
	return fvec4_make(a.r + b, a.g + b, a.b + b, a.a + b);
}
fvec4 fvec4_scalar_sub(fvec4 a, f32 b)
{
	return fvec4_make(a.r - b, a.g - b, a.b - b, a.a - b);
}
fvec4 fvec4_scalar_mul(fvec4 a, f32 b)
{
	return fvec4_make(a.r * b, a.g * b, a.b * b, a.a * b);
}
fvec4 fvec4_scalar_div(fvec4 a, f32 b)
{
	return fvec4_make(a.r / b, a.g / b, a.b / b, a.a / b);
}

fvec4 fvec4_clamp(fvec4 v, f32 min, f32 max)
{
	return v;
	v.r = fmax(v.r,min);
	v.r = fmin(v.r,min);

	v.g = fmax(v.g,min);
	v.g = fmin(v.g,max);

	v.b = fmax(v.b,min);
	v.b = fmin(v.b,max);

	v.a = fmax(v.a,min);
	v.a = fmin(v.a,max);
	return v;
}

fvec4 fvec4_lerp(fvec4 a, fvec4 b, f32 t)
{
	fvec4 c;
	c.r = lerp(a.r, b.r, t);
	c.g = lerp(a.g, b.g, t);
	c.b = lerp(a.b, b.b, t);
	c.a = lerp(a.a, b.a, t);
	return c;
}

b32 fvec4_is_zero(fvec4 a)
{
	return (
		(a.r == 0.0f) &&
		(a.g == 0.0f) &&
		(a.b == 0.0f) &&
		(a.a == 0.0f) 
	);
}




fmat2 fmat2_zero()
{
	return (fmat2){
		0.0,0.0,
		0.0,0.0	
		};
}
fmat2 fmat2_identity()
{
	return (fmat2){
		1.0,0.0,
		0.0,1.0	
		};
}
fmat2 fmat2_mul(fmat2 a, fmat2 b)
{
	return (fmat2){
		a.a * b.a + a.b * b.c, a.a * b.b + a.b * b.d,	
		a.c * b.a + a.d * b.c, a.c * b.b + a.d * b.d,
	};
}
// 2D Linear transformations
fmat2 fmat2_scale(f32 x, f32 y)
{
	return (fmat2){
		x,0.0,
		0.0,y	
		};
}
fmat2 fmat2_shear(f32 x, f32 y)
{ return (fmat2){
		1.0,x,
		y,1.0	
		};
}
fmat2 fmat2_rotate(f32 radians)
{
	f32 s = sinf(radians);
	f32 c = cosf(radians);
	return (fmat2){
		c, -s,
		s,  c,
	};
}

fvec2 fvec2_mul_fmat2(fvec2 v, fmat2 m)
{
	return (fvec2){
		v.x * m.a + v.y * m.b,
		v.x * m.c + v.y * m.d,
	};

	// reflect

	return (fvec2){
		v.x * m.a + v.y * m.c,
		v.x * m.b + v.y * m.d,
	};
}

fvec2 fvec2_scale_rotate(fvec2 v, f32 scale, f32 radians)
{
	fmat2 mat = fmat2_mul(
		fmat2_rotate(radians),
		fmat2_scale(scale, scale)
	);
	return fvec2_mul_fmat2(v, mat);
}


fmat3 fmat3_zero()
{
	return (fmat3){
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};
}
fmat3 fmat3_identity()
{
	return (fmat3){
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0
	};
}

fmat3 fmat3_add(fmat3 a, fmat3 b)
{
	fmat3 c;
	for(u32 i = 0; i < 9; i++)
	{
		((f32*)&c)[i] = ((f32*)&a)[i] + ((f32*)&b)[i];
	}
	return c;
}

fmat3 fmat3_mul(fmat3 a, fmat3 b)
{
	return (fmat3){
		a.a*b.a+a.b*b.d+a.c*b.g, a.a*b.b+a.b*b.e+a.c*b.h, a.a*b.c+a.b*b.f+a.c*b.i, 
		a.d*b.a+a.e*b.d+a.f*b.g, a.d*b.b+a.e*b.e+a.f*b.h, a.d*b.c+a.e*b.f+a.f*b.i, 
		a.g*b.a+a.h*b.d+a.i*b.g, a.g*b.b+a.h*b.e+a.i*b.h, a.g*b.c+a.h*b.f+a.i*b.i, 
	};
}
// 2D Affine

fmat3 fmat3_affine_translate_add(fmat3 a, f32 x, f32 y)
{
	a.c += x;
	a.f += y;
	return a;
}
fmat3 fmat3_affine_translate(f32 x, f32 y)
{
	return (fmat3){
		1.0, 0.0, x  ,
		0.0, 1.0, y  ,
		0.0, 0.0, 1.0,
	};
}
fmat3 fmat3_affine_scale(f32 x, f32 y)
{
	return (fmat3){
		x  , 0.0, 0.0,
		0.0, y  , 0.0,
		0.0, 0.0, 1.0,
	};
}
fmat3 fmat3_affine_shear(f32 x, f32 y)
{
	return (fmat3){
		1.0, x  , 0.0,
		y  , 1.0, 0.0,
		0.0, 0.0, 1.0,
	};
}
fmat3 fmat3_affine_rotate(f32 radians)
{
	f32 c = cosf(radians);
	f32 s = sinf(radians);
	return (fmat3){
		c  ,-s  , 0.0,
		s  , c  , 0.0,
		0.0, 0.0, 1.0
	};
}

fmat3 fmat3_lerp(fmat3 a, fmat3 b, f32 t)
{
	fmat3 c;
	for(u32 i = 0; i < 9; i++)
	{
		((f32*)&c)[i] = lerp(((f32*)&a)[i], ((f32*)&b)[i], t);
	}
	return c;
}
fvec2 fvec2_mul_fmat3(fvec2 v, fmat3 m)
{
	return (fvec2){
		m.a * v.x + m.b * v.y + 1.0 * m.c,
		m.d * v.x + m.e * v.y + 1.0 * m.f,	
	};
}

fmat3 fmat3_mul_va(u32 c, ...)
{
	va_list l;
	va_start(l, c);
	fmat3 mat = fmat3_identity();
	for(u32 i = 0; i < c; i++)
	{
		fmat3 mat_arg = va_arg(l, fmat3);
		mat = fmat3_mul(mat,mat_arg); 
	}
	va_end(l);
	return mat;
}
fmat3p fmat3_padding(fmat3 a)
{
	fmat3p b = {
		a.a, a.b, a.c, 0.0,
		a.d, a.e, a.f, 0.0,
		a.g, a.h, a.i, 0.0,
	};
	return b;
}

fvec2 fvec2_affine_offset(fmat3 m)
{
	return fvec2_make(m.c, m.f);	
}
f32 fmat3_affine_det(fmat3 m)
{
	return m.a * m.e - m.b * m.d;
}

fmat3 fmat3_affine_inverse(fmat3 in)
{
	f32 det = fmat3_affine_det(in);
	f32 inv_det = 1.0 / det;

	f32 inv_a =  inv_det * in.e;
	f32 inv_b = -inv_det * in.b;
	f32 inv_d = -inv_det * in.d;
	f32 inv_e =  inv_det * in.a;

	if(det == 0.0f)
	{
		print_fmat3(in);
		RUNTIME_ABORT("Matrix is not invertable\n");
	}
	
	f32 inv_c = -(inv_a * in.c + inv_b  * in.f);
	f32 inv_f = -(inv_d * in.c + inv_e  * in.f);

	fmat3 out;
	out.a = inv_a;	out.b = inv_b;	out.c = inv_c;
	out.d = inv_d;	out.e = inv_e;	out.f = inv_f;
	out.g = 0.0f;	out.h = 0.0f;	out.i = 1.0f;
	return out;
}

fmat3 print_fmat3(fmat3 m)
{
	print("mat3:\n");
	print("[ %f32 %f32 %f32 ]\n", m.a,m.b,m.c);
	print("[ %f32 %f32 %f32 ]\n", m.d,m.e,m.f);
	print("[ %f32 %f32 %f32 ]\n", m.g,m.h,m.i);
	return m;
}


