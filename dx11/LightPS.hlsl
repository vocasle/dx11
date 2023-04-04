#include "LightingHelper.hlsli"

cbuffer LightingData : register(b0)
{
	PointLight PL;
}

float4 main() : SV_TARGET
{
	float4 color = float4(1.0.xxx, 1.0);
	return color;
}