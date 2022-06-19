#include "Common.hlsli"

PSIn main(VSIn vin)
{
	PSIn vout;
	vout.TexCoord = vin.TexCoord;
	vout.Norm = mul(World, float4(vin.Norm, 0.0)).xyz;
	float4x4 projViewWorld = mul(Proj, View);
	projViewWorld = mul(projViewWorld, World);
	vout.Pos = mul(projViewWorld, float4(vin.Pos, 1.0));
	return vout;
}
