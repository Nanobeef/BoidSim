#version 450
#pragma shader_stage(fragment)

layout(binding = 0) uniform sampler2D texture_image;
layout(binding = 1) uniform sampler2D glyph_image;

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_texture;
layout(location = 2) in flat uint in_texture_index;
layout(location = 3) in flat uint in_index;




layout(location = 0) out vec4 out_color;

// Quick and dirty from the internet.
float rand(vec2 co){
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec4 color = in_color;
	if(in_index == 1)
	{
		
		out_color = vec4(
			rand(vec2(gl_PrimitiveID, 1)), // gl_PrimitiveID in a fragment shader requires geometry shaders to be supported.
			rand(vec2(gl_PrimitiveID, 2)),
			rand(vec2(gl_PrimitiveID, 3)),
			1.0
		) * 0.5 + color * 0.5;
		return;
	}
	else if(in_index == 2)
	{
		float c = color.r + color.g + color.b;
		color = vec4(vec3(c/3.0), 1.0);
	}


	if(in_texture_index == 0)
	{
		out_color = texture(texture_image, in_texture / textureSize(texture_image, 0)) * in_color;
	}
	else if(in_texture_index == 1)
	{
		float t = texture(glyph_image, in_texture / textureSize(glyph_image, 0)).r;
		out_color = vec4(t) * in_color;
	}
	else if(in_texture_index == 2)
	{
		out_color = color;
	}
	else if(in_texture_index == 3)
	{
		out_color = color;
	}
	else
	{
		out_color = vec4(1.0, 0.1, 1.0, 1.0);
	}
}

