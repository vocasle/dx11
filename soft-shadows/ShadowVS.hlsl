#include "Common.hlsli"

VSOut main(VSIn vin)
{
	//float4x4 toLightSpace = mul(shadowTransform, world);
	//float4 posH = mul(toLightSpace, float4(pos, 1.0f));
	VSOut vout = EMPTY_VSOUT;
	float posH = mul(proj, float4(vin.Pos, 1.0f));
	posH = mul(view, posH);
	posH = mul(world, posH);
	vout.PosH = posH;
	return vout;
}
