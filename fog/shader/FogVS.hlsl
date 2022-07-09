#include "Fog.hlsli"

PSIn main(VSIn vin)
{
    PSIn vout;
    float4x4 wvp = mul(proj, view);
    wvp = mul(wvp, world);
    vout.PosH = mul(wvp, float4(vin, 1.0f));
    return vout;
}
