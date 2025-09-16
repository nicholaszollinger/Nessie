// Skybox.frag.glsl
#version 450

layout (set = 0, binding = 0) uniform CameraUniforms
{
    mat4 proj;
    mat4 view;
} u_camera;

layout (set = 1, binding = 0) uniform sampler       inSkyboxSampler;
layout (set = 1, binding = 1) uniform textureCube   inSkyboxImage;

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(samplerCube(inSkyboxImage, inSkyboxSampler), inUVW);
}