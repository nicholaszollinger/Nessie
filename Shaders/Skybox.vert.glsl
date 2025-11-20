// Skybox.vert.glsl
#version 450

// Hardcoded fullscreen triangle (covers NDC space)
// Using a single large triangle instead of quad (avoids diagonal seam)
const vec2 vertices[3] = vec2[3]
(
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

layout (set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
    float exposureFactor;
} u_camera;

// Vertex Output.
layout (location = 0) out vec3 outUVW;

void main()
{
    vec2 pos = vertices[gl_VertexIndex];

    // Unproject to get view direction
    mat4 invProj = inverse(u_camera.proj);
    vec3 unprojected = (invProj * vec4(pos, 1.0, 1.0)).xyz;

    // Transform to world space
    mat3 invView = transpose(mat3(u_camera.view));
    outUVW = invView * unprojected;

    // Output fullscreen position
    gl_Position = vec4(pos, 1.0, 1.0);
}