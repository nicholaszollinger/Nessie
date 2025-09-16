#version 450

const float kPi = 3.14159265358979323846;
const float kPiOverOne = 1.0 / kPi;
const float kTwoPi = 2.0 * kPi;
const float kEpsilon = 0.00001;

//----------------------------
// Set 0: Camera Data
//----------------------------
layout (set = 0, binding = 0) uniform CameraUniforms
{
    mat4 proj;
    mat4 view;
} u_camera;

// Info sent the Fragment shader.
struct VertexOutput
{
    mat3 worldTransform; // Rotation & Scale Transform.
    mat3 worldNormals;
    vec3 worldPosition;  // World Position.
    vec3 normal;         // Vertex Normal
    vec2 texCoord;       // Vertex Texture coordinate, or UV.
    vec3 tangent;
    vec3 bitangent;
};

layout (location = 0) in VertexOutput inVertexData;
layout (location = 0) out vec4 outColor;

void main()
{
    // Set red color for now.
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}