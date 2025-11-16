#pragma once


typedef u32 RiaccAccumulator[256 * 256];

typedef struct{
	u8 a,b;	
}RiaccLine;

/*
	Riacc Buckets
	In positive X direction:

0	Left-Right
1	Left-Top
2	Left-Bottom
3	Left-Terminate
4	Top-Right
5	Top-Terminate
6	Bottom-Right
7	Bottom-Terminate
*/

typedef struct RiaccBucket{
	u32 count;
	u32 capacity;
	struct RiaccBucket *previous;
	RiaccLine lines[];
}RiaccBucket;

typedef struct{
	 RiaccBucket buckets[8];				
	 u32 bucket_capacity;
}RiaccSquare;

typedef struct{
	u32 squares_width, squares_height;
	RiaccSquare *squares;
	u32 new_bucket_capacity;
}Riacc;

