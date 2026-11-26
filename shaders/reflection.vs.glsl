#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;

// Time
uniform float time; // Time in seconds

void main()
{
    texcoord = in_texcoord;

    // Modify the transformation matrix to include a 180-degree rotation around the x-axis
    mat3 rotationMatrix = mat3(
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, -1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    // Apply a sine wave distortion to the x-coordinate based on time
    float distortionAmount = 3.0; // Adjust this parameter for the strength of distortion
    float waveFrequency = 20.0;    // Adjust this parameter for the frequency of waves

    float xDistortion = pow(cos(in_position.y * waveFrequency + time) * distortionAmount, 3.0);

    // Combine rotation and distortion in the y-direction
    vec3 pos = projection * rotationMatrix * transform * vec3(in_position.xy, 1.0) + vec3(xDistortion, 0.0 , 0.0);

    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
