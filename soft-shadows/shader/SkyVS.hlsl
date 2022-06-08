#include "Sky.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    float4x4 projView = mul(proj, view);

    // set z = w so that z/w = 1 (the skybox stays in the far plane)
    float4 posH = mul(projView, float4(vIn.PosL, 0.0f));
    vOut.PosH = posH.xyww;
    vOut.PosL = vIn.PosL;
    return vOut;
}