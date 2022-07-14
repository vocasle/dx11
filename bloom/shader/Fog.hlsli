

struct VSIn
{
    float3 Pos : POSITION;
};

struct PSIn
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer Constants : register(b0)
{
    float fogEnd;
    float fogStart;
    float width;
    float height;
    float4 fogColor;
    float4x4 world;
    float4x4 viewInverse;
    float4x4 projInverse;
    float3 cameraPos;
    float _pad1;
};

Texture2D<float4> backBuffer : register(t0);
Texture2D<float4> depthBuffer : register(t1);
sampler defaultSampler : register(s0);
