#include "boid.h"
#include <simde/x86/avx512.h>
#include "device_graphics.h"

void generate_cell_mask64()
{
	for(u32 i = 0; i < 64; i++)
	{
		u32 x = i & 7;
		u32 y = i >> 3;
		u32 r = x * x + y * y;
		u32 d  = (u32)(sqrt((f32)r)) + 1;
		print("%u32,", d);
	}
}

void boid_sim_next_frame(BoidSim *sim)
{
	sim->frame_index = (sim->frame_index + 1) % BOID_SIM_FRAME_COUNT;
	sim->next_frame_index = (sim->frame_index + 1) % BOID_SIM_FRAME_COUNT;
	sim->positions = sim->frame_positions[sim->frame_index];
	sim->velocities = sim->frame_velocities[sim->frame_index];
	sim->next_positions = sim->frame_positions[sim->next_frame_index];
	sim->next_velocities = sim->frame_velocities[sim->next_frame_index];
}

void* boid_sim_thread(Thread *thread);
BoidSim* create_boid_sim(Device *device, u32 max_boid_count, u32 max_thread_count, Arena *arena)
{
	max_thread_count = 32;
	max_boid_count = 1024 * 64;
	u32 boid_count_limit = 512 * 1024 * 1024 - 1024;
	max_boid_count = (max_boid_count & (~1023u));

	if(max_boid_count >= boid_count_limit)
	{
		print("I am very sorry, you may only allocate a maximum of (512*1024*1024-1024) (%u64) boids. The number (%u64) is (%u64) more than that. Please back off. Your GPU thanks you. FYI: The size of the contiguous GPU buffers will surpass a limit.\n", 
		// the limit is not x-1 but instead x-1024 because otherwise RenderDoc will not capture a frame. Probably because of buffer alignment.
			boid_count_limit, max_boid_count, max_boid_count - boid_count_limit);
		max_boid_count = boid_count_limit;
	}

	u32 max_thread_group_count = max_thread_count;
	BoidSim *sim = arena_alloc(sizeof(BoidSim), 0,0, arena);
	*sim = (BoidSim){
		.max_boid_count = max_boid_count,
		.boid_count = max_boid_count,
		.loop_time = loop_time_init(0),

		.cond = create_cond(arena),
		.mutex = create_mutex(arena),
		.threads = arena_alloc(sizeof(Thread*) * max_thread_count, 0,0, arena),
		.all_barriers = arena_alloc(sizeof(Barrier) * max_thread_count, 0,0, arena),
		.host_barrier = create_barrier(max_thread_count+1, arena),
		.host_barrier_for_two = create_barrier(2, arena),

		.thread_params = arena_alloc(sizeof(BoidSimParams) * max_thread_count, 0,0, arena),
		.thread_groups = arena_alloc(sizeof(BoidSimParams) * max_thread_group_count, 0,0, arena),
		.thread_count = max_thread_count,
		.max_thread_count = max_thread_count,
		.requested_thread_count = max_thread_count,
		.thread_group_count = max_thread_group_count,
		.should_run = true,
		.should_reset = true,
		.should_draw = false,
		.cells_width = 256,
		.cells_height = 256,
		.cells_width_rsh = 24,
		.cells_height_rsh = 24,
		.frame_index = 0,
	};
	sim->cells_count = sim->cells_width * sim->cells_height;

	for(u32 i = 0; i < max_thread_count; i++)
	{
		sim->all_barriers[i] = create_barrier(i+1, arena);			
		sim->thread_params[i] = (BoidSimParams){
			.global_index = i,	
			.sim = sim,
		};
	}

	sim->stage_params[BOID_SIM_STAGE_COUNT] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_COUNT,
		.task_size = 1024,		
		.task_max_count = sim->boid_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};

	sim->stage_params[BOID_SIM_STAGE_ALLOCATE] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_ALLOCATE,
		.task_size = 1024,		
		.task_max_count = sim->boid_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};
	sim->stage_params[BOID_SIM_STAGE_FILL] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_FILL,
		.task_size = 1024,		
		.task_max_count = sim->boid_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};
	sim->stage_params[BOID_SIM_STAGE_CONSTRUCT] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_CONSTRUCT,
		.task_size = 64,		
		.task_max_count = sim->cells_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};
	sim->stage_params[BOID_SIM_STAGE_RESOLVE] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_RESOLVE,
		.task_size = 8,		
		.task_max_count = sim->cells_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};
	sim->stage_params[BOID_SIM_STAGE_RESET] = (BoidSimStageParams) {
		.stage = BOID_SIM_STAGE_RESET,
		.task_size = 1024,		
		.task_max_count = sim->boid_count,
		.thread_count =  32,
		.group_size = 16,
		.group_count = 2,
	};

	{
		struct GlobalBoidParams *bp = &MAIN_THREAD->global.boid;

		bp->cohesion_range_norm = 0.5;
		bp->seperation_range_norm = 0.5;
		bp->alignment_range_norm = 1.0;

		bp->cohesion_strength_norm = 0.4;
		bp->seperation_strength_norm = 0.4;
		bp->alignment_strength_norm = 0.3;
		bp->boid_size_norm = 0.1;
	}

	for(u32 i = 0; i < BOID_SIM_FRAME_COUNT; i++)
	{
		u64 position_device_buffer_size = max_boid_count * (sizeof(uvec2));
		sim->position_device_buffers[i] = allocate_device_buffer(device->explicit_arena, position_device_buffer_size, DEVICE_MEMORY_TYPE_HOST_CACHED, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, arena);
		u64 velocity_device_buffer_size = max_boid_count * (sizeof(svec2));
		sim->velocity_device_buffers[i] = allocate_device_buffer(device->explicit_arena, velocity_device_buffer_size, DEVICE_MEMORY_TYPE_HOST_CACHED, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, arena);
		sim->frame_positions[i] = sim->position_device_buffers[i].memory.mapping;
		sim->frame_velocities[i] = sim->velocity_device_buffers[i].memory.mapping;
	}

	boid_sim_next_frame(sim);
	boid_sim_next_frame(sim);

	for(u32 i = 0; i < max_thread_count; i++)
	{
		sim->threads[i] = start_thread(boid_sim_thread, &sim->thread_params[i], 1024 * 1024 * 16, arena);
		set_thread_affinity(sim->threads[i], i);
	}
	
	for(u32 i = 0; i < max_thread_group_count; i++)
	{
		sim->thread_groups[i] = (ThreadGroup){
			.all_barriers = arena_alloc(sizeof(Barrier) * max_thread_count, 0,0,arena),
		};
		for(u32 j = 0; j < max_thread_count; j++)
		{
			sim->thread_groups[i].all_barriers[j] = create_barrier(j+1, arena);
		}
	}
// Stages
	{
		
		sim->cell_counters = arena_alloc(sim->cells_count * sizeof(u16), PAGE_SIZE, 0, arena);
		sim->cell_indices = arena_alloc(sim->max_boid_count * sizeof(CellIndex), PAGE_SIZE, 0, arena);
	}	
//
	{
		sim->cells = arena_alloc(sizeof(BoidSimCell) * sim->cells_count, PAGE_SIZE,0, arena);
	}
//

	u64 required_size = GB * 8;

	sim->arena = allocate_sub_arena(required_size, arena);

	barrier_wait(sim->host_barrier);

	return sim;
}

void destroy_boid_sim(BoidSim *sim)
{
	atomic_store(&sim->thread_count, sim->max_thread_count);
	atomic_store(&sim->should_run, false);
	reset_boid_sim(sim);

	mutex_lock(sim->mutex);
	cond_broadcast(sim->cond);
	mutex_unlock(sim->mutex);

	for(u32 i = 0; i < sim->max_thread_count; i++)
	{
		join_thread(sim->threads[i]);	
	}
	for(u32 i = 0; i < BOID_SIM_FRAME_COUNT; i++)
	{
		free_device_buffer(sim->position_device_buffers[i]);
		free_device_buffer(sim->velocity_device_buffers[i]);
	}
}

void reset_boid_sim(BoidSim *sim)
{
	atomic_store(&sim->should_reset, true);
}

void cmd_draw_boid_sim_boids(DeviceCommandBuffer cb, BoidSim *sim)
{
	VkBuffer buffers[] = {sim->position_device_buffers[sim->frame_index].handle, sim->velocity_device_buffers[sim->frame_index].handle};
	u64 offsets[] = {0, 0};
	vkCmdBindVertexBuffers(cb.handle, 0,2,buffers,offsets);
	u32 boid_count = atomic_load(&sim->boid_count);
	vkCmdDraw(cb.handle, 3, boid_count,0,0);
}

void draw_boid_sim_overlay(DeviceVertexBuffer *vb, Camera2 camera, SimpleFont simple_font, BoidSim *sim)
{
	f32 unit_pixel = camera.unit_pixel.y;
	fvec2 pos_a = camera.orig_top_left;
	fvec2 pos_b = camera.orig_bottom_right;
	fvec2 pos = pos_a;
	pos = fvec2_scalar_add(pos, unit_pixel * 2);

	Temp temp = begin_temp(0);

	fvec4 text_color = fvec4_make(0.1, 0.8, 0.1, 1.0);



	u8 *str = 0;
	BoidSimStageParams sp;

//	" Fill:     %u64 us \t* %u32 threads = %u64 us\n"
//	" Resolve:  %u64 us \t* %u32 threads = %u64 us\n"

	sp = sim->stage_params[BOID_SIM_STAGE_COUNT];
	str = (u8*)string_print(temp.arena, 
	"Update:     %u64 us\n"
	" Count:     %u64 us\n"
	,
	sim->elapsed_time / 1000, 
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	sp = sim->stage_params[BOID_SIM_STAGE_ALLOCATE];
	str = string_print(temp.arena,
	" Alloc:     %u64 us\n",
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	sp = sim->stage_params[BOID_SIM_STAGE_FILL];
	str = string_print(temp.arena,
	" Fill:      %u64 us\n",
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	sp = sim->stage_params[BOID_SIM_STAGE_CONSTRUCT];
	str = string_print(temp.arena,
	" Construct: %u64 us\n",
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	sp = sim->stage_params[BOID_SIM_STAGE_RESOLVE];
	str = string_print(temp.arena,
	" Resolve:   %u64 us\n",
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	sp = sim->stage_params[BOID_SIM_STAGE_RESET];
	str = string_print(temp.arena,
	" Reset:     %u64 us\n",
	sim->stage_times[sp.stage] / 1000
	).str;
	pos = gdraw_simple_text_box(vb, simple_font, str, text_color, pos_a, pos_b, pos, unit_pixel * 24);

	end_temp(temp);
}

void draw_boid_sim_grid(DeviceVertexBuffer *vb, Camera2 camera, SimpleFont simple_font, BoidSim *sim)
{
	atomic_store(&sim->should_draw, true);
	barrier_wait(sim->host_barrier_for_two);
	fvec2 a = fvec2_make(-0.5, -0.5);
	fvec2 b = fvec2_make(0.5, 0.5);
	for(u32 y = 0; y < sim->cells_height; y++)
	{
		for(u32 x = 0; x < sim->cells_width; x++)
		{
			fvec2 aa = fvec2_make(x,y);		
			fvec2 bb = fvec2_make(x+1,y+1);		
			aa = fvec2_scalar_div(aa, sim->cells_width);
			aa = fvec2_add(aa, a);
			bb = fvec2_scalar_div(bb, sim->cells_height);
			bb = fvec2_add(bb, a);
			aa = fvec2_scalar_add(aa, (bb.x - aa.x) / sim->cells_width);
			bb = fvec2_scalar_sub(bb, (bb.x - aa.x) / sim->cells_width);
			fvec2 avg_pos = aa;
			u16 count = sim->cell_counters[x + sim->cells_width * y];
			f32 cc = (f32)count / (f32)sim->cells_width;
			if(count == 0)
			{
			 	cc = 0.0;
			}
			gdraw_rectangle(vb, aa, bb, fvec4_make(cc+0.02, 0.02, 0.02, 0.1));
			BoidSimCell *cell = &sim->cells[x + sim->cells_width * y];
		}
	}
	atomic_store(&sim->should_draw, false);
	barrier_wait(sim->host_barrier_for_two);
}

typedef struct{
	u32 index;	
	u32 count;
	b32 has_work;
}Task;

ThreadGroup* enter_thread_group(BoidSimParams *p, b32 is_group_local_task)
{
	BoidSim* sim = p->sim;
	BoidSimStage stage = atomic_load(&sim->stage);

	u32 group_size = atomic_load(&sim->stage_params[stage].group_size);
	u32 group_index = p->global_index / group_size;
	u32 group_count = atomic_load(&sim->thread_count) / group_size;


	ThreadGroup *group = sim->thread_groups + group_index;

	if(barrier_wait(group->all_barriers[group_size-1]))
	{
		u32 task_size = atomic_load(&sim->stage_params[stage].task_size);
		u32 task_max_count = atomic_load(&sim->stage_params[stage].task_max_count);
		if(is_group_local_task)
		{
			group->task_count = task_max_count;
			group->task_index = 0;
		}
		else
		{
			group->task_count = task_max_count / group_count;
			group->task_index = group_index * group->task_count;
		}
		group->task_counter = 0;
		group->group_index = group_index;
		group->thread_count = group_size;
		group->group_count = group_count;
		group->all_barrier = group->all_barriers[group->thread_count-1];

	}
	p->local_index = p->global_index % group_size;
	barrier_wait(group->all_barriers[group_size-1]);
	return group;
}

Task reserve_boid_sim_task(BoidSimParams *p, ThreadGroup *group)
{
	BoidSim* sim = p->sim;
	Task task;
	task.has_work = false;
	BoidSimStage stage = atomic_load(&sim->stage);
	task.count = atomic_load(&sim->stage_params[stage].task_size);
	task.index = atomic_fetch_add(&group->task_counter, task.count);
	u32 max_index = atomic_load(&group->task_count);
	if(atomic_load(&sim->should_reset) == false)
	{
		task.has_work = (task.index < max_index);
	}
	if(task.count > max_index)
	{
		task.count = max_index - task.index;		
	}
	task.index += atomic_load(&group->task_index);
	return task;
}

void boid_sim_reset(BoidSimParams *p)
{
	Task task;
	uvec2 *pos=  p->sim->next_positions;
	svec2 *vel=  p->sim->next_velocities;
	PRNG rg = init_prng(p->global_index + p->sim->tick_accum + get_time_ns());

	ThreadGroup *thread_group = enter_thread_group(p, false);

	while((task = reserve_boid_sim_task(p, thread_group)).has_work)
	{
		if(1)
		{
			prng_memset(&rg, &pos[task.index], sizeof(uvec2) * task.count);
			prng_memset(&rg, &vel[task.index], sizeof(svec2) * task.count);
		}
		else
		{
			prng_memset(&rg, &vel[task.index], sizeof(svec2) * task.count);
			s64 r = U32_MAX / 5;
			for(u32 i = task.index; i < task.count + task.index; i++)
			{
				u32 x, y;
				s64 sx, sy;
				do{
					x = random_u32(&rg);	
					y = random_u32(&rg);	
					sx = S32_MAX - *(s32*)&x;
					sy = S32_MAX - *(s32*)&y;
				}while((sx * sx + sy * sy > r * r));
				pos[i] = uvec2_make(x,y);
				vel[i] = svec2_make(-sx,-sy);
			}
			
		}
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			const u32 rsh = 16;
			vel[i].x >>= rsh;
			vel[i].y >>= rsh;
		}
		continue;

		// Test for race conditions ... they should all be in a straight line.
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			pos[i].x = random_u32(&rg);
			pos[i].y = U32_MAX >> 1;
			vel[i].y = S32_MAX >> 1;
			vel[i].x = 0;
		}
	}
}

void boid_sim_count(BoidSimParams *p)
{
	ThreadGroup *tg = enter_thread_group(p, true);
	Task task;
	BoidSim *sim = p->sim;


	u32 cells_count = sim->cells_count / tg->group_count;
	u32 cells_start = cells_count * tg->group_index;
	u32 cells_end = cells_start + cells_count;

	{
		u32 cells_per_thread = cells_count / tg->thread_count;
		u32 pos = cells_per_thread * p->local_index + cells_start;
		memzero(sim->cell_counters + pos, cells_per_thread * sizeof(*sim->cell_counters));
	}

	barrier_wait(tg->all_barrier);

	uvec2 *pos=  sim->positions;
	while((task = reserve_boid_sim_task(p, tg)).has_work)
	{
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			u32 x = pos[i].x >> sim->cells_width_rsh;
			u32 y = pos[i].y >> sim->cells_height_rsh;
			u32 boid_cell_index = x + y * sim->cells_width;
			if((boid_cell_index >= cells_start) && (boid_cell_index < cells_end))
			{
				atomic u16 *cell  = sim->cell_counters + boid_cell_index;
				u16 current_count = atomic_fetch_add_explicit(cell, 1, memory_order_relaxed); // explicit does not help much
				sim->cell_indices[i].index = current_count;
				sim->cell_indices[i].cell = boid_cell_index;
			}
		}
	}
}

void boid_sim_allocate(BoidSimParams *p)
{
	BoidSim *sim = p->sim;
	const u64 fixed_size = sizeof(BoidSimCell);
	const u64 size_per_boid = sizeof(svec2) + sizeof(uvec2);
	arena_raw(&sim->arena, true);
	u8* pos = sim->arena.pos;
	pos = forward_align_pointer(pos, 64);
	u32 global_boid_offset = 0;
	for(u32 i = 0; i < sim->cells_count; i++)
	{
		BoidSimCell *cell = sim->cells + i;
		u32 local_boid_count = sim->cell_counters[i];

		cell->boid_count = local_boid_count;
		cell->global_boid_offset = global_boid_offset;
		global_boid_offset += local_boid_count;

		cell->positions = (fvec2*)pos;
		pos += sizeof(fvec2) * local_boid_count;
		cell->velocities = (fvec2*)pos;
		pos += sizeof(fvec2) * local_boid_count;
	}
	sim->arena.pos = pos;
	arena_raw(&sim->arena, false);
}

fvec2 boid_sim_uvec2_to_fvec2(uvec2 u)
{
	f64 x = (f64)u.x / (f64)U32_MAX;
	f64 y = (f64)u.y / (f64)U32_MAX;
	fvec2 f = fvec2_make(x,y);
	return f;
}
fvec2 boid_sim_svec2_to_fvec2(svec2 s)
{
	f64 x = (f64)s.x / (f64)S32_MAX;
	f64 y = (f64)s.y / (f64)S32_MAX;
	fvec2 f = fvec2_make(x,y);
	return f;
}
uvec2 boid_sim_fvec2_to_uvec2(fvec2 f)
{
	uvec2 u;
	u.x = (u32)((f64)f.x * (f64)((u64)U32_MAX));
	u.y = (u32)((f64)f.y * (f64)((u64)U32_MAX));
	return u;
}
svec2 boid_sim_fvec2_to_svec2(fvec2 f)
{
	svec2 s;
	s.x = (s32)((f64)f.x * (f64)S32_MAX);
	s.y = (s32)((f64)f.y * (f64)S32_MAX);
	return s;
}

void boid_sim_fill(BoidSimParams *p)
{
	ThreadGroup *tg = enter_thread_group(p, false);
	Task task;
	BoidSim *sim = p->sim;

	uvec2 *pos=  sim->positions;
	svec2 *vel=  sim->velocities;
	while((task = reserve_boid_sim_task(p, tg)).has_work)
	{
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			CellIndex *cell_index = sim->cell_indices + i;
			BoidSimCell *cell  = sim->cells + cell_index->cell;
			cell->positions[cell_index->index] = boid_sim_uvec2_to_fvec2(pos[i]);
			cell->velocities[cell_index->index] = boid_sim_svec2_to_fvec2(vel[i]);
		}
	}
}


void boid_sim_construct(BoidSimParams *p)
{
	ThreadGroup *tg= enter_thread_group(p, false);
	BoidSim *sim = p->sim;
	Task task;
	while((task = reserve_boid_sim_task(p, tg)).has_work)
	{
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			BoidSimCell *cell  = sim->cells + i;
			simde__m128 avg = simde_mm_set1_ps(0.0);

			for(u32 j = 0; j < cell->boid_count; j++)
			{
				simde__m128 vec = simde_mm_set_ps(
					cell->velocities[j].y,
					cell->velocities[j].x,
					cell->positions[j].y,
					cell->positions[j].x
				);
				avg = simde_mm_add_ps(avg, vec);
			}
			simde__m128 count = simde_mm_set1_ps((f32)cell->boid_count);
			avg = simde_mm_div_ps(avg, count);

			align(16) f32 dst[4];
			simde_mm_store_ps(dst, avg);
			memcpy(&cell->avg_pos, dst, 8);
			memcpy(&cell->avg_vel, dst+2, 8);

		}
	}
}
/*
0000000000011980 <boid_sim_search_average>:
   11980:	55                   	push   %rbp
   11981:	48 89 e5             	mov    %rsp,%rbp
   11984:	e8 67 67 02 00       	call   380f0 <mcount@plt> // -p
   1198d:	c5 f8 28 c1          	vmovaps %xmm1,%xmm0
   11991:	5d                   	pop    %rbp
   11992:	e9 29 4f 01 00       	jmp    268c0 <fvec2_make> // Jump because of profiling
   11997:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
   1199e:	00 00 

0000000000011980 <boid_sim_search_average>:
   11980:	55                   	push   %rbp
   11981:	48 89 e5             	mov    %rsp,%rbp
   11984:	e8 27 67 02 00       	call   380b0 <mcount@plt> // Profiler is not free
   11989:	c5 f9 ef c0          	vpxor  %xmm0,%xmm0,%xmm0
   1198d:	5d                   	pop    %rbp
   1198e:	c3                   	ret
   1198f:	90                   	nop
*/

u32 boid_sim_search_average(BoidSimParams *p, uvec2 upos, fvec2 orig_pos, fvec2 orig_vel, u32 range, fvec2 *restrict out_pos, fvec2 *restrict out_vel)
{
	BoidSim *sim = p->sim;

	u32 mask = sim->cells_width-1;
	u32 cell_rsh = sim->cells_width_rsh;

	u8 uy0 = (upos.y-range) >> cell_rsh;
	u8 uy1 = (upos.y+range) >> cell_rsh;
	uy1 = (uy1+1);

	u8 ux0 = (upos.x-range) >> cell_rsh;
	u8 ux1 = (upos.x+range) >> cell_rsh;
	ux1 = (ux1+1);

	u32 count = 0;

	f32 r = (f64)range / (f64)U32_MAX;

	if(out_pos)
	{
		*out_pos = fvec2_make(0.0, 0.0);
	}
	if(out_vel)
	{
		*out_vel = fvec2_make(0.0, 0.0);
	}


	f32 cell_size = 1.0f / (f32)sim->cells_width;

	for(u8 uy = uy0; uy != uy1; uy = (uy+1))
	{
		for(u8 ux = ux0; ux != ux1; ux = (ux+1))
		{
			BoidSimCell *cell = &sim->cells[ux + (uy * sim->cells_width)];
			uvec2 upos = uvec2_make(ux<<cell_rsh, uy<<cell_rsh);
			fvec2 cell_pos = boid_sim_uvec2_to_fvec2(upos);

			u32 corner_count = 0;

			if(corner_count == 4 && 0)
			{
				if(out_vel)
				{
					*out_vel = fvec2_add(*out_vel, cell->avg_vel);
				}
				if(out_pos)
				{
					*out_pos = fvec2_add(*out_pos, cell->avg_pos);
				}
				count++;
			}
			else if(1)
			{
				for(u32 i = 0; i < cell->boid_count; i++)
				{
					if(fvec2_distance(orig_pos, cell->positions[i]) < r)
					{
						if(out_vel)
						{
							*out_vel = fvec2_add(*out_vel, cell->velocities[i]);
						}
						if(out_pos)
						{
							*out_pos = fvec2_add(*out_pos, cell->positions[i]);
						}
						count++;
					}
				}
			}
		}
	}
	if(count)
	{
		if(out_vel)
		{
			*out_vel = fvec2_scalar_div(*out_vel, (f32)count);
		}
		if(out_pos)
		{
			*out_pos = fvec2_scalar_div(*out_pos, (f32)count);
		}
		return count;
	}
	return 0;

}

void boid_sim_resolve(BoidSimParams *p)
{
	ThreadGroup *tg= enter_thread_group(p, false);
	BoidSim *sim = p->sim;
	Task task;

	
	f32 pw = 3.0;
	f32 seperation_strength = powf(sim->global.alignment_strength_norm, pw);
	f32 cohesion_strength = powf(sim->global.cohesion_strength_norm, pw);
	f32 alignment_strength = powf(sim->global.alignment_strength_norm, pw);
	
	u32 seperation_range = (u32)(powf(sim->global.seperation_range_norm,pw) * (f32)(1<<24));
	u32 cohesion_range = (u32)(powf(sim->global.cohesion_range_norm,pw) * (f32)(1<<24));
	u32 alignment_range = (u32)(powf(sim->global.alignment_range_norm,pw) * (f32)(1<<24));

	f32 min_speed = powf(sim->global.min_speed_norm,pw) * 0.01;
	f32 max_speed = powf(sim->global.max_speed_norm,pw) * 0.01;
	f32 acceleration = powf(sim->global.acceleration_norm,pw) * 0.1;


	PRNG rg = init_prng(get_time_ms());

	while((task = reserve_boid_sim_task(p, tg)).has_work)
	{
		for(u32 i = task.index; i < task.count + task.index; i++)
		{
			BoidSimCell *orig_cell  = sim->cells + i;
			for(u32 j = 0; j < orig_cell->boid_count; j++)
			{
			// Read
				fvec2 pos = orig_cell->positions[j];
				fvec2 vel = orig_cell->velocities[j];
				uvec2 upos = boid_sim_fvec2_to_uvec2(pos);
				svec2 svel = boid_sim_fvec2_to_svec2(vel);

				if(1)
				{
					fvec2 average;
					u32 count = boid_sim_search_average(p, upos, pos, vel, seperation_range, &average, 0);
					if(count)
					{
						average = fvec2_sub(pos, average);
						average = fvec2_scalar_mul(average, seperation_strength);
						vel = fvec2_add(vel, average);
					}
				}
				if(1)
				{
					fvec2 average;
					u32 count = boid_sim_search_average(p, upos, pos, vel, cohesion_range, &average, 0);
					if(count)
					{
						average = fvec2_sub(pos, average);
						average = fvec2_scalar_mul(average, cohesion_strength);
						vel = fvec2_sub(vel, average);
					}
				}
				if(1)
				{
					fvec2 average;
					u32 count = boid_sim_search_average(p, upos, pos, vel, alignment_range, 0, &average);
					if(count)
					{
						vel = fvec2_lerp(vel, average, alignment_strength);
					}
				}

				f32 speed = fvec2_magnitude(vel);
				if(speed > max_speed)
				{
					vel = fvec2_scalar_mul(vel, 1.0-acceleration);
				}
				if(speed < min_speed)
				{
					vel = fvec2_scalar_mul(vel, 1.0+acceleration);
				}
				


				svel = boid_sim_fvec2_to_svec2(vel);

			// Update
				{
					upos.x += svel.x;
					upos.y += svel.y;
				}

			// Write
				u32 o = orig_cell->global_boid_offset + j;
				sim->next_positions[o] = upos;
				sim->next_velocities[o] = svel;
			}
		}
	}
}

void* boid_sim_thread(Thread *thread)
{
	BoidSimParams* p = thread->parameters;
	BoidSim* sim = p->sim;

	barrier_wait(sim->host_barrier);

	while(true)
	{

		b32 is_first_thread = barrier_wait(sim->all_barriers[atomic_load(&sim->thread_count)-1]);
		if(is_first_thread)
		{
			BoidSimStage last_stage = atomic_load(&sim->stage);
			BoidSimStage stage = last_stage + 1;
			b32 should_reset = false;
			if((should_reset = atomic_load(&sim->should_reset)))
			{
				stage = BOID_SIM_STAGE_RESET;
				atomic_store(&sim->should_reset, false);
			}
			if(stage == BOID_SIM_STAGE_RESET)
			{
				if(should_reset == false)
				{
					stage++;
				}
				else
				{
					atomic_store(&sim->tick_accum, 0);
				}
			}
			if(atomic_load(&sim->should_run) == false)
			{
				stage = BOID_SIM_STAGE_MAX;	
			}
			else if(stage == BOID_SIM_STAGE_MAX)
			{
				reset_arena(&sim->arena);
				stage = (BoidSimStage)0;	
				boid_sim_next_frame(sim);
				atomic_fetch_add(&sim->tick_accum, 1);
			}
			{
				u64 time = get_time_ns();
				sim->end_stage_time = time;
				sim->elapsed_stage_time = sim->end_stage_time - sim->start_stage_time;
				sim->start_stage_time = time;
				sim->stage_times[last_stage] = sim->elapsed_stage_time;
			}

			if(stage == 0)
			{
				u64 time = get_time_ns();
				sim->end_time = time;
				sim->elapsed_time = sim->end_time - sim->start_time;
				sim->start_time = time;
			}
			atomic_store(&sim->stage, stage);
			atomic_store(&sim->thread_count, atomic_load(&sim->requested_thread_count));
			if(atomic_load(&sim->should_draw) == true)
			{
				barrier_wait(sim->host_barrier_for_two);
				barrier_wait(sim->host_barrier_for_two);
			}
		}
		while(atomic_load(&sim->thread_count) <= p->global_index)
		{
			mutex_lock(sim->mutex);
			cond_wait(sim->cond, sim->mutex);
			mutex_unlock(sim->mutex);
		}

		sim->global = MAIN_THREAD->global.boid;

		is_first_thread = barrier_wait(sim->all_barriers[atomic_load(&sim->thread_count)-1]);


		switch(sim->stage)
		{
			case BOID_SIM_STAGE_COUNT:
				boid_sim_count(p);
			break;
			case BOID_SIM_STAGE_ALLOCATE:
				if(p->global_index == 0)
				{
					boid_sim_allocate(p);
				}
			break;
			case BOID_SIM_STAGE_FILL:
				boid_sim_fill(p);
			break;
			case BOID_SIM_STAGE_CONSTRUCT:
				boid_sim_construct(p);
			break;
			case BOID_SIM_STAGE_RESOLVE:
				boid_sim_resolve(p);
			break;
			case BOID_SIM_STAGE_RESET:
				boid_sim_reset(p);
			break;
			case BOID_SIM_STAGE_MAX:
				goto END;
			break;
		}
	}
END:
	return NULL;
}
