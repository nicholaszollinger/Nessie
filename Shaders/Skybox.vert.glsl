// Skybox.vert.glsl
#version 450

layout (set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
    float exposureFactor;
} u_camera;

// Vertex Input
layout (location = 0) in vec3 inVertPosition;
layout (location = 1) in vec3 inVertNormal; 
layout (location = 2) in vec2 inUV;         
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

// Vertex Output.
layout (location = 0) out vec3 outUVW;

void main()
{
    // Our UV position is the same as the vertex position.
    outUVW = inVertPosition;
    
    // We are removing the 4th column from the viewMatrix because we don't want the translation 
    // from the view Matrix, but we do want the 4th column of the projection matrix to be applied
    vec4 position = u_camera.proj * mat4(mat3(u_camera.view)) * vec4(inVertPosition, 1.f);
    
    // Force the skybox at maximum depth
    // By setting z = w, after perspective division (z/w) we get z = 1.0
    gl_Position = position.xyww;
}