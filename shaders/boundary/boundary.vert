#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 model;
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
 
void main() {

    fragColor  = vec3(1.0, 1.0, 1.0);
    fragNormal = inNormal;
    fragUV     = inUV;

    gl_Position = camera.proj * camera.view * camera.model * vec4(inPos, 1.0);
}


