#version 450

layout (set = 0, binding = 0) uniform CascadedShadowMapsUBO
{
    mat4    cascadeViewProjMatrices[4];
    vec4    splitDepths;
    uint    numCascades;
    float   shadowBias;
} u_shadow;

layout (push_constant) uniform Push
{
    mat4 modelMatrix;
    uint cascadeIndex;
} u_push;

// Vertex Data:
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 pos = u_push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = u_shadow.cascadeViewProjMatrices[u_push.cascadeIndex] * pos;
}
