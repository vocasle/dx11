#include "LightingHelper.hlsli"

struct VSIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 TexCoords : TEXCOORDS;
};

struct VSOut
{
	float4 PosH : SV_POSITION;
	float3 NormalW : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
	float4 ShadowPosH : TEXCOORD1;
	float3 TangentW : TANGENT;
	float3 BitangentW : BITANGENT;
};

cbuffer PerObjectConstants : register(b0)
{
	float4x4 worldInvTranspose;
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

cbuffer PerSceneConstants : register(b2)
{
	PointLight pointLights[4];
	DirectionalLight dirLight;
	SpotLight spotLights[2];
};

sampler defaultSampler					: register(s0);
SamplerComparisonState shadowSampler	: register(s1);

Texture2D<float4> diffuseTexture	: register(t0);
Texture2D<float4> specularTexture	: register(t1);
Texture2D<float4> glossTexture		: register(t2);
Texture2D<float4> normalTexture		: register(t3);
Texture2D<float4> shadowTexture		: register(t4);
TextureCube	cubeTexture				: register(t5);
TextureCube envTexture				: register(t6);