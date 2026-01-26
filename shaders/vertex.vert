#version 450


struct ParticleData {
    vec4 position;
    vec4 velocity;
    vec4 density;
    vec4 predicted_position;
};

layout(binding = 0) uniform Camera {
    mat4 model;
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 1) buffer Read {
    ParticleData[] particles;
} read;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
 
void main() {

    ParticleData instance_data = read.particles[gl_InstanceIndex];

    fragColor  = instance_data.velocity.xyz;
    fragNormal = inNormal;
    fragUV     = inUV;

    vec3 pos = instance_data.position.xyz;
        
    gl_Position = camera.proj * camera.view * camera.model * vec4(pos + inPos, 1.0);
}


