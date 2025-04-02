// Skybox.frag.glsl
#version 450

layout (binding = 0) uniform CameraUniforms
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout (set = 1, binding = 3) uniform samplerCube inSkyboxTexture;

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(inSkyboxTexture, inUVW);
}