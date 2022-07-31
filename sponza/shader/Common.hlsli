#include "LightingHelper.hlsli"

struct VSIn
{
	float4 Pos : POSITION;
	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
};

struct VSOut
{
	float4 PosH : SV_POSITION;
	float4 NormalW : NORMAL;
	float4 PosW : POSITION;
	float4 ShadowPosH : TEXCOORD1;
	float4 TangentW : TANGENT;
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
	// PointLight pointLights[4];
	DirectionalLight dirLight;
	// SpotLight spotLights[2];
	// float4 fogColor;
	// float fogStart;
	// float fogEnd;
	// float pad[2];
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
