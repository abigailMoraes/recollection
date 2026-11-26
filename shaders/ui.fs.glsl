#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec2 tiling;  // Added this for texture tiling

// Output color
layout(location = 0) out  vec4 color;

void main()
{
    // Multiply the texture coordinates by the tiling factor
    vec2 tiledTexcoord = texcoord * tiling;
    color = vec4(fcolor, 1.0) * texture(sampler0, texcoord * tiling);

}
