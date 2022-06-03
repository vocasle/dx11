#include "Common.hlsli"

float4 main(float3 pos : POSITION) : SV_POSITION
{
	float4x4 toLightSpace = mul(shadowTransform, world);
	float4 posH = mul(toLightSpace, float4(pos, 1.0f));
	return posH;
}
