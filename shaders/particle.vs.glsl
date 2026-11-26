#version 330

// Input attributes
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 velocity;
layout(location = 2) in vec2 acc;
layout(location = 3) in float time;
layout(location = 4) in float life_time;

// Application data
uniform mat3 transform;
uniform mat3 projection;

out vec3 vcolor;

void main()
{
	float percent = time/life_time;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);

	vcolor = .5 * cos(percent * vec3(in_position, 0.7) * 50.0) + .5;
	gl_Position = vec4(pos, 1.0);
	gl_PointSize = 4.0;
}