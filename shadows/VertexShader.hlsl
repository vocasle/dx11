#include "Common.hlsli"

VSOut main(VSIn In)
{
	VSOut Out;
	float4x4 pvw = mul(proj, view);
	pvw = mul(pvw, world);
	Out.PosH = mul(pvw, float4(In.Pos, 1.0f));
	Out.TexCoords = In.TexCoords;
	Out.NormalW = mul(world, float4(In.Normal, 1.0f)).xyz;
	Out.PosW = mul(world, float4(In.Pos, 1.0f)).xyz;
	Out.ShadowPosH = mul(shadowTransform, float4(In.Pos, 1.0f));
	return Out;
}
