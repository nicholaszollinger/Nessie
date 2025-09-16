// 3D Geometry Shader.
#version 450

layout (set = 0, binding = 0) uniform CameraUniforms
{
    mat4 proj;
    mat4 view;
} u_camera;

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

// Vertex Data:
layout (location = 0) in vec3 inVertPosition;
layout (location = 1) in vec3 inVertNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

// Transform Data:
layout (location = 5) in vec4 inMatRow0;
layout (location = 6) in vec4 inMatRow1;
layout (location = 7) in vec4 inMatRow2;
layout (location = 8) in uint inMeshIndex;
layout (location = 9) in uint inMaterialIndex;

// Output:
layout (location = 0) out VertexOutput outData;

void main() 
{
    // Construct the object matrix from the row data:
    mat4 objectMatrix = mat4
    (
        vec4(inMatRow0.x, inMatRow1.x, inMatRow2.x, 0.0), // Column 0
        vec4(inMatRow0.y, inMatRow1.y, inMatRow2.y, 0.0), // Column 1
        vec4(inMatRow0.z, inMatRow1.z, inMatRow2.z, 0.0), // Column 2
        vec4(inMatRow0.w, inMatRow1.w, inMatRow2.w, 1.0)  // Column 4
    );
    vec4 worldPosition = vec4(objectMatrix * vec4(inVertPosition, 1.0));

    // Set the Output Data
    outData.worldPosition = worldPosition.xyz;
    outData.worldTransform = mat3(objectMatrix);
    outData.worldNormals = mat3(objectMatrix) * mat3(inTangent, inBitangent, inVertNormal);
    outData.normal = normalize((objectMatrix * vec4(inVertNormal, 0)).xyz);
    outData.texCoord = inTexCoord;
    outData.tangent = inTangent;
    outData.bitangent = inBitangent;
    
    gl_Position = u_camera.proj * u_camera.view * objectMatrix * worldPosition;
}