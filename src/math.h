#pragma once
#include "types.h"
#include <math.h>

f32 lerp(f32 a, f32 b, f32 t);


/////////////////////////////////////////////////////////////////////////////////////////
// VECTOR
/////////////////////////////////////////////////////////////////////////////////////////


typedef struct{
	u32 x,y;
}uvec2;

uvec2 uvec2_make(u32 a, u32 b);
uvec2 uvec2_scalar(u32 a);
uvec2 uvec2_zero();
uvec2 uvec2_one();

// vector and scalar operations
uvec2 uvec2_scalar_add(uvec2 a, u32 s);
uvec2 uvec2_scalar_sub(uvec2 a, u32 s);
uvec2 uvec2_scalar_mul(uvec2 a, u32 s);
uvec2 uvec2_scalar_div(uvec2 a, u32 s);
uvec2 uvec2_scalar_mod(uvec2 a, u32 s);
uvec2 uvec2_scalar_max(uvec2 a, u32 s);
uvec2 uvec2_scalar_min(uvec2 a, u32 s);

// component wise operations 
uvec2 uvec2_add(uvec2 a, uvec2 b);
uvec2 uvec2_sub(uvec2 a, uvec2 b);
uvec2 uvec2_mul(uvec2 a, uvec2 b);
uvec2 uvec2_div(uvec2 a, uvec2 b);
uvec2 uvec2_mod(uvec2 a, uvec2 b);
uvec2 uvec2_max(uvec2 a, uvec2 b);
uvec2 uvec2_min(uvec2 a, uvec2 b);

// signed

u32 uvec2_area(uvec2 a);
b32 uvec2_equal(uvec2 a, uvec2 b);
uvec2 uvec2_flip(uvec2 a);

uvec2 uvec2_rsh(uvec2 v, u32 sh);
uvec2 uvec2_lsh(uvec2 v, u32 sh);

typedef struct{
	s32 x,y;
}svec2;

svec2 svec2_make(s32 a, s32 b);
svec2 svec2_scalar(s32 a);
svec2 svec2_zero();
svec2 svec2_one();

// vector and scalar operations
svec2 svec2_scalar_add(svec2 a, s32 s);
svec2 svec2_scalar_sub(svec2 a, s32 s);
svec2 svec2_scalar_mul(svec2 a, s32 s);
svec2 svec2_scalar_div(svec2 a, s32 s);
svec2 svec2_scalar_mod(svec2 a, s32 s);
svec2 svec2_scalar_max(svec2 a, s32 s);
svec2 svec2_scalar_min(svec2 a, s32 s);

// component wise operations 
svec2 svec2_add(svec2 a, svec2 b);
svec2 svec2_sub(svec2 a, svec2 b);
svec2 svec2_mul(svec2 a, svec2 b);
svec2 svec2_div(svec2 a, svec2 b);
svec2 svec2_mod(svec2 a, svec2 b);
svec2 svec2_max(svec2 a, svec2 b);
svec2 svec2_min(svec2 a, svec2 b);

// signed

s32 svec2_area(svec2 a);
b32 svec2_equal(svec2 a, svec2 b);
svec2 svec2_flip(svec2 a);
svec2 svec2_rsh(svec2 v, u32 sh);
svec2 svec2_lsh(svec2 v, u32 sh);

typedef struct{
	f32 x,y;
}fvec2;

fvec2 fvec2_make(f32 a, f32 b);
fvec2 fvec2_scalar(f32 a);
fvec2 fvec2_zero();
fvec2 fvec2_one();

// vector and scalar operations
fvec2 fvec2_scalar_add(fvec2 a, f32 s);
fvec2 fvec2_scalar_sub(fvec2 a, f32 s);
fvec2 fvec2_scalar_mul(fvec2 a, f32 s);
fvec2 fvec2_scalar_div(fvec2 a, f32 s);
fvec2 fvec2_scalar_mod(fvec2 a, f32 s);
fvec2 fvec2_scalar_max(fvec2 a, f32 s);
fvec2 fvec2_scalar_min(fvec2 a, f32 s);

// component wise operations 
fvec2 fvec2_add(fvec2 a, fvec2 b);
fvec2 fvec2_sub(fvec2 a, fvec2 b);
fvec2 fvec2_mul(fvec2 a, fvec2 b);
fvec2 fvec2_div(fvec2 a, fvec2 b);
fvec2 fvec2_mod(fvec2 a, fvec2 b);
fvec2 fvec2_max(fvec2 a, fvec2 b);
fvec2 fvec2_min(fvec2 a, fvec2 b);
fvec2 fvec2_floor(fvec2 a);

// signed

f32 fvec2_area(fvec2 a);
fvec2 fvec2_flip(fvec2 a);
fvec2 fvec2_perp(fvec2 a);
b32 fvec2_equal(fvec2 a, fvec2 b);


// floating point

fvec2 fvec2_inv(fvec2 a);
fvec2 fvec2_lerp(fvec2 a, fvec2 b, f32 t);
fvec2 fvec2_unit(fvec2 a);
fvec2 fvec2_unit2(fvec2 a, fvec2 b);
fvec2 fvec2_normal(fvec2 a);
fvec2 fvec2_normal2(fvec2 a, fvec2 b);
fvec2 fvec2_normal3(fvec2 a, fvec2 b, fvec2 c); // average of (b-a) and (c-b)

f32 fvec2_magnitude_squared(fvec2 v);
f32 fvec2_magnitude(fvec2 v);
f32 fvec2_distance_squared(fvec2 a, fvec2 b);
f32 fvec2_distance(fvec2 a, fvec2 b);
// signed
f32 fvec2_dot(fvec2 a, fvec2 b);
f32 fvec2_cross(fvec2 a, fvec2 b);
fvec2 fvec2_negate(fvec2 a);


// vector trig
f32 fvec2_atan(fvec2 v);

/////////////////////////////////////////////////////////////////////////////////////////
// VECTOR 2 CASTING 
/////////////////////////////////////////////////////////////////////////////////////////

fvec2 fvec2_cast_svec2(svec2 v);
fvec2 fvec2_cast_uvec2(uvec2 v);

svec2 svec2_cast_fvec2(fvec2 v);
svec2 svec2_cast_uvec2(uvec2 v);
	
uvec2 uvec2_cast_fvec2(fvec2 v);
uvec2 uvec2_cast_svec2(svec2 v);

/////////////////////////////////////////////////////////////////////////////////////////
// VECTOR 3 
/////////////////////////////////////////////////////////////////////////////////////////

typedef struct{
	f32 r,g,b;
}fvec3;

/////////////////////////////////////////////////////////////////////////////////////////
// VECTOR 4 
/////////////////////////////////////////////////////////////////////////////////////////


typedef struct{
	f32 r,g,b,a;
}fvec4;

fvec4 fvec4_make(f32 r, f32 g, f32 b, f32 a);
fvec4 fvec4_zero();
fvec4 fvec4_scalar(f32 a);
fvec4 fvec4_add(fvec4 a, fvec4 b);
fvec4 fvec4_sub(fvec4 a, fvec4 b);
fvec4 fvec4_mul(fvec4 a, fvec4 b);
fvec4 fvec4_div(fvec4 a, fvec4 b);
u32 print_fvec4(fvec4 v);
u32 println_fvec4(fvec4 v);
fvec4 fvec4_scalar_add(fvec4 a, f32 b);
fvec4 fvec4_scalar_sub(fvec4 a, f32 b);
fvec4 fvec4_scalar_mul(fvec4 a, f32 b);
fvec4 fvec4_scalar_div(fvec4 a, f32 b);

fvec4 fvec4_clamp(fvec4 v, f32 min, f32 max);
fvec4 fvec4_lerp(fvec4 a, fvec4 b, f32 t);
b32 fvec4_is_zero(fvec4 a);


/////////////////////////////////////////////////////////////////////////////////////////
// MATRIX
/////////////////////////////////////////////////////////////////////////////////////////


typedef struct{
	f32 a,b;
	f32 c,d;
}fmat2;

fmat2 fmat2_zero();
fmat2 fmat2_identity();
fmat2 fmat2_mul(fmat2 a, fmat2 b);

// 2D Linear transformations
fmat2 fmat2_scale(f32 x, f32 y);
fmat2 fmat2_shear(f32 x, f32 y);
fmat2 fmat2_rotate(f32 radians);
fvec2 fvec2_mul_fmat2(fvec2 v, fmat2 m);
fvec2 fvec2_scale_rotate(fvec2 v, f32 scale, f32 radians);


typedef struct{
	f32 a,b,c;
	f32 d,e,f;
	f32 g,h,i;
}fmat3;

typedef struct{ 
	f32 a,b,c, padding0;
	f32 d,e,f, padding1;
	f32 g,h,i, padding2;
}fmat3p;

fmat3 fmat3_zero();
fmat3 fmat3_identity();
fmat3 fmat3_add(fmat3 a, fmat3 b);
fmat3 fmat3_affine_translate_add(fmat3 a, f32 x, f32 y);
fmat3 fmat3_mul(fmat3 a, fmat3 b);
fmat3 fmat3_affine_translate(f32 x, f32 y);
fmat3 fmat3_affine_scale(f32 x, f32 y);
fmat3 fmat3_affine_shear(f32 x, f32 y);
fmat3 fmat3_affine_rotate(f32 radians);
fmat3 fmat3_lerp(fmat3 a, fmat3 b, f32 t);
fvec2 fvec2_mul_fmat3(fvec2 v, fmat3 m);
fmat3 fmat3_mul_va(u32 c, ...);
fmat3p fmat3_padding(fmat3 a);


fvec2 fvec2_affine_offset(fmat3 m);

f32 fmat3_affine_det(fmat3 m);
fmat3 fmat3_affine_inverse(fmat3 in);

fmat3 print_fmat3(fmat3 m);


typedef struct{
	f32 a,b,c,d;
	f32 e,f,g,h;
	f32 i,j,k,l;
	f32 m,n,o,p;
}fmat4;
