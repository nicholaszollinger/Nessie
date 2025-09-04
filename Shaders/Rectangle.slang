// Vertex and Fragment shader using slang.
// The benefit of slang is that we can include multiple entry points within the same file.

// `Static` variables will tell the compiler that we don't need any bindings.
//static float2 positions[3] = float2[]
//(
//    float2(0.0, -0.5),
//    float2(0.5, 0.5),
//    float2(-0.5, 0.5)
//);

// static float3 colors[3] = float3[]
//(
//    float3(1.0, 0.0, 0.0),
//    float3(0.0, 1.0, 0.0),
//    float3(0.0, 0.0, 1.0)
//);

struct VertexInput
{
    float2 inPosition;
    float3 inColor;
};

struct UniformBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput 
{
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) 
{
    VertexOutput output;
    output.position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 0.0, 1.0))));
    output.color = input.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.color, 1.0);
}