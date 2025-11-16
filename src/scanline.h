#pragma once

// Successor to draw.h
// Scanline Rasterizer

#include "image.h"


typedef enum{
	FRAGMENT_NONE = 0,
	FRAGMENT_FLAT,
	FRAGMENT_LERP,
	FRAGMENT_TEXTURE,
}FragmentTypeFlags;
typedef u8 FragmentType;

typedef enum{
	FRAGMENT_EFFECT_NONE,
	FRAGMENT_EFFECT_NOISE,
}FragmentEffectFlags;
typedef u8 FragmentEffect;

typedef struct{
	Color32 colors[256];		
	PRNG rg;
}ScanlineRegister;

typedef struct{
	FragmentType type;	
	union{
		struct{
			u8 ca, cb;
		};
		struct{
			u8 ta, tb;
		};
	};
	FragmentEffect effect;
	u16 count;
	u16 depth;
}Fragment align(8);

typedef struct{
	// Bucket list
}Scanline;

// Look into "American Flag Sort"

typedef struct{
	u32 width, height;	

	Arena *arena;
}Rasterizer;


