#include "Bloom.hlsli"

struct PSIn
{
    float4 PositionH : SV_POSITION;
};

cbuffer Constants : register(b0)
{
	float4 color;
};

PSOut main(PSIn pin)
{
    PSOut pout;
    pout.Color = color;
    pout.Brightness = saturatePixel(color);
    return pout;
}
