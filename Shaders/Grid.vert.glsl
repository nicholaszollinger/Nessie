#version 450

// A---------D
// |         |
// |    +    |
// |         |
// B---------C

vec2 positions[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0)
);

int indices[6] = int[](
    0, 1, 2,
    3, 0, 2
);

layout (set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
    float exposureFactor;
} u_camera;

layout(location = 0) out vec3 nearPoint;
layout(location = 1) out vec3 farPoint;

vec3 UnprojectPoint(vec3 point, mat4 viewMatrix, mat4 projMatrix)
{
    vec4 unprojected = vec4(inverse(viewMatrix) * inverse(projMatrix) * vec4(point.xyz, 1));
    return unprojected.xyz / unprojected.w;
}

void main() 
{
    vec2 position = vec2(positions[indices[gl_VertexIndex]]);
    nearPoint = UnprojectPoint(vec3(position, 0), u_camera.view, u_camera.proj);
    farPoint = UnprojectPoint(vec3(position, 1), u_camera.view, u_camera.proj);
    gl_Position = vec4(position, 0.5, 1);
}