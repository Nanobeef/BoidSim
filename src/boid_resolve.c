#include "boid.h"
#include <simde/x86/avx512.h>
#include "device_graphics.h"

u32 boid_sim_search_average2(BoidSimParams *p, uvec2 upos, fvec2 orig_pos, fvec2 orig_vel, u32 range, fvec2 *restrict out_pos, fvec2 *restrict out_vel)
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

void boid_sim_simd_resolve(BoidSimParams *p)
{
	ThreadGroup *tg= enter_thread_group(p, false);
	BoidSim *sim = p->sim;
	Task task;

	
	f32 pw = 2;
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
					u32 count = boid_sim_search_average2(p, upos, pos, vel, seperation_range, &average, 0);
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
					u32 count = boid_sim_search_average2(p, upos, pos, vel, cohesion_range, &average, 0);
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
					u32 count = boid_sim_search_average2(p, upos, pos, vel, alignment_range, 0, &average);
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





