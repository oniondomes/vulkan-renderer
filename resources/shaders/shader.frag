#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform LightInfo {
    vec3 reversedDirection;
} light;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 _normal = normalize(normal);
    float nDotL = max(dot(light.reversedDirection, _normal), 0.0);
    outColor = (vec4(vec3(1.0, 1.0, 1.0) * vec3(1.0, 1.0, 1.0) * nDotL, 1.0) + 0.02) * texture(texSampler, fragTexCoord) ;
}
