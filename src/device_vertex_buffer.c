#include "device_vertex_buffer.h"
#include "device_geometry.h"


DeviceVertexBufferLink* allocate_vertex_buffer_link(DeviceArena* device_arena, u64 size, u32 vertex_stride, Arena* arena)
{
	DeviceBuffer buffer = allocate_device_buffer(
		device_arena, size,
		DEVICE_MEMORY_TYPE_HOST_CACHED,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		arena
	);
	if(buffer.memory.mapping == 0)
	{
		print("mapping failed\n");

	}
	DeviceVertexBufferLink* link = arena_alloc(sizeof(DeviceVertexBufferLink),0,0, arena);
	*link = (DeviceVertexBufferLink){
		.buffer = buffer,
		.vertex_stride = vertex_stride,
	};
	return link;
}

DeviceVertexBuffer create_vertex_buffer(DeviceArena* device_arena, u32 vertex_stride, u64 link_buffer_size, Arena* arena)
{
	DeviceVertexBufferLink* link = allocate_vertex_buffer_link(device_arena, link_buffer_size, vertex_stride, arena);
	DeviceVertexBuffer vb = {
		.device_arena = device_arena,			
		.newest = link,
		.oldest = link,
		.arena = arena,
		.vertex_stride = vertex_stride,
		.link_buffer_size = link_buffer_size,
	};
	return vb;
}
void destroy_vertex_buffer(DeviceVertexBuffer vb)
{
	clear_vertex_buffer(&vb);
	DeviceVertexBufferLink* link = vb.freed;
	while(link)
	{
		free_device_buffer(link->buffer);
		link = link->next;
	}
	free_device_buffer(vb.oldest->buffer);
}

DeviceVertexBuffer* clear_vertex_buffer(DeviceVertexBuffer* vb)
{
	
	DeviceVertexBufferLink* link = vb->oldest;
	while(link)
	{
		link->index_count = 0; 
		link->vertex_count = 0;
		link = link->next;
	}
	if(vb->freed)
	{
		vb->newest->next = vb->freed;
	}
		vb->freed = vb->oldest;
		vb->oldest = vb->freed;
		vb->newest = vb->freed;
		vb->freed = vb->freed->next;
	return vb;
}

b32 vertex_buffer_link_draw_indexed(DeviceVertexBufferLink* link, u32 index_count, const u16* indices, u16 vertex_count, const void* vertices)
{

	u64 required_size = (link->index_count + vertex_count) * sizeof(u16) + (link->vertex_count + index_count) * link->vertex_stride;
	if(required_size >= link->buffer.size)
	{
		return true;
	}

	u8* buffer_start = link->buffer.memory.mapping;
	u8* buffer_end = buffer_start + link->buffer.size;
	u32 vertex_stride = link->vertex_stride;

	u16* index_start = (u16*)buffer_start + link->index_count;
	u8* vertex_start = buffer_end - ((link->vertex_count + vertex_count) * vertex_stride);


	for(u32 i = 0; i < index_count; i++)
	{
		index_start[i] = indices[i] + (U16_MAX - link->vertex_count - vertex_count) + 1;
//		print("%u16 ", index_start[i]);
	}
//	print("\n");


	for(u32 i = 0; i < vertex_count; i++)
	{
		memcpy(vertex_start + (i* vertex_stride), vertices + (i * vertex_stride), vertex_stride);
	}

	link->index_count += index_count;
	link->vertex_count += vertex_count;

	return false;		
}

void vertex_buffer_draw_indexed(DeviceVertexBuffer* vb, u32 index_count, const u16* indices, u16 vertex_count, const void* vertices)
{
	while(vertex_buffer_link_draw_indexed(vb->newest, index_count, indices, vertex_count, vertices))
	{
		u64 required_size = vertex_count * vb->vertex_stride + index_count * sizeof(u16);
		required_size = defmax(required_size, vb->link_buffer_size);
		DeviceVertexBufferLink* link = 0;
		if(vb->freed)
		{
			if(vb->freed->buffer.size >= required_size)
			{
				link = vb->freed;
				vb->freed = vb->freed->next;
			}
			else
			{
			}
		}
		if(link == 0)
		{
			link = allocate_vertex_buffer_link(vb->device_arena, required_size, vb->vertex_stride, vb->arena);
		}
		else
		{
		}
		vb->newest->next = link;
		vb->newest = link;
	}

}

b32 vertex_buffer_link_draw(DeviceVertexBufferLink* link, u16 vertex_count, const void* vertices)
{
	u32 index_count = vertex_count;
	u64 required_size = (link->index_count + vertex_count) * sizeof(u16) + (link->vertex_count + index_count) * link->vertex_stride;
	if(required_size >= link->buffer.size)
	{
		return true;
	}


	u8* buffer_start = link->buffer.memory.mapping;
	u8* buffer_end = buffer_start + link->buffer.size;
	u32 vertex_stride = link->vertex_stride;

	u16* index_start = (u16*)buffer_start + link->index_count;
	u8* vertex_start = buffer_end - ((link->vertex_count + vertex_count) * vertex_stride);


	for(u32 i = 0; i < index_count; i++)
	{
		index_start[i] = i + (U16_MAX - link->vertex_count - vertex_count) + 1;
//		print("%u16 ", index_start[i]);
	}
//	print("\n");


	for(u32 i = 0; i < vertex_count; i++)
	{
		memcpy(vertex_start + (i* vertex_stride), vertices + (i * vertex_stride), vertex_stride);
	}

	link->index_count += index_count;
	link->vertex_count += vertex_count;

	return false;		
}

void vertex_buffer_draw(DeviceVertexBuffer* vb, u16 vertex_count, const void* vertices)
{
	while(vertex_buffer_link_draw(vb->newest, vertex_count, vertices))
	{
		u64 required_size = vertex_count * vb->vertex_stride + vertex_count * sizeof(u16);
		required_size = defmax(required_size, vb->link_buffer_size);
		DeviceVertexBufferLink* link = 0;
		if(vb->freed)
		{
			if(vb->freed->buffer.size >= required_size)
			{
				link = vb->freed;
				vb->freed = vb->freed->next;
			}
			else
			{
			}
		}
		if(link == 0)
		{
			link = allocate_vertex_buffer_link(vb->device_arena, required_size, vb->vertex_stride, vb->arena);
		}
		else
		{
		}
		vb->newest->next = link;
		vb->newest = link;
	}
}

void cmd_draw_vertex_buffer_link(DeviceCommandBuffer cb, DeviceVertexBufferLink* link)
{
	if(link->vertex_count)
	{
		u64 offset = (link->buffer.size) - (U16_MAX * link->vertex_stride);


		vkCmdBindVertexBuffers(cb.handle, 0, 1, &link->buffer.handle, &offset);
		vkCmdBindIndexBuffer(cb.handle, link->buffer.handle, 0, VK_INDEX_TYPE_UINT16);
		s32 vertex_offset = -1;
		vkCmdDrawIndexed(cb.handle, link->index_count, 1, 0, vertex_offset, 0);
	}
}

void cmd_draw_vertex_buffer(DeviceCommandBuffer cb, const DeviceVertexBuffer* vb)
{
	DeviceVertexBufferLink* link = vb->oldest;
	while(link)
	{
		cmd_draw_vertex_buffer_link(cb, link);
		link = link->next;
	}
}
