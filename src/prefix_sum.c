#include "prefix_sum.h"


typedef u64 uPS;

void prefix_sum_simple(uPS *src, uPS *dst)
{
	uPS total = 0;
	for(u32 i = 0; i < buffer_count(dst); i++)
	{
		total += src[i];
		dst[i] = total;
	}
}

void prefix_sum_init(uPS *dst)
{
	PRNG rg = init_prng(32);
	prng_memset(&rg, dst, buffer_count(dst) * sizeof(*dst));
	for(u32 i = 0; i < buffer_count(dst); i++)
	{
		dst[i] = dst[i] & (0xFFFF);
	}
}


b32 prefix_sum_compare(uPS *a, uPS *b)
{
	return memcmp(a,b, buffer_count(a) * sizeof(*a));
}

void run_prefix_sum(const char *name, uPS *src, uPS *dst, uPS *ref, void(*pfn_prefix_sum)(uPS*, uPS*))
{
	u64 time = TIME_CALL(pfn_prefix_sum(src,dst));
	if(prefix_sum_compare(dst,ref))
	{
		print("%cstr: FAIL\n", name);
	}
	else
	{
		print("%cstr: %u64 ps / number\n", name, time * 1000 / buffer_count(dst));
	}
}

void prefix_sum_bench()
{
	u64 seed = 999445333;
	const u64 test_count = 1024 * 1024;

	Temp temp = begin_temp(&main_arena);
	uPS *src = allocate_buffer(uPS, test_count, temp.arena);
	prefix_sum_init(src);
	uPS *ref = allocate_buffer(uPS, buffer_count(src), temp.arena);
	uPS *dst = allocate_buffer(uPS, buffer_count(src), temp.arena);
	prefix_sum_simple(src, ref);
	prefix_sum_simple(src, dst);
	memzero(dst, buffer_size(dst));

	run_prefix_sum("Simple", src, dst, ref, prefix_sum_simple);
	run_prefix_sum("Simple", src, dst, ref, prefix_sum_simple);
	run_prefix_sum("Simple", src, dst, ref, prefix_sum_simple);
	run_prefix_sum("Simple", src, dst, ref, prefix_sum_simple);
	run_prefix_sum("Simple", src, dst, ref, prefix_sum_simple);

	end_temp(temp);
}

