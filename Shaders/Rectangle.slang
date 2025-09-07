// Vertex and Fragment shader using slang.
// The benefit of slang is that we can include multiple entry points within the same file.

struct VertexInput
{
    float2 inPosition;
    float2 inTexCoord;
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
    float2 fragTexCoord;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) 
{
    VertexOutput output;
    output.position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 0.0, 1.0))));
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

Texture2D texture;
SamplerState sampler;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.color * texture.Sample(sampler, inVert.fragTexCoord).rgb, 1.0);
}