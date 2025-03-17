// 3D Geometry Shader.
#version 450

struct MeshMaterial
{
    vec4 baseColor;
};

layout (binding = 0) uniform CameraUniforms
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout (push_constant) uniform GeometryConstants
{
    mat4 objectMatrix;
    MeshMaterial material; // Only a push constant because it just contains a color.
};

layout (location = 0) in vec3 inVertPosition;
layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = material.baseColor;
    gl_Position = projectionMatrix * viewMatrix * objectMatrix * vec4(inVertPosition, 1.0);
}