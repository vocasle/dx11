#include "Fog.hlsli"

PSIn main(VSIn vin)
{
    PSIn vout;
    vout.PosH = float4(vin.Pos.xy, 0.0f, 1.0f);
    vout.TexCoord = float2((vin.Pos.x + 1) / 2.0f, -(vin.Pos.y - 1) / 2.0f);
    return vout;
}
