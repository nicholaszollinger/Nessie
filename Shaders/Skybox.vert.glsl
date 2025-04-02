// Skybox.vert.glsl
#version 450

layout (binding = 0) uniform CameraUniforms
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout (location = 0) in vec3 inVertPosition;
layout (location = 0) out vec3 outUVW;

void main()
{
    // Our UV position is the same as the vertex position.
    outUVW = inVertPosition;
    
    // We are removing the 4th column from the viewMatrix because we don't want the translation 
    // from the view Matrix, but we do want the 4th column of the projection matrix to be applied
    gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) * vec4(inVertPosition, 1.f);
}