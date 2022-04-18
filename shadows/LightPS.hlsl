#include "LightingHelper.hlsli"

cbuffer LightingData : register(b0)
{
	PointLight PL;
}

float4 main() : SV_TARGET
{
	float4 color = PL.Ambient + PL.Diffuse;
	return color;
}