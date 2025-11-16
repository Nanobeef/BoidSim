#pragma once

#include "basic.h"

static u32 collatz(u64 x)
{
	u32 c = 0;
	while(x != 1)
	{
		if(x & 1)
			x = x * 3 + 1;
		else
			x = x >> 1; 
		c++;
	}
	return c;
}

static void run_collatz()
{
	print("%u64\n", collatz(U32_MAX - 1221llu));
}

