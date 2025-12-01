#version 450
#pragma shader_stage(vertex)

layout(location = 0) in uint in_position_bits;
layout(location = 1) in uint in_velocity_bits;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_velocity;

#include "vert2.h"

const vec2 vertices[3] = {
	vec2(0.0, -1.0),
	vec2(-0.5, 1.0),
	vec2(0.5, 1.0)
};
const vec2 center = vec2(0.0, 0.25);

void main()
{


	uint px = bitfieldExtract(in_position_bits, 0, 16);
	uint py = bitfieldExtract(in_position_bits, 16, 16);

	vec2 position;
	position = vec2(px,py) / float(1<<16);
	position -= vec2(0.5);

	vec2 velocity;
	uint vx = bitfieldExtract(in_velocity_bits, 0, 16);
	uint vy = bitfieldExtract(in_velocity_bits, 16, 16);
	velocity = vec2(vx,vy) - 32768.0;
	if(abs(velocity.x) < 1.0)
	{
		velocity.x = 1.0;
	}
	if(abs(velocity.y) < 1.0)
	{
		velocity.y = 1.0;
	}
	vec2 u = normalize(velocity);

	u.y = -u.y;

	vec2 p = vertices[gl_VertexIndex % 3];

	p += center;
	p *= pc.scale * 0.001;

	p *= mat2(u.y, -u.x, u.x, u.y);
	
	p += position;
	p = transform_position(p);


	gl_Position = vec4(p, 1.0, 1.0);

	out_color = pc.color;
	if(false){
		float index = float(gl_InstanceIndex) / (32*1024);;
		out_color = vec4(index,  1.0 - index,0.0,  1.0);
	}
}
