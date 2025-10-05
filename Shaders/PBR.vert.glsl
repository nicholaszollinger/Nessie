// 3D Geometry Shader.
#version 450

layout (set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
    float exposureFactor;
} u_camera;

//------------------------------------
// Push Constants
//------------------------------------
layout (push_constant) uniform InstanceData
{
    mat4 modelMatrix;   // Transform vertices from object -> world space.
    mat4 normalMatrix;  // Transform normals/tangents from object -> world space.
    uint meshIndex;
    uint materialIndex;
} u_instance;

// Vertex Data:
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

// Output:
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outWorldTangent;
layout(location = 4) out vec3 outWorldBitangent;
layout(location = 5) out vec3 outViewPos; // Vertex Position in View Space.

void main()
{
    vec4 worldPosition = u_instance.modelMatrix * vec4(inPosition, 1.0);
    outWorldPos = worldPosition.xyz;
    outTexCoord = inTexCoord;
    
    mat3 normalMatrix = mat3(u_instance.normalMatrix);
    outWorldNormal = normalize(normalMatrix * inNormal);
    outWorldTangent = normalize(normalMatrix * inTangent);
    outWorldBitangent = cross(outWorldNormal, outWorldTangent);
    outViewPos = (u_camera.view * worldPosition).xyz;
    
    gl_Position = u_camera.viewProj * worldPosition;
}