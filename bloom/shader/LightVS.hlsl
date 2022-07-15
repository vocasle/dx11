struct VSIn
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct PSIn
{
    float4 PositionH : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer Constants : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

PSIn main(VSin vin)
{
    PSIn vout;
    float4 posH = mul(projection, float(vin.Position, 1.0f));
    posH = mul(view, posH);
    posH = mul(world, posH);
    vout.PositionH = posH;
    vout.Color = vin.Color;
    return vout;
}