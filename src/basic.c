#include "basic.h"
#include <simde/x86/avx512.h>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>



b32 global_close_window = false;
b32 global_keep_terminal_open = false;

void stop()
{
	printf(":::ABORT:::\n");
	abort();
}

u8 popcount_uint(u64 a)
{
	u8 c = 0;
	for(u8 i = 0; i < 64; i++) {
		c += (a & (1<<i));
	}
	return c;	
}

b32 uint_is_power_of_two(u64 x)
{
	return (x > 0) && ((x & (x-1)) == 0);
}

u64 forward_align_uint(u64 x, const u64 align)
{
	DEBUG_ASSERT(uint_is_power_of_two(align), "Forward Alignment must be a power of 2\n");
	x = (x + (align - 1)) & ~(align - 1);
	return x;
}

void* forward_align_pointer(void* x, u64 align)
{
	return (void*)forward_align_uint((u64)x, align);
}

u64 uint_next_power_of_two(u64 x)
{
	if(x == 0)
	{
		return 1;
	}
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	x++;
	return x;
}

u8 uint_trailing_zeros(u64 x)
{
	DEBUG_ASSERT(x != 0, "Input of zero is undefined");
	return (u8)__builtin_ctz(x);	
}

u8 uint_max_bit_set(u64 x)
{
	return 1 << (31-__builtin_clz(x));

}


void main_init(u64 size)
{
	while(allocate_arena(size, &main_arena) == false)
	{
		//printf("%lu\n", size);
		size /= 2;
	}
	init_threads();
}

void main_cleanup()
{
	cleanup_threads();
	free_arena(main_arena);
	if(global_keep_terminal_open){ getchar();};
}


b32 arena_raw(Arena *arena, b32 raw)
{
	DEBUG_ASSERT(arena->raw != raw, "Arena is already in RAW mode!");
	return arena->raw = raw;
}

Arena make_arena(u64 size, void* data)
{
	Arena arena = {
		.base_address = data,
		.base_size = size,
		.start = data,
		.pos = data,
		.last = data,
		.end = data + size - sizeof(Arena),
		.default_alignment = 64,
		.page_size = 4096,
		.next = 0,
	};
	return arena;
}

Arena *get_arena(Arena *arena)
{
	if(arena == 0)
	{
		PEDANTIC_ASSERT(THREAD->arena.start != NULL, "Thread arena is uninitialized!");
		return &THREAD->arena;
	}
	return arena;
}



b32 allocate_arena(u64 size, Arena* out)
{
	size = forward_align_uint(size, 4096);
	void* data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0,0);				
	if(data == MAP_FAILED)
	{
	//	perror("mmap");
		return false;
	}
	*out = make_arena(size, data);
	return true;
}
void free_arena(Arena arena)
{
	munmap(arena.base_address, arena.base_size);
}


void* arena_alloc_only_increment(u64 size, Arena* arena)
{
	arena = get_arena(arena);
	void* data = arena->pos;
	arena->last = arena->pos;
	arena->pos += size;
	return data;
}

void* arena_alloc_no_alignment(u64 size, Arena* arena)
{
	arena = get_arena(arena);
	if(arena->pos + size > arena->end)
	{
		DEBUG_ABORT("Arena overflow!\n");
		return 0;
	}
	void* data = arena->pos;
	arena->last = arena->pos;
	arena->pos += size;
	return data;
}

void* arena_alloc_aligned(u64 size, u64 alignment, Arena* arena)
{
	arena = get_arena(arena);
	arena->pos = forward_align_pointer(arena->pos, alignment);
	if(arena->pos + size > arena->end)
	{
		DEBUG_ABORT("Arena overflow!\n");
		return 0;
	}
	void* data = arena->pos;
	arena->last = arena->pos;
	arena->pos += size;
	return data;
}

void *arena_alloc(u64 size, u32 alignment, b32 zero, Arena *arena)
{
	arena = get_arena(arena);
	switch(alignment)
	{
		case 0:
			arena->pos = (void*)(((u64)arena->pos & ~63) + 64);
		break;
		case 1:
			// Do nothing
		break;
		default:
			arena->pos = forward_align_pointer(arena->pos, alignment);
		break;
	}
	if(arena->pos + size > arena->end)
	{
		DEBUG_PRINT("Arena overflow!\n");
		return 0;
	}
	void* data = arena->pos;
	arena->last = arena->pos;
	arena->pos += size;
	if(zero)
	{
		memset(data, 0, size);
	}
	return data;
}


void* arena_alloc_before_alignment(u64 size, u64 before, Arena* arena)
{
	arena = get_arena(arena);
	PEDANTIC_ASSERT(before <= arena->default_alignment, "This value needs to be smaller than the arenas default alignment!");

	void* aligned = forward_align_pointer(arena->pos, arena->default_alignment);
	if(aligned - before < arena->pos)
	{
		arena->pos = aligned + arena->default_alignment - before;	
	}
	else
	{
		arena->pos = aligned - before;
	}
	void* data = arena->pos;
	arena->last = arena->pos;
	arena->pos += size;
	return data;
}

Arena allocate_sub_arena(u64 size, Arena *parent)
{
	size = forward_align_uint(size, parent->page_size);
	void* data = arena_alloc(size,parent->page_size,0, parent);
	Arena child = make_arena(size, data);
	return child;
}

void reset_arena(Arena* arena)
{
	arena->pos = arena->start;
}

u64 arena_space(Arena *arena)
{
	return arena->end - arena->pos;
}

Temp begin_temp(Arena* arena)
{
	arena = get_arena(arena);
	arena->temp_counter++;
	Temp temp = {
		.arena = arena,
		.starting_pos = arena->pos,
	};
	return temp;
}
Arena* end_temp(Temp temp)
{
	temp.arena->temp_counter--;
	temp.arena->pos = temp.starting_pos;
	return temp.arena;
}

ArenaRing *allocate_arena_ring(u32 count, u64 arena_size, Arena* parent)
{
	ArenaRing *ring = arena_alloc(sizeof(ArenaRing) + sizeof(Arena) * count,0,0, parent);
	ring->count = count;
	ring->index = 0;
	for(u32 i = 0; i < count; i++)
	{
		ring->arenas[i] = allocate_sub_arena(arena_size, parent);
	}
	return ring;
}
Arena* arena_ring_current(ArenaRing* ring)
{
	return &ring->arenas[ring->index];
}
Arena* arena_ring_increment(ArenaRing* ring, b32 reset)
{
	ring->index = (ring->index + 1) % ring->count;
	Arena* current = arena_ring_current(ring);
	if(reset)
	{
		reset_arena(current);
	}
	return current;
}

void* protect_and_clear_pages(void* data, u64 page_count)
{
	DEBUG_ASSERT(((u64)data & 4095lu) == 0lu, "Protection can only work for whole pages\n");
	void* res = mmap(data, 4096 * page_count, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS| MAP_FIXED, 0,0);
	if(res == MAP_FAILED)
	{
		perror("map failed\n");
		return 0;
	}
	return data;
}

u64 get_time_ns()
{
	struct timespec ts = {0};
	if(clock_gettime(CLOCK_REALTIME, &ts))
	{
		// ERROR
		return 0;
	}
	u64 ns = ts.tv_sec * 1000000000 + ts.tv_nsec;
	return ns;
}
u64 get_time_us()
{
	return get_time_ns() / 1000;
}
u64 get_time_ms()
{
	return get_time_ns() / 1000000;
}
u64 get_time_s()
{
	struct timespec ts = {0};
	if(clock_gettime(CLOCK_REALTIME, &ts))
	{
		// ERROR
		return 0;
	}
	u64 s = ts.tv_sec;
	return s;
}
u64 get_time_m()
{
	return get_time_s() / 60;
}
u64 get_time_h()
{
	return get_time_s() / 3600;
}

u64 ns_to_us(u64 ns)
{
	return ns / 1000;
}
u64 ns_to_ms(u64 ns)
{
	return ns / 1000000;
}
u64 ns_to_s(u64 ns)
{
	return ns / 1000000000;
}

u64 mean_time(u64 count, u64* times)
{
	u64 average = 0;
	for(u64 i = 0; i < count; i++)
	{
		average += times[i];	
	}
	return average / count;
}

u64 max_time_index(u64 count, u64* times)
{
	u64 max = 0;
	u64 max_index = 0;
	for(u64 i = 0; i < count; i++)
	{
		if(times[i] > max)
		{
			max = times[i];
			max_index = i;
		}
	}
	return max_index;
}

u64 min_time_index(u64 count, u64* times)
{
	u64 min = U64_MAX;
	u64 min_index = 0;
	for(u64 i = 0; i < count; i++)
	{
		if(times[i] < min)
		{
			min = times[i];
			min_index = i;
		}
	}
	return min_index;
}

u64 max_time(u64 count, u64* times)
{
	u64 max = 0;
	for(u64 i = 0; i < count; i++)
	{
		if(times[i] > max)
		{
			max = times[i];
		}
	}
	return max;
}

u64 min_time(u64 count, u64* times)
{
	u64 min = U64_MAX;
	for(u64 i = 0; i < count; i++)
	{
		if(times[i] < min)
		{
			min = times[i];
		}
	}
	return min;
}

u64 range_times(u64 count, u64* times)
{
	return max_time(count, times) - min_time(count, times);
}

u64 sum_times(u64 count, u64* times)
{
	u64 sum = 0;
	for(u64 i = 0; i < count; i++)
	{
		sum += times[i];
	}
	return sum;
}


u64 sleep_s(u64 s)
{
	sleep(s);
	return s;
}
u64 sleep_ms(u64 ms)
{
	usleep(ms * 1000);
	return ms;
}
u64 sleep_us(u64 us)
{
	usleep(us);
	return us;
}
u64 sleep_ns(u64 ns)
{
	struct timespec ts;
	ts.tv_nsec = ns;
	ts.tv_sec = 0;
	nanosleep(&ts, 0);
	return ns;
}


LoopTime loop_time_init(u64 target)
{
	u64 time = get_time_ns();
	LoopTime l = {
		.start = time,
		.end = time,
		.elapsed = 0,
		.target = target,
	};
	return l;
}

LoopTime loop_time_start(LoopTime l)
{
	l.start = get_time_ns();
	return l;
}

LoopTime loop_time_end(LoopTime l)
{
	l.end = get_time_ns();
	l.elapsed = l.end - l.start;
	if(l.elapsed < l.target)
	{
		sleep_ns(l.target - l.elapsed);
	}
	l.real_elapsed = get_time_ns() - l.start;
	return l;
}

u64 splitmix64(u64 s[1])
{
    u64 x = (s[0] += UINT64_C(0x9e3779b97f4a7c15));
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;
    return x;
}

u64 splitmix64_hash(u64 s)
{
    u64 x = (s + UINT64_C(0x9e3779b97f4a7c15));
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;
    return x;
}

void seed_rng(u64 seed, u64 count, u64* state)
{
	for(u64 i = 0; i < count; i++)
	{
		state[i] = splitmix64(&seed);
	}
}


#define ROMU_ROTL(d, l) ((d<<(l)) |  (d>>(8*sizeof(d)-(l))))

u64 romu_quad64(u64 s[4])
{
	u64 w = s[0];
	u64 x = s[1];
	u64 y = s[2];
	u64 z = s[3];
	s[0] = U64(15241094284759029579) * z;
	s[1] = z + ROMU_ROTL(w, 52);
	s[2] = y - x;
	s[3] = y + w;
	s[3] = ROMU_ROTL(s[3], 19);
	return x;
}

void* romu_quad64x8_scalar(u64* restrict dst8, u64* restrict state32)
{
	for(u32 i = 0; i < 8; i++)
	{
		u64 state[4] = {
			state32[i],
			state32[i+8],
			state32[i+16],
			state32[i+24]
		};
		dst8[i] = romu_quad64(state);
		
		state32[i] = 	state[0];
		state32[i+8] = 	state[1];
		state32[i+16] = state[2];
		state32[i+24] = state[3];
	}
	return dst8;
}

void* romu_quad64x8(u64* restrict dst8, u64* restrict state32)
{
	const simde__m512i magic_constant = simde_mm512_set1_epi64(U64(15241094284759029579));

	simde__m512i s0 = simde_mm512_load_epi64(state32);
	simde__m512i s1 = simde_mm512_load_epi64(state32 + 8);
	simde__m512i s2 = simde_mm512_load_epi64(state32 + 16);
	simde__m512i s3  = simde_mm512_load_epi64(state32 + 24);

	simde__m512i w = s0;
	simde__m512i x = s1;
	simde__m512i y = s2;
	simde__m512i z = s3;

	simde__m512i temp; 

	s0 = simde_mm512_mullo_epi64(magic_constant, z);

	temp = simde_mm512_rol_epi64(w, 52);
	s1 = simde_mm512_add_epi64(z, temp);

	s2 = simde_mm512_sub_epi64(y,x);

	s3 = simde_mm512_add_epi64(y,w);
	s3 = simde_mm512_rol_epi64(s3, 19);

	simde_mm512_store_epi64(state32,			s0);
	simde_mm512_store_epi64(state32 + 8,		s1);
	simde_mm512_store_epi64(state32 + 16,	s2);
	simde_mm512_store_epi64(state32 + 24,	s3);

	simde_mm512_storeu_epi64(dst8, x); // Doing unaligned store for now
	return dst8;
}



PRNG init_prng(u64 seed)
{
	PRNG prng;
	for(u32 i = 0; i < 32; i++)
	{
		prng.state[i] = splitmix64(&seed);	
	}
	return prng;
}

u64 random_u64(PRNG *prng)
{
	return romu_quad64(prng->state);
}

u32 random_u32(PRNG *prng)
{
	return (u32)romu_quad64(prng->state);
}

s32 random_s32(PRNG *prng)
{
	s32 s;
	u32 u = (u32)romu_quad64(prng->state);
	memcpy(&s, &u, 4);
	return s;
}

s64 random_s64(PRNG *prng)
{
	s64 s;
	u64 u = romu_quad64(prng->state);
	memcpy(&s, &u, 8);
	return s;
}

f32 compose_random_snorm_f32(u32 random)
{
	u32 bits = (127u << 23) | (random & 0x7FFFFF);		
	f32 f;
	memcpy(&f, &bits, sizeof(u32));
	return (f - 1.5f) * 2.0f;
}

f32 random_f32(PRNG *prng)
{
	u32 u = romu_quad64(prng->state);
	return compose_random_snorm_f32(u);
}

CacheLine random_cache_line(PRNG *prng)
{
	CacheLine line;
	romu_quad64x8((u64*)&line, prng->state);
	return line;
}

void prng_memset(PRNG *prng, void* data, u64 size)
{
	PEDANTIC_ASSERT(((u64)prng & 63) == 0, "PRNG is not aligned to 64!");
	//PEDANTIC_ASSERT(((u64)data & 63) == 0, "DATA is not aligned to 64!");
	if(size < 8)
	{
		u64 v = random_u64(prng);
		memcpy(data, &v, size);
	}
	else if(size < 64)
	{
		CacheLine line;
		line = random_cache_line(prng);
		memcpy(data, &line, size);
	}
	else
	{
		CacheLine line;
		u64 line_count = size / 64;
		for(u64 i = 0; i < line_count; i++)
		{
			line = random_cache_line(prng);
			memcpy((CacheLine*)data + i, &line, 64);
		}
		u64 rem = (size & 63);
		if(rem)
		{
			line = random_cache_line(prng);
			memcpy((u8*)data + (size - rem), &line, rem); 
		}
	}
}








