#pragma once
#include "basic.h"


// This file defines basic type independent data structures.

// Some basic data structures. 
/*
Packed:
	ALIGNMENT | META | DATA 
* Metadata and data is packed after alignment (cache line)
Aligned:
	META | ALIGNMENT | DATA
* Metadata is allocated before alignment (cache line) and data is right after. 
*/


typedef enum{
	DS_DATA = 0, // TODO
	DS_BUFFER,	
	DS_ARRAY,
	DS_RING,
	DS_RING_BUFFER,
#if 0
	DS_SINGLE_LINK,
	DS_DOUBLE_LINK,
	DS_GRAPH_LINK,
#endif 

// TODO
#if 0
	DS_ASCII_STRING, 
	DS_UTF8_STRING,
	DS_UTF16_STRING,
	DS_UTF32_STRING,
	DS_STRING = DS_ASCII_STRING,
#endif
}AlgType;


typedef struct{
	u64 count;
}Buffer;

typedef struct{
	u64 cap;
	u64 count; // Keep this here: Array reinterpret to a Buffer.
}Array;

typedef struct{
	u32 index, cap;
}Ring;

typedef struct{
	u64 a, b, count, mask, cap;
}RingBuffer;

#define buffer_count(TYPE_PTR) ((u64*)((Buffer*)(TYPE_PTR)-1))[0]
#define buffer_size(TYPE_PTR) (sizeof(*(TYPE_PTR)) * buffer_count(TYPE_PTR))


#define allocate_buffer_after_alignment(TYPE, COUNT, ARENA)\
({\
	Buffer *buffer = arena_alloc(sizeof(TYPE) * (COUNT) + sizeof(Buffer), ARENA);\
	buffer->count = (COUNT);\
	(TYPE*)(buffer+1);\
})

#define allocate_buffer(TYPE, COUNT, ARENA)\
({\
	Buffer *buffer = arena_alloc_before_alignment(sizeof(TYPE) * (COUNT) + sizeof(Buffer), sizeof(Buffer), ARENA);\
	buffer->count = (COUNT);\
	(TYPE*)(buffer+1);\
})

#define copy_buffer(DST_TYPE_PTR, SRC_TYPE_PTR)\
({\
	memcpy(DST_TYPE_PTR, SRC_TYPE_PTR, buffer_size(SRC_TYPE_PTR));\
	DST_TYPE_PTR;\
})

#define allocate_copy_buffer(SRC_TYPE_PTR, ARENA)\
({\
	Buffer *buffer = arena_alloc_before_alignment(buffer_size(SRC_TYPE_PTR) + sizeof(Buffer), sizeof(Buffer), ARENA);\
	buffer->count = (COUNT);\
	memcpy(buffer+1, SRC_TYPE_PTR, buffer_size(SRC_TYPE_PTR));\
	(void*)(buffer+1);\
})


#define zero_buffer(TYPE_PTR)\
({\
	memset((void*)(TYPE_PTR), 0, sizeof(*(TYPE_PTR)) * buffer_count(TYPE_PTR));\
	(TYPE_PTR);\
})\

#define itterate_buffer(TYPE_PTR, CALL)\
({\
	for(u64 i = 0; i < buffer_count(TYPE_PTR); i++)\
	{\
		CALL;\
	}\
	(TYPE_PTR);\
})

#define set_buffer(TYPE_PTR, VAL)\
({\
	(TYPE_PTR)[0] = VAL;\
	for(u64 i = 0; i < buffer_count(TYPE_PTR); i++)\
	{\
		memcpy(((void*)(TYPE_PTR) + sizeof(*(TYPE_PTR)) * i), (TYPE_PTR), sizeof(*TYPE_PTR));\
	}\
	(TYPE_PTR);\
})


#define array_to_buffer(TYPE_PTR) (TYPE_PTR)
#define buffer_to_array(TYPE_PTR) (TYPE_PTR)

#define array_count(TYPE_PTR) ((((Array*)(TYPE_PTR))-1)->count)
#define array_capacity(TYPE_PTR) ((((Array*)(TYPE_PTR))-1)->cap)

#define array_count_size(TYPE_PTR) (sizeof(*(TYPE_PTR)) * array_count(TYPE_PTR))
#define array_capacity_size(TYPE_PTR) (sizeof(*(TYPE_PTR)) * array_capacity(TYPE_PTR))

#define array_last(TYPE_PTR) ((TYPE_PTR) + array_count(TYPE_PTR) - 1)
#define array_set_full(TYPE_PTR) (array_count(TYPE_PTR) = array_capacity(TYPE_PTR))

#define allocate_array_after_alignment(TYPE, CAP, ARENA)\
({\
	Array *array = arena_alloc(sizeof(TYPE) * (CAP) + sizeof(Array), ARENA);\
	array->count = 0;\
	array->cap = (COUNT);\
	(TYPE*)(array+1);\
})

#define allocate_array(TYPE, CAP, ARENA)\
({\
	Array *array = arena_alloc_before_alignment(sizeof(TYPE) * (CAP) + sizeof(Array), sizeof(Array), ARENA);\
	array->count = 0;\
	array->cap = (CAP);\
	(TYPE*)(array+1);\
})

#define zero_array(TYPE_PTR)\
({\
	memset((void*)(TYPE_PTR), 0, sizeof(*(TYPE_PTR)) * array_count(TYPE_PTR));\
	(TYPE_PTR);\
})\

#define itterate_array(TYPE_PTR, CALL)\
({\
	for(u64 i = 0; i < array_count(TYPE_PTR); i++)\
	{\
		CALL;\
	}\
	(TYPE_PTR);\
})

#define set_array(TYPE_PTR, VAL)\
({\
	(TYPE_PTR)[0] = VAL;\
	for(u64 i = 0; i < array_count(TYPE_PTR); i++)\
	{\
		memcpy(((void*)(TYPE_PTR) + sizeof(*(TYPE_PTR)) * i), (TYPE_PTR), sizeof(*TYPE_PTR));\
	}\
	(TYPE_PTR);\
})

#define append_array_factor(TYPE_PTR, VAL, NEW_CAP, ARENA)\
({\
	ASSERT_LVALUE(TYPE_PTR);\
	if(array_count(TYPE_PTR) >= array_capacity(TYPE_PTR))\
	{\
		if(((TYPE_PTR) - sizeof(Array)) == (ARENA)->last)\
		{\
			arena->pos = arena->last;\
			void* data = arena_alloc_before_alignment(sizeof(*TYPE_PTR) * (NEW_CAP) + sizeof(Array), sizeof(Array), ARENA);\
			TYPE_PTR = data + sizeof(Array);\
		}\
		else\
		{\
			void* data = arena_alloc_before_alignment(sizeof(*TYPE_PTR) * (NEW_CAP) + sizeof(Array), sizeof(Array), ARENA);\
			memcpy(data, (void*)TYPE_PTR - sizeof(Array), sizeof(Array) + sizeof(*TYPE_PTR) * array_count(TYPE_PTR));\
			TYPE_PTR = data + sizeof(Array);\
		}\
		array_capacity(TYPE_PTR) = NEW_CAP;\
	}\
	array_count(TYPE_PTR)++;\
	array_last(TYPE_PTR)[0] = VAL;\
	TYPE_PTR;\
})

#define CPYTHON_ARRAY_FACTOR(TYPE_PTR) (((array_capacity(TYPE_PTR)+1) + ((array_capacity(TYPE_PTR)+1) >> 3) + 6) & ~(u64)3)
#define SIMPLE_ARRAY_FACTOR(TYPE_PTR) ((array_capacity(TYPE_PTR)+1) * 2)

#define append_array(TYPE_PTR, VAL, ARENA)\
({\
	append_array_factor(TYPE_PTR, VAL, SIMPLE_ARRAY_FACTOR(TYPE_PTR), ARENA);\
})




#define ring_index(TYPE_PTR) (((Ring*)(TYPE_PTR)-1)->index)
#define ring_capacity(TYPE_PTR) (((Ring*)(TYPE_PTR)-1)->cap)

#define allocate_ring(TYPE, CAP, ARENA)\
({\
	Ring *ring= arena_alloc_before_alignment(sizeof(TYPE) * (CAP) + sizeof(Ring), sizeof(Ring), ARENA);\
	ring->cap = (CAP);\
	ring->index = 0;\
	(TYPE*)(ring+1);\
})

#define ring_current(TYPE_PTR) ((TYPE_PTR) + ring_index(TYPE_PTR))

#define ring_next(TYPE_PTR) \
({\
	ring_index(TYPE_PTR) = (ring_index(TYPE_PTR) + 1) % ring_capacity(TYPE_PTR);\
	ring_current(TYPE_PTR);\
})
#define ring_prev(TYPE_PTR) \
({\
	ring_index(TYPE_PTR) = (ring_index(TYPE_PTR) - 1) % ring_capacity(TYPE_PTR);\
	ring_current(TYPE_PTR);\
})


#define set_ring(TYPE_PTR, VAL)\
({\
	(TYPE_PTR)[0] = VAL;\
	for(u64 i = 0; i < ring_capacity(TYPE_PTR); i++)\
	{\
		memcpy(((void*)(TYPE_PTR) + sizeof(*(TYPE_PTR)) * i), (TYPE_PTR), sizeof(*TYPE_PTR));\
	}\
	(TYPE_PTR);\
})

#define itterate_ring(TYPE_PTR, CALL)\
({\
	for(u64 i = 0; i < ring_capacity(TYPE_PTR); i++)\
	{\
		CALL;\
	}\
	(TYPE_PTR);\
})



#define ring_buffer_capacity(TYPE_PTR) 	(((RingBuffer*)(TYPE_PTR)-1)->cap)
#define ring_buffer_mask(TYPE_PTR) 		(((RingBuffer*)(TYPE_PTR)-1)->mask)
#define ring_buffer_count(TYPE_PTR) 	(((RingBuffer*)(TYPE_PTR)-1)->count)
#define ring_buffer_head(TYPE_PTR)	 	(((RingBuffer*)(TYPE_PTR)-1)->a)
#define ring_buffer_tail(TYPE_PTR) 		(((RingBuffer*)(TYPE_PTR)-1)->b)

#define allocate_ring_buffer(TYPE, CAP, ARENA)\
({\
	u64 cap = uint_next_power_of_two(CAP);\
	RingBuffer *ring_buffer = arena_alloc_before_alignment(sizeof(TYPE) * (cap) + sizeof(RingBuffer), sizeof(RingBuffer), ARENA);\
	ring_buffer->mask = cap - 1;\
	ring_buffer->cap = cap;\
	ring_buffer->count = 0;\
	ring_buffer->a = 0;\
	ring_buffer->b = 0;\
	(TYPE*)(ring_buffer+1);\
})

#define ring_buffer_push(TYPE_PTR, VAL)\
({\
	if(ring_buffer_count(TYPE_PTR) == ring_buffer_capacity(TYPE_PTR))\
	{\
		ring_buffer_count(TYPE_PTR) = 0;\
	}\
	TYPE_PTR[ring_buffer_head(TYPE_PTR)] = (VAL);\
	ring_buffer_head(TYPE_PTR)++;\
	ring_buffer_head(TYPE_PTR) &= ring_buffer_mask(TYPE_PTR);\
	ring_buffer_count(TYPE_PTR)++;\
	TYPE_PTR;\
})

#define ring_buffer_pop(TYPE_PTR, VAL_PTR)\
({\
	b32 ret = false;\
	if(ring_buffer_count(TYPE_PTR) != 0)\
	{\
		if(VAL_PTR)\
		{\
			memcpy(VAL_PTR, &(TYPE_PTR)[ring_buffer_tail(TYPE_PTR)], sizeof(*TYPE_PTR));\
		}\
		ring_buffer_tail(TYPE_PTR)++;\
		ring_buffer_tail(TYPE_PTR) &= ring_buffer_mask(TYPE_PTR);\
		ring_buffer_count(TYPE_PTR)--;\
		ret = true;\
	}\
	ret;\
})

#define export_ring_buffer_to_buffer(TYPE_PTR, ARENA)\
({\
	Buffer *buffer = arena_alloc(sizeof(*TYPE_PTR) * (ring_buffer_count(TYPE_PTR) + sizeof(Buffer), ARENA);\
	buffer->count = (COUNT);\
	RingBuffer temp = (RingBuffer*)(TYPE_PTR-1)[0];\
	u64 index = 0;\
	while(ring_buffer_pop(TYPE_PTR, (buffer+1) + (index++) * sizeof(*TYPE_PTR));\
	(TYPE*)(buffer+1);\
})







typedef struct{
}SingleLink;

typedef struct{
}DoubleLink;










