#pragma once

#include "def.h"

#include <stdint.h>

// UNSIGNED INT
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u8 u128[16];
typedef u8 u256[32];
typedef u8 u512[64];

// SIGNED INT
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef u8 s128[16];
typedef u8 s256[32];
typedef u8 s512[64];

// FLOAT 
typedef u8 f8;
typedef _Float16 f16;
typedef float f32;
typedef double f64;
typedef	__float80 f80;
typedef __float128 f128;
typedef u8 f256[32];
typedef u8 f512[64];

// VOID
typedef u8 v8[1];
typedef u16 v16[2];
typedef u32 v32[4];
typedef u64 v64[8];
typedef u128 v128[16];
typedef u256 v256[32];
typedef u512 v512[64];

// BOOL
typedef s8 b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

// UNSIGNED VECTOR
typedef u8  u8v align(1);
typedef u16 u16v align(2);
typedef u32 u32v align(4);
typedef u64 u64v align(8);
typedef u128 u128v align(16);
typedef u256 u256v align(32);
typedef u512 u512v align(64);

typedef u8v u8x1;
typedef u16v u8x2;
typedef u32v u8x4;
typedef u64v u8x8;
typedef u128v u8x16;
typedef u256v u8x32;
typedef u512v u8x64;

typedef u16v u16x1;
typedef u32v u16x2;
typedef u64v u16x4;
typedef u128v u16x8;
typedef u256v u16x16;
typedef u512v u16x32;

typedef u32v u32x1;
typedef u64v u32x2;
typedef u128v u32x4;
typedef u256v u32x8;
typedef u512v u32x16;

typedef u64v u64x1;
typedef u128v u64x2;
typedef u256v u64x4;
typedef u512v u64x8;

// SIGNED VECTOR
typedef s8  s8v align(1);
typedef s16 s16v align(2);
typedef s32 s32v align(4);
typedef s64 s64v align(8);
typedef s128 s128v align(16);
typedef s256 s256v align(32);
typedef s512 s512v align(64);

typedef s8v s8x1;
typedef s16v s8x2;
typedef s32v s8x4;
typedef s64v s8x8;
typedef s128v s8x16;
typedef s256v s8x32;
typedef s512v s8x64;

typedef s16v s16x1;
typedef s32v s16x2;
typedef s64v s16x4;
typedef s128v s16x8;
typedef s256v s16x16;
typedef s512v s16x32;

typedef s32v s32x1;
typedef s64v s32x2;
typedef s128v s32x4;
typedef s256v s32x8;
typedef s512v s32x16;

typedef s64v s64x1;
typedef s128v s64x2;
typedef s256v s64x4;
typedef s512v s64x8;

// FLOAT VECTOR
typedef f8 f8v align(1);
typedef f16 f16v align(2);
typedef f32 f32v align(4);
typedef f64 f64v align(8);
typedef f128 f128v align(16);
typedef f256 f256v align(32);
typedef f512 f512v align(64);

typedef f8v f8x1;
typedef f16v f8x2;
typedef f32v f8x4;
typedef f64v f8x8;
typedef f128v f8x16;
typedef f256v f8x32;
typedef f512v f8x64;

typedef f16v f16x1;
typedef f32v f16x2;
typedef f64v f16x4;
typedef f128v f16x8;
typedef f256v f16x16;
typedef f512v f16x32;

typedef f32v f32x1;
typedef f64v f32x2;
typedef f128v f32x4;
typedef f256v f32x8;
typedef f512v f32x16;

typedef f64v f64x1;
typedef f128v f64x2;
typedef f256v f64x4;
typedef f512v f64x8;

// TYPE ENUMERATION

// Setting enum values explicitly can change how switch statements can be optimized. Cool.
typedef enum{
	TYPE_VOID,
	TYPE_V8,
	TYPE_V16,
	TYPE_V32,
	TYPE_V64,
	TYPE_V128,
	TYPE_V256,
	TYPE_V512,

	TYPE_B8,
	TYPE_B16,
	TYPE_B32,
	TYPE_B64,

	TYPE_U8,
	TYPE_U16,
	TYPE_U32,
	TYPE_U64,
	TYPE_U128,
	TYPE_U256,
	TYPE_U512,

	TYPE_S8,
	TYPE_S16,
	TYPE_S32,
	TYPE_S64,
	TYPE_S128,
	TYPE_S256,
	TYPE_S512,

	TYPE_F8,
	TYPE_F16,
	TYPE_F32,
	TYPE_F64,
	TYPE_F128,
	TYPE_F256,
	TYPE_F512,

	TYPE_F32VEC2,
	TYPE_U32VEC2,
	TYPE_S32VEC2,

	TYPE_F32VEC3,
	TYPE_F32VEC4,
	TYPE_F32MAT2,
	TYPE_F32MAT3,
	TYPE_F32MAT4,
	

	// These are all explicit types because they occupy different registers, they cannot be accessed like normal.
	// Though do I really need to specify x8, x16, ...? 
	// I will leave them for now.


	TYPE_U8_VECTOR,
	TYPE_U16_VECTOR,
	TYPE_U32_VECTOR,
	TYPE_U64_VECTOR,
	TYPE_U128_VECTOR,
	TYPE_U256_VECTOR,
	TYPE_U512_VECTOR,

	TYPE_U8X1_VECTOR,
	TYPE_U8X2_VECTOR,
	TYPE_U8X4_VECTOR,
	TYPE_U8X8_VECTOR,
	TYPE_U8X16_VECTOR,
	TYPE_U8X32_VECTOR,
	TYPE_U8X64_VECTOR,

	TYPE_U16X1_VECTOR,
	TYPE_U16X2_VECTOR,
	TYPE_U16X4_VECTOR,
	TYPE_U16X8_VECTOR,
	TYPE_U16X16_VECTOR,
	TYPE_U16X32_VECTOR,

	TYPE_U32X1_VECTOR,
	TYPE_U32X2_VECTOR,
	TYPE_U32X4_VECTOR,
	TYPE_U32X8_VECTOR,
	TYPE_U32X16_VECTOR,

	TYPE_U64X1_VECTOR,
	TYPE_U64X2_VECTOR,
	TYPE_U64X4_VECTOR,
	TYPE_U64X8_VECTOR,

	TYPE_S8_VECTOR,
	TYPE_S16_VECTOR,
	TYPE_S32_VECTOR,
	TYPE_S64_VECTOR,
	TYPE_S128_VECTOR,
	TYPE_S256_VECTOR,
	TYPE_S512_VECTOR,

	TYPE_S8X1_VECTOR,
	TYPE_S8X2_VECTOR,
	TYPE_S8X4_VECTOR,
	TYPE_S8X8_VECTOR,
	TYPE_S8X16_VECTOR,
	TYPE_S8X32_VECTOR,
	TYPE_S8X64_VECTOR,

	TYPE_S16X1_VECTOR,
	TYPE_S16X2_VECTOR,
	TYPE_S16X4_VECTOR,
	TYPE_S16X8_VECTOR,
	TYPE_S16X16_VECTOR,
	TYPE_S16X32_VECTOR,

	TYPE_S32X1_VECTOR,
	TYPE_S32X2_VECTOR,
	TYPE_S32X4_VECTOR,
	TYPE_S32X8_VECTOR,
	TYPE_S32X16_VECTOR,

	TYPE_S64X1_VECTOR,
	TYPE_S64X2_VECTOR,
	TYPE_S64X4_VECTOR,
	TYPE_S64X8_VECTOR,

	TYPE_F8_VECTOR,
	TYPE_F16_VECTOR,
	TYPE_F32_VECTOR,
	TYPE_F64_VECTOR,
	TYPE_F128_VECTOR,
	TYPE_F256_VECTOR,
	TYPE_F512_VECTOR,

	TYPE_F8X1_VECTOR,
	TYPE_F8X2_VECTOR,
	TYPE_F8X4_VECTOR,
	TYPE_F8X8_VECTOR,
	TYPE_F8X16_VECTOR,
	TYPE_F8X32_VECTOR,
	TYPE_F8X64_VECTOR,

	TYPE_F16X1_VECTOR,
	TYPE_F16X2_VECTOR,
	TYPE_F16X4_VECTOR,
	TYPE_F16X8_VECTOR,
	TYPE_F16X16_VECTOR,
	TYPE_F16X32_VECTOR,

	TYPE_F32X1_VECTOR,
	TYPE_F32X2_VECTOR,
	TYPE_F32X4_VECTOR,
	TYPE_F32X8_VECTOR,
	TYPE_F32X16_VECTOR,

	TYPE_F64X1_VECTOR,
	TYPE_F64X2_VECTOR,
	TYPE_F64X4_VECTOR,
	TYPE_F64X8_VECTOR,

	TYPE_PAGE,
	TYPE_CACHE_LINE,

	TYPE_BUFFER,
	TYPE_ARRAY,
	TYPE_RING,
	TYPE_RING_BUFFER,

	TYPE_THREAD,
	TYPE_ARENA,

	TYPE_ASCII,
	TYPE_UTF8,
	TYPE_UTF16,
	TYPE_UTF32,
	TYPE_UTF64,

	TYPE_ASCII_CSTRING,
	TYPE_UTF8_CSTRING,
	TYPE_UTF16_CSTRING,
	TYPE_UTF32_CSTRING,

	TYPE_ASCII_STRING,
	TYPE_UTF8_STRING,
	TYPE_UTF16_STRING,
	TYPE_UTF32_STRING,

	TYPE_CSTRING = TYPE_ASCII_CSTRING,
	TYPE_STRING = TYPE_ASCII_STRING,

	TYPE_TIME,
	TYPE_PICOSECOND,
	TYPE_NANOSECOND,
	TYPE_MICORSECOND,
	TYPE_MILLISECOND,
	TYPE_SECOND,
	


}Type;








