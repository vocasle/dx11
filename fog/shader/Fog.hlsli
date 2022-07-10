

struct VSIn
{
    float3 Pos : POSITION;
};

struct PSIn
{
    float4 PosH : SV_POSITION;
};

cbuffer Constants : register(b0)
{
    float fogEnd;
    float fogStart;
    float2 _pad;
    float4 fogColor;
    float4x4 world;
    float4x4 view;
    float4x4 proj;
    float3 cameraPos;
    float _pad1;
};

Texture2D<float4> backBuffer : register(t0);
sampler defaultSampler : register(s0);
