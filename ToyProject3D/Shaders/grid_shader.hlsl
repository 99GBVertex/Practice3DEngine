#define _DXC 1

// Vertex input
struct VertexInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR0;
    [[vk::location(2)]] float3 normal : NORMAL0;
    [[vk::location(3)]] float2 uv : TEXCOORD0;
};

// Vertex output
struct VertexOutput
{
	[[vk::location(0)]] float3 fragColor : TEXCOORD0;
    float4 position : SV_POSITION;
};

cbuffer GlobalBuffer : register(b0)
{
    float4x4 projectionViewMatrix;
    float3 lightDirection;
};

struct Push
{
    float4x4 modelMatrix;
};

#ifdef _DXC
[[vk::push_constant]] Push push;
#else
[[vk::push_constant]] ConstantBuffer<Push> push;
#endif

VertexOutput VSMain(VertexInput In)
{
    VertexOutput Out;
	
	Out.position = mul(projectionViewMatrix, mul(push.modelMatrix, float4(In.position, 1.0)));
	Out.fragColor = In.color;
    return Out;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
    float4 color = float4(1.f, 1.f, 1.f, 0.1f);
    
    return color;
}