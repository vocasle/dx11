struct VSIn
{
	float3 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct PSIn
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
	float Radius;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
	float Range;
	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
	float Range;
	float3 Direction;
	float Spot;
	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflection;
};

cbuffer PerObjectConstants : register(b0)
{
    float4x4 World;
};

cbuffer PerFrameConstants :  register(b1)
{
    float4x4 View;
    float4x4 Proj;
    float3 CameraPos;
    float Pad;
};

cbuffer PerSceneConstants : register(b2)
{
    PointLight PointLights[4];
    DirectionalLight DirLight;
    SpotLight SpotLights[2];
};

sampler anisSam : register(s0);
Texture2D diffuseMap : register(t0);
