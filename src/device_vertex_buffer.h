#pragma once

#include "vk.h"



// Vertex buffer:
// Array for vertices and indices
// Mode 1): vertex_buffer_draw_indexed(...)
	// [indices -> ... <- vertices]
	// indexed draw commands will draw vertices in reverse order.
// Mode 2): vertex_buffer_draw(...)
	// No indices vertices are drawn in standard order

typedef struct DeviceVertexBufferLink{
	DeviceBuffer buffer;
	struct DeviceVertexBufferLink* next;
	u32 index_count; 				// This got me, it was u16 and an overflow caused a bug. Three hours; 
	u32 vertex_stride;
	u32 vertex_count;
}DeviceVertexBufferLink;

typedef struct{
	DeviceArena* device_arena;
	DeviceVertexBufferLink *newest, *oldest;
	DeviceVertexBufferLink *freed;

	u32 vertex_stride;
	u64 link_buffer_size;
	Arena* arena;
}DeviceVertexBuffer;


DeviceVertexBuffer create_vertex_buffer(DeviceArena* device_arena, u32 vertex_stride, u64 link_buffer_size, Arena* arena);
void destroy_vertex_buffer(DeviceVertexBuffer vb);
DeviceVertexBuffer* clear_vertex_buffer(DeviceVertexBuffer* vb);
void vertex_buffer_draw_indexed_implicit(DeviceVertexBuffer* vb, u16 vertex_count, const void* vertices);
void vertex_buffer_draw_indexed(DeviceVertexBuffer* vb, u32 index_count, const u16* indices, u16 vertex_count, const void* vertices);
void vertex_buffer_draw(DeviceVertexBuffer* vb, u16 vertex_count, const void* vertices);
void cmd_draw_vertex_buffer(DeviceCommandBuffer cb, const DeviceVertexBuffer* vb);




