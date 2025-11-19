#pragma once
#include "math.h"
#include "global.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdatomic.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>







#define arrlen( x ) (sizeof(x) / sizeof((x)[0]))

#ifndef DEBUG
	#define DEBUG 1
#endif
#ifndef PEDANTIC
	#define PEDANTIC 1
#endif

void stop();

#define ASSERT_LVALUE(x) ({void* assert_lvalue = &(x);})

#ifdef PEDANTIC
	#define PEDANTIC_PRINT( message ) \
	{\
		printf("PEDANTIC_PRINT: %s\n", message);\
	}
	#define PEDANTIC_ASSERT( call, message ) \
	{\
		if((call) == false)\
		{\
			printf("PEDANTIC_ASSERT: %s\n", message);\
			stop();\
		}\
	}
#else

	#define PEDANTIC_ASSERT( call, message )
	#define PEDANTIC_PRINT( message )

#endif // PEDANTIC


#ifdef DEBUG
	#define DEBUG_PRINT( message ) \
	{\
		printf("DEBUG_PRINT: %s\n", message);\
	}

	#define DEBUG_ASSERT( call, message ) \
	{\
		if((call) == false)\
		{\
			printf("DEBUG_ASSERT: %s\n", message);\
			stop();\
		}\
	}

	#define DEBUG_ABORT( message ) \
	{\
		printf("DEBUG_ABORT: %s\n", message);\
		stop();\
	}

	#define DEBUG_CALL(call, message)\
	{\
		if((call) == false)\
		{\
			printf("DEBUG_CALL: %s\n", message);\
		}\
	}
#else
	#define DEBUG_PRINT( message ) 
	#define DEBUG_ASSERT( call, message )
	#define DEBUG_ABORT( message )
	#define DEBUG_CALL(call, message)
#endif // DEBUG

#define RUNTIME_PRINT( message ) \
{\
	printf("RUNTIME_PRINT: %s\n", message);\
}

#define RUNTIME_ASSERT( call, message ) \
{\
	if((call) == false)\
	{\
		printf("RUNTIME_ASSERT: %s\n", message);\
		stop();\
	}\
}

#define RUNTIME_ABORT( message ) \
{\
	printf("RUNTIME_ABORT: %s\n", message);\
	stop();\
}

#define RUNTIME_CALL(call, message)\
{\
	if((call) == false)\
	{\
		printf("RUNTIME_CALL: %s\n", message);\
	}\
}




u8 popcount_uint(u64 a);
b32 uint_is_power_of_two(u64 x);
u64 forward_align_uint(u64 x, const u64 align);
void* forward_align_pointer(void* x, u64 align);
u64 uint_next_power_of_two(u64 x);
u8 uint_trailing_zeros(u64 x);
u8 uint_max_bit_set(u64 x);

void main_init(u64 size);
void main_cleanup();

// ARENA


typedef struct Arena{
	void* base_address;
	u64 base_size;
	void* start;
	void* pos;
	void* last;
	void* end;
	u64 default_alignment;
	u64 page_size;
	b32 raw;
	u64 temp_counter;
	struct Arena* next;
}Arena;


b32 arena_raw(Arena *arena, b32 raw); // Toggle padding
b32 allocate_arena(u64 size, Arena* out);
Arena *get_arena(Arena *arena);
void free_arena(Arena arena);

Arena allocate_sub_arena(u64 size, Arena *arena);
void reset_arena(Arena* arena);
u64 arena_space(Arena *arena);

void* arena_alloc(u64 size, u32 alignment, b32 zero, Arena *arena);
void* arena_alloc_aligned(u64 size, u64 alignment, Arena* arena);
void* arena_alloc_before_alignment(u64 size, u64 before, Arena* arena);

#define memzero(DST, SIZE) memset(DST, 0, SIZE)

#define memset_type(DST, SRC, COUNT)\
({\
	for(u64 i = 0; i < (COUNT); i++)\
	{\
		(DST)[i] = (SRC);\
	}\
	(SRC);\
})

#define forward_itterate(COUNT, CALL)\
({\
	for(u64 i = 0; i < (COUNT); i++)\
	{\
		CALL;\
	}\
})

#define reverse_itterate(COUNT, CALL)\
({\
	for(u64 i = (COUNT-1); i != U64_MAX; i--)\
	{\
		CALL;\
	}\
})


#define swap(TYPE, A, B) \
{\
	TYPE temp = (A);\
	(A) = (B);\
	(B) = temp;\
}


typedef struct{
	Arena* arena;
	void* starting_pos;
}Temp;

Temp begin_temp(Arena* arena);
Arena* end_temp(Temp temp);

typedef struct{
	u32 count;
	u32 index;
	Arena arenas[];
}ArenaRing;

ArenaRing *allocate_arena_ring(u32 count, u64 arena_size, Arena* parent);
Arena* arena_ring_current(ArenaRing* ring);
Arena* arena_ring_increment(ArenaRing* ring, b32 reset);



void* protect_and_clear_pages(void* data, u64 page_count);

u64 get_time_ns();
u64 get_time_us();
u64 get_time_ms();
u64 get_time_s();
u64 get_time_m();
u64 get_time_h();

u64 ns_to_us(u64 ns);
u64 ns_to_ms(u64 ns);
u64 ns_to_s(u64 ns);


u64 mean_time(u64 count, u64* times);
u64 max_time_index(u64 count, u64* times);
u64 min_time_index(u64 count, u64* times);
u64 max_time(u64 count, u64* times);
u64 min_time(u64 count, u64* times);
u64 range_times(u64 count, u64* times);
u64 sum_times(u64 count, u64* times);

#define TIME_CALL( _call )\
	({\
		u64 _time = get_time_ns();\
		(_call);\
		_time = get_time_ns() - _time;\
		_time;\
	})

#define TIME_LOOP( call, ITTITT)\
({\
	u64 start = get_time_ns();\
	for(u64 ITTI = 0; ITTI < ITTITT; ITTI++)\
	{\
		call;\
	}\
	u64 end = get_time_ns();\
	u64 time = end - start;\
	time;\
})

#define TIME_LOOP_MEAN( call, ITTITT)\
({\
	u64 t = (u64)((f64)TIME_LOOP(call, ITTITT) / (f64)ITTITT);\
	t;\
})

u64 sleep_s(u64 s);
u64 sleep_ms(u64 ms);
u64 sleep_us(u64 us);
u64 sleep_ns(u64 ns);

typedef struct{
	u64 start, end, elapsed;
	u64 target, real_elapsed;
}LoopTime;

LoopTime loop_time_init(u64 target);
LoopTime loop_time_start(LoopTime l);
LoopTime loop_time_end(LoopTime l);


typedef struct{
	u8 data[64];
}CacheLine align(64);

u64 splitmix64(u64 s[1]);
u64 splitmix64_hash(u64 s);

u64 romu_quad64(u64 s[4]);
void* romu_quad64x8_scalar(u64* restrict dst8, u64* restrict state32);
void* romu_quad64x8_vector(u64* restrict dst8, u64* restrict state32);
void* romu_quad64x8(u64* restrict dst8, u64* restrict state32);

typedef struct{
	u64 state[32];
}PRNG align(64);

PRNG init_prng(u64 seed);
u64 random_u64(PRNG *prng);
u32 random_u32(PRNG *prng);
s32 random_s32(PRNG *prng);
s64 random_s64(PRNG *prng);
f32 random_f32(PRNG *prng);
f32 compose_random_snorm_f32(u32 random);
CacheLine random_cache_line(PRNG *prng);
void prng_memset(PRNG *prng, void* data, u64 size);



// Global variables

extern Arena main_arena;

b32 char_is_space(u8 c);
b32 char_is_upper(u8 c);
b32 char_is_lower(u8 c);
b32 char_is_letter(u8 c);
b32 char_is_slash(u8 c);
b32 char_is_printable(char c);

b32 cstrings_are_equal(const char* a, const char* b);
u32 cstring_length(const char* cstring);
u64 hash_cstring(const char* c);

typedef struct{
	u8* str;
	u64 size;
}String8;


#define str8_lit( CSTRING ) (String8){(u8*)CSTRING, sizeof(CSTRING)-1}
String8 str8(u8 *str, u64 size);
String8 str8_range(u8 *start, u8* one_past_end);
String8 str8_zero(void);
String8 str8_copy(String8 src, Arena *arena);

String8 str8_random(u64 len, PRNG *rg, Arena* arena);
String8 va_str8_combine(Arena *arena, String8 seperator, u32 count, va_list l);
String8 str8_combine(Arena *arena, String8 seperator, u32 count, ...);
u64 str8_hash(String8 s, u64 seed);
b32 str8_equal(String8 a, String8 b);



// Print

typedef struct{
	const char *format;
	char *start;
	char *out;
	char *end;
	u64 left_precision;
	u64 right_precision;
	b32 is_array;
	b32 has_hidden_count;
	u64 count;
	Type type;
	Type last_type;
}Print;


String8 va_string_print(Arena *arena, const char *format, va_list l);
String8 string_print(Arena *arena, const char *format, ...);
String8 va_arena_print(Arena *arena, const char *format, va_list l);
String8 arena_print(Arena *arena, const char *format, ...);
u32 va_print(const char* format, va_list l);
u32 print(const char* format, ...);


#define atomic _Atomic

u64 sleep_s(u64 s);
u64 sleep_ms(u64 ms);
u64 sleep_us(u64 us);
u64 sleep_ns(u64 ns);

typedef struct{
	void* handle;
}Mutex;

Mutex create_mutex(Arena* arena);
void destroy_mutex(Mutex mtx);
Mutex mutex_lock(Mutex mtx);
Mutex mutex_unlock(Mutex mtx);


typedef struct{
	void* handle;
}Cond;

Cond create_cond(Arena* arena);
void destroy_cond(Cond cond);
Cond cond_wait(Cond cond, Mutex mutex);
Cond cond_timedwait(Cond cond, Mutex mutex, u64 ns);
Cond cond_signal(Cond cond);
Cond cond_broadcast(Cond cond);


typedef struct{
	void* handle;
}Semaphore;

Semaphore create_semaphore(Arena* arena);
void destroy_semaphore(Semaphore semaphore);
Semaphore semaphore_signal(Semaphore semaphore);
Semaphore semaphore_wait(Semaphore semaphore);

typedef struct{
	void* handle;
}Barrier;

Barrier create_barrier(u32 count, Arena* arena);
void destroy_barrier(Barrier barrier);
b32 barrier_wait(Barrier barrier);

typedef struct{

	struct GlobalBoidParams{
		f32 seperation_range_norm;
		f32 cohesion_range_norm;
		f32 alignment_range_norm;
		f32 seperation_strength_norm;
		f32 cohesion_strength_norm;
		f32 alignment_strength_norm;

		f32 max_speed_norm;
		f32 min_speed_norm;
		f32 acceleration_norm;

		f32 random_chance_norm;
		f32 random_strength_norm;

		f32 boid_count_norm;
		f32 boid_size_norm;

		b32 attractor_enable;
		f32 attractor_range_norm;
		f32 attractor_strength_snorm;
		fvec2 attractor_position;

	}boid;
}Global;

typedef struct{
	Arena arena;
	Print print; // Should be zero at thread creation
	Global global;
	void *parameters;
	void *handle;
	void *pfn;
}Thread;

typedef void*(*pfn_thread)(Thread*);

Thread* start_thread(pfn_thread pfn, void *parameters, u64 arena_size, Arena *arena);
b32 set_thread_affinity(Thread *thread, u32 cpu_index);
void* join_thread(Thread *thread);

// Thread
extern thread_local Thread *THREAD;
extern Thread *MAIN_THREAD;
void init_threads();
void cleanup_threads();



typedef struct{
	const char** map;
	const void** pointers;
	u64 mask;
}StringMap;

StringMap alloc_string_map(u64 capacity, Arena* arena);
u64 string_map_insert(StringMap map, const char* cstring, const void* data);
u64 string_map_lookup(StringMap map, const char* cstring, const void** dst);
const void* string_map_lookup_raw(StringMap map, const char* cstring);


u8* read_entire_binary_file(const char* path, Arena* arena);
