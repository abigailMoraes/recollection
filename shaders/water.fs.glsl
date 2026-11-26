#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float screen_darken_factor;
uniform float intensity = 4.;
uniform float spread = .4;

// Sepia filter from FFMPEG
uniform mat3 sepia = mat3(.393, .349, .272, .769, .686, .534, .189, .168, .131);

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	if (screen_darken_factor > 0) {
		float progress = min(screen_darken_factor * 1.5, 1.); // cap it at 1
		mat3 sepia_mat = mat3(1.) - (mat3(1.) - sepia) * progress;
		in_color = vec4(sepia_mat * in_color.rgb, in_color.a);
	}
	return in_color;
}

vec4 fade_color(vec4 in_color, vec2 uv)
{
	uv *= 2 * (1.0 - uv.xy); // map to [1., 1.], [1., 1.]

	float vig = dot(uv, uv.yx) * intensity; // vignette effect strength
	vig = pow(vig, spread) + 1./intensity; // spread (curvature) control
	in_color *= vig;
	return in_color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color, coord);
}