struct PSIn
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float Lifespan : LIFESPAN;
};

struct VSIn
{
	float3 Pos : POSITION;
	float Lifespan : LIFESPAN;
};

Texture2D<float4> diffuseTexture : register(t0);
sampler defaultSampler : register(s0);

cbuffer PerFrameConstants : register(b0)
{
	float4x4 view;
	float4x4 proj;
};