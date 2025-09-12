// Skybox.frag.glsl
#version 450

layout (set = 0, binding = 1) uniform sampler       inSkyboxSampler;
layout (set = 0, binding = 2) uniform textureCube   inSkyboxImage;

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(samplerCube(inSkyboxImage, inSkyboxSampler), inUVW);
}