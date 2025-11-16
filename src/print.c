#include "print.h"
#include "basic.h"
#include <math.h>

// Most bounds cheks have been removed, just going to have checks for printing array types.

typedef struct{
	const char* str;
	u32 len;
	Type type;
}Keyword;

#define init_keyword( STR , TYPE) (Keyword){ STR, arrlen(STR)-1, TYPE}

const Keyword keywords[] = { // Sort manually for now.
	init_keyword("b16", TYPE_B16),
	init_keyword("b32", TYPE_B32),
	init_keyword("b64", TYPE_B64),
	init_keyword("b8", TYPE_B8),

	init_keyword("cstr", TYPE_CSTRING),

	init_keyword("f16", TYPE_F16),
	init_keyword("f32", TYPE_F32),
	init_keyword("f64", TYPE_F64),

	init_keyword("fmat2", TYPE_F32MAT2),
	init_keyword("fmat3", TYPE_F32MAT3),
	init_keyword("fmat4", TYPE_F32MAT4),

	init_keyword("fvec2", TYPE_F32VEC2),
	init_keyword("fvec3", TYPE_F32VEC3),
	init_keyword("fvec4", TYPE_F32VEC4),


	init_keyword("s16", TYPE_S16),
	init_keyword("s32", TYPE_S32),
	init_keyword("s64", TYPE_S64),
	init_keyword("s8", TYPE_S8),
	
	init_keyword("str8", TYPE_STRING),

	init_keyword("svec2", TYPE_S32VEC2),

	init_keyword("u16", TYPE_U16),
	init_keyword("u32", TYPE_U32),
	init_keyword("u64", TYPE_U64),
	init_keyword("u8", TYPE_U8),

	init_keyword("uvec2", TYPE_U32VEC2),

	init_keyword("v16", TYPE_V16),
	init_keyword("v32", TYPE_V32),
	init_keyword("v64", TYPE_V64),
	init_keyword("v8", TYPE_V8),
};

b32 is_number(char c)
{
	return ((c >= '0') && (c <= '9'));
}

u64 string_to_uint(const char **str)
{
	const char *p, *end;
	p = *str;
	while(is_number(*p)){p++;}; 
	end = p;
	u64 n = 0;
	u64 base = 1;
	do{
		p--;	
		u64 x = (u64)(p[0] - '0') * base;
		n += x;
		base *= 10;
	}
	while(p != *str);
	*str = end;
	return n;
}


char* cstring_to_string(const char *cstr, char *out, char *end)
{
	while(*cstr && (out != end))
	{
		*out++ = *cstr++;
	}
	return out;
}


char* sint_to_string(s64 n, char *out, char *end)
{
	char buffer[arrlen("18446744073709551615")];
	u32 c = 0;
	b32 is_negative = false;
	if(n == 0)
	{
		*out++ = '0';
	}
	if(n < 0)
	{
		is_negative = true;
		n = -n;
	}

	while(n)
	{
		u64 x = n % 10;	
		n = n / 10;
		buffer[c] = (x + '0');
		c++;
	}
	if(is_negative)
	{
		*out++ = '-';
	}
	while(out != end && c)
	{
		c--;
		out[0] = buffer[c];
		out++;
	}
	return out;
}

char* uint_to_string(u64 n, char *out, char *end)
{
	char buffer[arrlen("18446744073709551615")];
	u32 c = 0;
	if(n == 0)
	{
		*out++ = '0';
	}
	while(n)
	{
		u64 x = n % 10;	
		n = n / 10;
		buffer[c] = (x + '0');
		c++;
	}
	while(out != end && c)
	{
		c--;
		out[0] = buffer[c];
		out++;
	}
	return out;
}

char* void_to_string(u64 v, u32 bit_count, char *out, char *end)
{
	char bits[64];	
	for(u32 i = 0; i < bit_count; i++)
	{
		char b = (char)((v>>i) & 1) + '0';
		bits[i] = b;
	}
	for(u32 i = bit_count; i > 0; i--)
	{
		if(out == end)
		{
			return out;
		}
		out[0] = bits[i-1];
		out++;
	}
	return out;
}

char* float_to_string(f64 n, u64 left_precision, u64 right_precision, char *out, char *end)
{

	//out += snprintf(out, (u64)(end - out), "%f", n);
	//return out;
	
	b32 is_negative = false;
	if(n < 0)
	{
		is_negative = true;
		n = -n;
	}

	u64 ipart = (u64)n;
	f64 fpart = n - (f64)ipart;
	u64 ilength = (u64)out;
	if(is_negative)
	{
		*out++ = '-';
	}
	out = uint_to_string(ipart, out, end);
	ilength = (u64)out - ilength;
	u64 rem_figures = 0;
	if(left_precision > ilength)
	{
		rem_figures = left_precision - ilength;
	}
	else if(left_precision == 0)
	{
		if(right_precision == 0)
		{
			rem_figures = 1;
		}
		else
		{
			rem_figures = right_precision;
		}
	}
	else
	{
		return out;
	}
	u64 pow = 1;
	for(u32 i = 0; i < rem_figures; i++)
	{
		pow *= 10;
	}
	fpart = fpart * (f64)pow;
//	printf("%lu, %lu\n", (u64)ceil(fpart), rem_figures);
	*out++ = '.';
	out = uint_to_string((u64)round(fpart), out, out + rem_figures);
	return out;
}

void handle_scalar_keyword(Print *p, Type type, const void *data);
void handle_vector_keyword(Print *p, Type scalar_type, u32 count, const void* data);
void handle_array_keyword(Print *p, Type scalar_type, u32 count, const void *data);

void handle_scalar_keyword(Print *p, Type type, const void *data)
{
	switch(type)
	{
		case TYPE_V8:
		{

			u64 n = *(u8*)data;
			p->out = void_to_string(n, 8, p->out, p->end);
		}break;
		case TYPE_V16:
		{
			u64 n = *(u16*)data;
			p->out = void_to_string(n, 16, p->out, p->end);
		}break;
		case TYPE_V32:
		{
			u64 n = *(u32*)data;
			p->out = void_to_string(n, 32, p->out, p->end);
		}break;
		case TYPE_V64:
		{
			u64 n = *(u64*)data;
			p->out = void_to_string(n,64, p->out, p->end);
		}break;

		case TYPE_U8:
		{
			u64 n = *(u8*)data;
			p->out = uint_to_string(n, p->out, p->end);
		}break;
		case TYPE_U16:
		{
			u64 n = *(u16*)data;
			p->out = uint_to_string(n, p->out, p->end);
		}break;
		case TYPE_U32:
		{
			u64 n = *(u32*)data;
			p->out = uint_to_string(n, p->out, p->end);
		}break;
		case TYPE_U64:
		{
			u64 n = *(u64*)data;
			p->out = uint_to_string(n, p->out, p->end);
		}break;

		case TYPE_S8:
		{
			s64 n = *(s8*)data;
			p->out = sint_to_string(n, p->out, p->end);
		}break;
		case TYPE_S16:
		{
			s64 n = *(s16*)data;
			p->out = sint_to_string(n, p->out, p->end);
		}break;
		case TYPE_S32:
		{
			s64 n = *(s32*)data;
			p->out = sint_to_string(n, p->out, p->end);
		}break;
		case TYPE_S64:
		{
			s64 n = *(s64*)data;
			p->out = sint_to_string(n, p->out, p->end);
		}break;

		case TYPE_F16:
		{
			f64 n = *(f16*)data;
			p->out = float_to_string(n, p->left_precision, p->right_precision, p->out, p->end);
		}break;
		case TYPE_F32:
		{
			f64 n = *(f32*)data;
			p->out = float_to_string(n, p->left_precision, p->right_precision, p->out, p->end);
		}break;
		case TYPE_F64:
		{
			f64 n = *(f64*)data;
			p->out = float_to_string(n, p->left_precision, p->right_precision, p->out, p->end);
		}break;
		case TYPE_CSTRING:
		{
			char* cstring = (char*)data;
			p->out = cstring_to_string(cstring, p->out, p->end);
		}break;
		case TYPE_STRING:
		{
			String8* s = (String8*)data;
			for(u64 i = 0; i < s->size; i++){*p->out++ = s->str[i];};
		}break;
		default:
			return;
	}
}

u64 type_stride(Type type)
{
	switch(type)
	{
		case TYPE_U8:
		case TYPE_V8:
		case TYPE_S8:
		return 1;
		case TYPE_U16:
		case TYPE_V16:
		case TYPE_S16:
		case TYPE_F16:
		return 2;
		case TYPE_U32:
		case TYPE_V32:
		case TYPE_S32:
		case TYPE_F32:
		return 4;
		case TYPE_U64:
		case TYPE_V64:
		case TYPE_S64:
		case TYPE_F64:
		return 8;
		default:
		return 0;
	}
}

void handle_vector_keyword(Print *p, Type scalar_type, u32 count, const void* data)
{
	*p->out++ = '(';
	for(u32 i = 0; i < count; i++)
	{
		handle_scalar_keyword(p, scalar_type, (u8*)data + type_stride(scalar_type) * i);
		*p->out++ = ',';
		*p->out++ = ' ';
	}
	p->out -= 2;
	*p->out++ = ')';
}

void handle_matrix_keyword(Print *p, Type scalar_type, u32 x_count, u32 y_count, const void* data)
{
	for(u32 y = 0; y < y_count; y++)
	{
		*p->out++ = '[';
		for(u32 x = 0; x < x_count; x++)
		{
			handle_scalar_keyword(p, scalar_type, (u8*)data + type_stride(scalar_type) * (x + x_count * y));
			*p->out++ = ',';
			*p->out++ = ' ';
		}
		p->out-=2;
		*p->out++ = ']';
		*p->out++ = '\n';
	}
	p->out--;
	*p->out++ = '.';
	*p->out++ = '\n';
}

void handle_array_keyword(Print *p, Type scalar_type, u32 count, const void *data)
{
	u64 a = 0;
	u64 b = count;
	if(p->left_precision)
	{
		a = defmin(count, p->left_precision);
	}
	if(p->right_precision)
	{
		b = defmin(count, p->right_precision);
	}
	*(p->out)++ = '[';
	p->out = uint_to_string(count, p->out, p->end);
	*(p->out)++ = ']';

	if((a <= b) && ((a != 0) || (b != count)))
	{
		*(p->out)++ = '{';
		p->out = uint_to_string(a, p->out, p->end);
		*(p->out)++ = ':';
		p->out = uint_to_string(b, p->out, p->end);
		*(p->out)++ = '}';
	}

	*(p->out)++ = '(';
	for(u32 i = 0; i < count; i++)
	{
		handle_scalar_keyword(p, scalar_type, (u8*)data + type_stride(scalar_type) * i);
		*p->out++ = ',';
		*p->out++ = ' ';
	}
	p->out -= 2;
	*p->out++ = ')';
	*(p->out)++ = '.';
}


void handle_keyword(Print *p, va_list l)
{
	if(p->is_array) // DS types
	{
		void * ptr = va_arg(l, void*);
		u64 count = 0;
		if(p->has_hidden_count)
		{
			if(ptr != 0)
			{
				count = ((u64*)ptr)[-1];
			}
		}
		else
		{
			count = p->count;
		}
		handle_array_keyword(p, p->type, count, ptr);
	}
	else
	{ 	
		switch(p->type)
		{
		case TYPE_V8:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_V16:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_V32:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_V64:
		{
			u64 n = (u64)va_arg(l,u64);
			handle_scalar_keyword(p, p->type, &n);
		}break;

		case TYPE_U8:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_U16:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_U32:
		{
			u64 n = (u64)va_arg(l,u32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_U64:
		{
			u64 n = (u64)va_arg(l,u64);
			handle_scalar_keyword(p, p->type, &n);
		}break;

		case TYPE_S8:
		{
			s64 n = (s64)va_arg(l,s32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_S16:
		{
			s64 n = (s64)va_arg(l,s32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_S32:
		{
			s64 n = (s64)va_arg(l,s32);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_S64:
		{
			s64 n = (s64)va_arg(l,s64);
			handle_scalar_keyword(p, p->type, &n);
		}break;

		case TYPE_F16:
		{
			f64 n = (f64)va_arg(l, f64);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_F32:
		{
			f64 n = (f64)va_arg(l, f64);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_F64:
		{
			f64 n = (f64)va_arg(l, f64);
			handle_scalar_keyword(p, p->type, &n);
		}break;
		case TYPE_CSTRING:
		{
			char* cstring = (char*)va_arg(l, char*);
			handle_scalar_keyword(p, p->type, cstring);
		}break;
		case TYPE_STRING:
		{
			String8 string = va_arg(l, String8);
			handle_scalar_keyword(p, p->type, &string);
		}break;

		case TYPE_U32VEC2:
		{
			uvec2 v = va_arg(l, uvec2);
			handle_vector_keyword(p, TYPE_U32, 2, &v);
		}break;

		case TYPE_S32VEC2:
		{
			svec2 v = va_arg(l, svec2);
			handle_vector_keyword(p, TYPE_S32, 2, &v);
		}break;

		case TYPE_F32VEC2:
		{
			fvec2 v = va_arg(l, fvec2);
			handle_vector_keyword(p, TYPE_F32, 2, &v);
		}break;
		case TYPE_F32VEC3:
		{
			fvec3 v = va_arg(l, fvec3);
			handle_vector_keyword(p, TYPE_F32, 3, &v);
		}break;
		case TYPE_F32VEC4:
		{
			fvec4 v = va_arg(l, fvec4);
			handle_vector_keyword(p, TYPE_F32, 4, &v);
		}break;
		case TYPE_F32MAT2:
		{
			fmat2 m = va_arg(l, fmat2);
			handle_matrix_keyword(p, TYPE_F32, 2,2, &m);
		}break;
		case TYPE_F32MAT3:
		{
			fmat3 m = va_arg(l, fmat3);
			handle_matrix_keyword(p, TYPE_F32, 3,3, &m);
		}break;
		case TYPE_F32MAT4:
		{
			fmat4 m = va_arg(l, fmat4);
			handle_matrix_keyword(p, TYPE_F32, 4,4, &m);
		}break;

		default:
			printf("This keyword can not be printed: %u\n", p->type);
		break;
		}
	}
}

void handle_escape(Print *p, va_list l)
{
	const char *start_format = p->format;
	if(p->format[0] == '%')
	{
		*p->out++ = '%';
		p->format++;
		return;
	}
	else if(p->format[0] == '#')
	{
		p->format++;
		p->is_array = true;
		p->has_hidden_count = true;
	}
	b32 precision_set = false;

	if(is_number(p->format[0]))
	{
		p->left_precision = string_to_uint(&p->format);
		precision_set = true;
	}
	else if(p->format[0] == '*') 
	{
		p->left_precision = va_arg(l, u32); 
		p->format++;
		precision_set = true;
	}
	else if(p->format[0] == '$')
	{
		p->count = va_arg(l, u64); 
		p->format++;
		p->has_hidden_count = false;
	}

	b32 has_dot = false;
	if(p->format[0] == '.') {p->format++; has_dot = true;}

	if(is_number(p->format[0]))
	{
		p->right_precision = string_to_uint(&p->format);
		precision_set = true;
	}
	else if(p->format[0] == '*') 
	{
		p->right_precision = va_arg(l, u32); 
		p->format++;
		precision_set = true;
	}
	if((has_dot == true) && (precision_set == false))
	{
		p->left_precision = false;	
		p->right_precision = false;	
	}

	if(p->format[0] == 0)
	{
		return;
	}
	{
		u32 match_count  = 2;
		u32 a = 0;
		u32 b = arrlen(keywords)-1;
		u32 j = 0; // format
		while((match_count) || is_number(p->format[j]))
		{
			//printf("----\n");
			match_count = 0;
			for(u32 i = a; i < b; i++)
			{
				Keyword k = keywords[i];
				if(k.len > j)
				{
					if(k.str[j] == p->format[j])
					{
						if(match_count == 0)
						{
							a = i;
						}
						//printf("YES: %s %s %u\n", keywords[a].str, keywords[b].str,i);
						match_count++;
					}
					else
					{
						if(match_count)
						{
							b = i;	
						}
						//printf("NO:  %s %s %u\n", keywords[a].str, keywords[b].str,i);
					}
				}
			}
			j++;
			if(match_count == 0)
			{
				break;
			}
			else if(match_count == 1)
			{
				if(j == keywords[a].len)
				{
					p->last_type = p->type;
					p->type = keywords[a].type;
					if((p->type != p->last_type) && (precision_set == false))
					{
						p->left_precision = p->right_precision = 0;
					}


					handle_keyword(p, l);

					p->format += j;
					return;
				}
			}
			//printf("f %u %s\n", j, keywords[a].str);
		}
		p->format = start_format;
		handle_keyword(p, l);
	}
}



String8 va_string_print(Arena *arena, const char *format, va_list l)
{
	if(format == 0){return (String8){0};}

	arena_raw(arena, true);
	{
		char* start = arena->pos;
		char* out = start;
		char* end = arena->end;

		THREAD->print.format = format;
		THREAD->print.start = start;
		THREAD->print.out = out;
		THREAD->print.end = end;
		THREAD->print.is_array = false;
		THREAD->print.has_hidden_count = false;
		THREAD->print.count = 0;
	}
	Print *p = &THREAD->print;
	while((p->format[0]) && (p->out != p->end))
	{
		if(p->format[0] == '%')
		{
			p->format++;
			if(p->format[0] == 0)
			{
				handle_keyword(p, l);
				break;
			}
			else
			{
				handle_escape(p, l);
			}
		}
		else
		{
			*p->out++ = p->format[0];	
			p->format++;	
		}
	}
	*p->out++ = 0;
	arena->pos = p->out+1;
	String8 ret;
	ret.size = (u64)(p->out - p->start);
	ret.str = (u8*)p->start;
	arena_raw(arena, false);
	return ret;
}


String8 string_print(Arena *arena, const char *format, ...)
{
	va_list l;
	va_start(l, format);
	String8 ret = va_string_print(arena, format, l);
	va_end(l);
	return ret;
}

String8 va_arena_print(Arena *arena, const char *format, va_list l)
{
	String8 string = va_string_print(arena, format, l);		
	fputs((char*)string.str, stdout);
	fflush(stdout);
	return string;
}

String8 arena_print(Arena *arena, const char *format, ...)
{
	va_list l;
	va_start(l, format);
	String8 ret = va_arena_print(arena, format, l);
	va_end(l);
	return ret;
}

u32 va_print(const char* format, va_list l)
{
	Temp temp = begin_temp(0);
	String8 string = va_arena_print(temp.arena, format, l);
	end_temp(temp);
	return string.size;
}

u32 print(const char* format, ...)
{
	va_list l;
	va_start(l, format);
	u32 ret = va_print(format, l);
	va_end(l);
	return ret;
}







