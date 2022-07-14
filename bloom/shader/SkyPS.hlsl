#include "Sky.hlsli"

float4 main(VertexOut pIn) : SV_Target
{
    return cubeMap.Sample(cubeSampler, pIn.PosL);
}
