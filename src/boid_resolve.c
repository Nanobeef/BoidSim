#include "boid.h"
#include <simde/x86/avx512.h>
#include "device_graphics.h"

/*
__m512 _mm512_mask_compress_ps (__m512 src, __mmask16 k, __m512 a);
__m512 _mm512_maskz_compress_ps (__mmask16 k, __m512 a);

void _mm512_mask_compressstoreu_ps (void* base_addr, __mmask16 k, __m512 a);

int _mm_popcnt_u32 (unsigned int a);

*/

void boid_sim_resolve_test()
{
}

