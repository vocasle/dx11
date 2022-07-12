#include "Fog.hlsli"

// this is supposed to get the world position from the depth buffer
float3 WorldPosFromDepth(float depth,
    float2 texCoord,
    float4x4 projMatrixInv,
    float4x4 viewMatrixInv)
{
    float z = depth * 2.0 - 1.0;

    float4 clipSpacePosition = float4(texCoord * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(projMatrixInv, clipSpacePosition);

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    float4 worldSpacePosition = mul(viewMatrixInv, viewSpacePosition);

    return worldSpacePosition.xyz;
}


float4 main(PSIn pin) : SV_TARGET
{
    static const float MAX_DEPTH = 1000;

    float4 depth = depthBuffer.Sample(defaultSampler, pin.TexCoord);
//    return float4(depth.xxx, 1.0f);

    float d = WorldPosFromDepth(depth.x, pin.TexCoord, projInverse, viewInverse).x;

	float4 color =  backBuffer.Sample(defaultSampler, pin.TexCoord);
	float fogFactor = clamp((fogEnd - d) / (fogEnd - fogStart), 0, 1);
    color = fogFactor * color + (1.0f - fogFactor) * fogColor;
	return color;
}
