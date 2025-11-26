#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_velocity;

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
	vec2 velocity = in_velocity;

	vec2 position = in_position;
	position -= vec2(0.5);

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
