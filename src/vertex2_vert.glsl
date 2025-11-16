#version 450
#pragma shader_stage(vertex)


layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_position;
layout(location = 2) in uint in_texture;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_texture;
layout(location = 2) out uint out_texture_index;
layout(location = 3) out uint out_index;

#include "vert2.h"

void main()
{

	out_texture = vec2(
		float(bitfieldExtract(in_texture, 0, 15)),	
		float(bitfieldExtract(in_texture, 15, 15))
	);

	out_texture_index = bitfieldExtract(in_texture, 30, 2);


	vec2 p = transform_position(in_position);
	gl_Position = vec4(p, 1.0, 1.0);
	out_color = in_color;
	out_index = pc.index;
	
}
