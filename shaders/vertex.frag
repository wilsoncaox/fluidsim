#version 450


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

vec3 jet(float t) {
    return clamp(vec3(
        1.5 - abs(4.0*t - 3.0),
        1.5 - abs(4.0*t - 2.0),
        1.5 - abs(4.0*t - 1.0)
    ), 0.0, 1.0);
}

void main() {
    float speed = length(fragColor);

    float minSpeed = 0.0;
    float maxSpeed = 5;

    float t = clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0, 1.0);
    vec3 particle_color = jet(t);


    vec3 N = normalize(fragNormal);

    vec3 L = normalize(vec3(0.4, 0.8, 0.2));

    float diffuse = max(dot(N, L), 0.0);

    float ambient = 0.2;

    vec3 color = particle_color * (ambient + diffuse);
    outColor = vec4(color, 1.0);

}

