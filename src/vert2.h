

layout(push_constant) uniform PushConstant{
	mat3 affine;
	vec4 color;
	uint time;
	uint index;
	float scale;
}pc;

vec2 transform_position(vec2 v)
{
	vec2 pos = v;
	pos = (vec3(v, 1.0) * pc.affine).xy;
	return pos.xy;
}
