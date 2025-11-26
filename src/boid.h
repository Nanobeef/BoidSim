#pragma once

#include "device_renderer.h"

typedef struct{
	u64 x,y;
}u64vec2;

typedef struct{
	s64 x,y;
}s64vec2;

typedef struct{
	s16 x,y;
}s16vec2;

typedef struct{
	u16 cell;
	u16 index;
}CellIndex;

typedef struct{
	fvec2 *positions;
	fvec2 *velocities;
	u64 boid_count;
	u64 padding; // Do not remove
}BoidSimCell;

typedef struct{
	u32 thread_count;
	u32 task_index;
	u32 task_count;
	atomic u32 task_counter;

	u32 group_index;
	u32 group_count;

	Barrier *all_barriers;
	Barrier all_barrier;
}ThreadGroup;

typedef enum{
	BOID_SIM_STAGE_ALLOCATE,
	BOID_SIM_STAGE_FILL,
	BOID_SIM_STAGE_RESOLVE,
	BOID_SIM_STAGE_RESET,
	BOID_SIM_STAGE_MAX,
}BoidSimStageFlags;
typedef u32 BoidSimStage;

typedef struct{
	BoidSimStage stage;
	u32 task_size;
	u32 task_max_count;
	u32 thread_count;
	u32 group_size;
	u32 group_count;
}BoidSimStageParams;


union BoidSimParams;

#define BOID_SIM_FRAME_COUNT 2

typedef struct{


	BoidSimStageParams stage_params[BOID_SIM_STAGE_MAX];

	atomic u64 tick_accum;
	atomic b32 should_run;
	atomic b32 should_reset;
	atomic b32 should_draw;

	atomic u32 boid_count;
	atomic u32 draw_count;
	atomic u32 thread_count;
	atomic u32 boid_sim_mod;
	atomic u32 thread_group_count;
	atomic u32 requested_thread_count;
	atomic BoidSimStage stage;

	struct GlobalBoidParams global;

	Mutex mutex;
	Cond cond;
	Barrier host_barrier;
	Barrier host_barrier_for_two;

	u32 frame_index;
	u32 next_frame_index;
	u32 max_boid_count;	
	u32 max_thread_count;
	u32 max_thread_group_count;
	DeviceBuffer position_device_buffers[BOID_SIM_FRAME_COUNT];
	DeviceBuffer velocity_device_buffers[BOID_SIM_FRAME_COUNT];

	fvec2* frame_positions[BOID_SIM_FRAME_COUNT];
	fvec2* frame_velocities[BOID_SIM_FRAME_COUNT];

	fvec2* positions;
	fvec2* velocities;

	fvec2* next_positions;
	fvec2* next_velocities;

	LoopTime loop_time;
	u64 start_time, end_time;
	u64 elapsed_time;

	u64 start_stage_time, end_stage_time;
	u64 elapsed_stage_time;

	u64 stage_times[BOID_SIM_STAGE_MAX];

	Arena arena;

	Thread **threads;
	Barrier *all_barriers; // max_thread_count
	union BoidSimParams *thread_params;
	ThreadGroup *thread_groups;

// Stages
	u32 cells_count;
	u32 cells_width;
	u32 cells_height;
	u32 cells_width_rsh;
	u32 cells_height_rsh;
//
	u16 atomic *cell_counters;
	CellIndex *cell_indices;
//
	BoidSimCell *cells;
//

}BoidSim;

typedef union BoidSimParams{
	struct{
		BoidSim *sim;
		u32 global_index;
		u32 local_index;
		PRNG prng;
	};
}BoidSimParams align(64);

void boid_sim_resolve_test();
BoidSim* create_boid_sim(Device *device, u32 max_boid_count, u32 max_thread_count, Arena *arena);
void destroy_boid_sim(BoidSim *sim);
void reset_boid_sim(BoidSim *sim);
void cmd_draw_boid_sim_boids(DeviceCommandBuffer cb, BoidSim *sim);
void draw_boid_sim_overlay(DeviceVertexBuffer *vb, Camera2 camera, SimpleFont simple_font, BoidSim *sim);
void draw_boid_sim_grid(DeviceVertexBuffer *vb, Camera2 camera, SimpleFont simple_font, BoidSim *sim);

fvec2 boid_sim_uvec2_to_fvec2(uvec2 u);
fvec2 boid_sim_svec2_to_fvec2(svec2 s);
uvec2 boid_sim_fvec2_to_uvec2(fvec2 f);
svec2 boid_sim_fvec2_to_svec2(fvec2 f);

typedef struct{
	u32 index;	
	u32 count;
	b32 has_work;
}Task;

ThreadGroup* enter_thread_group(BoidSimParams *p, b32 is_group_local_task);
Task reserve_boid_sim_task(BoidSimParams *p, ThreadGroup *group);

/*

======== Boids ==========

Each boid fllows 3 rules.

1.) Seperation:
	Try to keep some distance from other boids. Personal Space.
2.) Alignment:
	Fly in the same direction as other boids.
3.) Choesion
	Stay close to other boids. Flock together.

These 3 rules make the a flock of boids flock like real birds.

Changing the strength and the effective distance of each rule can cause different emergent patterns.

*/


