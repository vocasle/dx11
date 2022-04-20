#include "LightingHelper.hlsli"

struct VSIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
};

struct VSOut
{
	float4 PosH : SV_POSITION;
	float3 NormalW : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

cbuffer PerObjectConstants : register(b0)
{
	float4x4 world;
	Material material;
};

cbuffer PerFrameConstants : register(b1)
{
	float4x4 view;
	float4x4 proj;
	float3 cameraPosW;
};

cbuffer PerSceneConstants : register(b2)
{
	PointLight pointLights[4];
	DirectionalLight dirLight;
	SpotLight spotLights[2];
};

sampler defaultSampler : register(s0);

Texture2D<float4> diffuseTexture	: register(t0);
Texture2D<float4> specularTexture	: register(t1);
Texture2D<float4> glossTexture		: register(t2);
Texture2D<float4> normalTexture		: register(t3);