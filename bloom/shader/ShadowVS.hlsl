#include "Common.hlsli"


VSOut main(VSIn vin)
{
	VSOut vout = EMPTY_VSOUT;
float4x4 pvw = mul(proj, view);
pvw = mul(pvw, world);
    vout.PosH = mul(pvw, float4(vin.Pos, 1.0f));
	return vout;
}
