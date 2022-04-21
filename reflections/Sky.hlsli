#include "LightingHelper.hlsli"

TextureCube cubeMap : register(t0);
sampler cubeSampler : register(s0);

cbuffer PerObjectConstants : register(b0)
{
	float4x4 world;
	Material material;
};

cbuffer PerFrameConstants : register(b1)
{
	float4x4 view;
	float4x4 proj;
	float4x4 shadowTransform;
	float3 cameraPosW;
};

struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};
