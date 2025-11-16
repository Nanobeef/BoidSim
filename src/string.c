#include <string.h>
#include "basic.h"
#include "alg.h"


b32 char_is_space(u8 c)
{
	return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

b32 char_is_upper(u8 c)
{
	return (c >= 'A' && c <= 'Z');
}

b32 char_is_lower(u8 c)
{
	return (c >= 'A' && c <= 'Z');
}

b32 char_is_letter(u8 c)
{
	return (char_is_upper(c) || char_is_lower(c));
}

b32 char_is_slash(u8 c)
{
	return (c == '/' || c == '\\');
}

b32 char_is_printable(char c)
{
	return (c >= ' ') && (c < 128);
}

u32 cstring_length(const char* cstring)
{
	return strlen(cstring);
}

b32 cstrings_are_equal(const char* a, const char* b)
{
	while((*a == *b) && (*a) && (*b)) {a++; b++;}
	return (b32)((*a) == (*b));
}

String8 str8(u8 *str, u64 size)
{
	String8 ret = {str, size};
	return ret;
}

String8 str8_range(u8 *start, u8* one_past_end)
{
	String8 ret = {start, (u64)(one_past_end - start)};
	return ret;
}

String8 str8_zero(void)
{
	return (String8){0};
}

String8 str8_copy(String8 src, Arena *arena)
{
	String8 dst = str8(
		arena_alloc(src.size,1,0,arena),
		src.size
	);
	memcpy(dst.str, src.str, dst.size);
	return dst;
}


String8 str8_random(u64 len, PRNG *rg, Arena* arena)
{
	static const char* alnum = "aaabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	u8* d = arena_alloc(sizeof(char) * (len+1),1,0, arena);
	for(u64 i = 0; i < (len >> 3)+1; i++)
	{
		((u64*)d)[i] = random_u64(rg);
	}
	for(u32 i = 0; i < len; i++)
	{
		d[i] = alnum[(d[i] & 63)];
	}
	d[len] = 0;
	return str8((u8*)d,len);
}

String8 va_str8_combine(Arena *arena, String8 seperator, u32 count, va_list l)
{
	arena_raw(arena, true);
	u8* start = arena->pos;
	u8* pos = start;
	for(u32 i = 0; i < count*2-1; i++)
	{
		String8 s;
		if(i & 1)
		{
			s = seperator;
		}
		else
		{
			s = va_arg(l, String8);	
		}
		memcpy(pos, s.str, s.size);
		pos += s.size;
	}
	String8 ret = str8(start, pos-start);
	*pos=0;
	pos++;
	arena->pos = pos;
	arena_raw(arena, false);
	return ret;
}

String8 str8_combine(Arena *arena, String8 seperator, u32 count, ...)
{
	va_list l;
	va_start(l,count);
	String8 ret = va_str8_combine(arena, seperator, count, l);
	va_end(l);
	return ret;
}

u64 str8_hash(String8 s, u64 seed)
{
    u64 hash = seed;
	for(u32 i = 0; i < s.size; i++)
	{
        hash = ((hash << 5) + hash) + s.str[i];  
		hash = hash * 33 + s.str[i];
	}
	return hash;
}

b32 str8_equal(String8 a, String8 b)
{
	if(a.size != b.size)
		return false;
	for(u32 i = 0; i < a.size; i++)
	{
		if(a.str[i] != b.str[i])
		{
			return false;
		}
	}
	return true;
}







u64 hash_cstring(const char* c)
{
    u64 hash = 5381;
	while(c[0])
	{
        hash = ((hash << 5) + hash) + c[0];  
		hash = hash * 33 + c[0];
		c++;
	}
    return hash;
}


StringMap alloc_string_map(u64 capacity, Arena* arena)
{
	capacity = uint_next_power_of_two(capacity);
	StringMap map = {
		.map = allocate_array(const char*, capacity, arena),
		.pointers = allocate_array(const void*, capacity, arena),
		.mask = capacity - 1,
	};
	memzero(map.map, array_capacity_size(map.map));
	return map;
}

u64 string_map_insert(StringMap map, const char* cstring, const void* pointer)
{
	u64 index = hash_cstring(cstring) & map.mask;
	for(u64 i = index; i < array_capacity(map.map); i++)
	{
		if(map.map[i] == NULL)
		{
			map.map[i] = cstring;
			map.pointers[i] = pointer;
			return i;
		}
	}
	for(u64 i = 0; i < index; i++)
	{
		if(map.map[i] == NULL)
		{
			map.map[i] = cstring;
			map.pointers[i] = pointer;
			return i;
		}
	}
	return array_capacity(map.map);
}

u64 string_map_lookup(StringMap map, const char* cstring, const void** dst)
{
	u64 index = hash_cstring(cstring) & map.mask;
	for(u64 i = index; i < array_capacity(map.map); i++)
	{
		if(map.map[i] == 0){return false;}
		if(cstrings_are_equal(map.map[i], cstring))
		{
			dst[0] = map.pointers[i];
			return i;
		}
	}
	for(u64 i = 0; i < index; i++)
	{
		if(map.map[i] == 0){return false;}
		if(cstrings_are_equal(map.map[i], cstring))
		{
			dst[0] = map.pointers[i];
			return i;
		}
	}
	return array_capacity(map.map);
}

const void* string_map_lookup_raw(StringMap map, const char* cstring)
{
	const void* data = 0;
	if(string_map_lookup(map, cstring, &data) == 0)
	{
		return 0;	
	}
	return data;
}
