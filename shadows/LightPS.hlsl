#include "Common.hlsli"

float4 main() : SV_TARGET
{
	float4 color = /*PL.Ambient + PL.Diffuse*/float4(1.0f, 1.0f, 1.0f, 1.0f);
	return color;
}