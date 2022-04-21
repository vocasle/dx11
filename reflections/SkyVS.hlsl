#include "Sky.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    float4x4 projViewWorld = mul(proj, view);
    projViewWorld = mul(projViewWorld, world);

    // set z = w so that z/w = 1 (the skybox stays in the far plane)
    float4 posH = mul(projViewWorld, float4(vIn.PosL, 1.0f));
    vOut.PosH = posH.xyww;
    vOut.PosL = vIn.PosL;
    return vOut;
}