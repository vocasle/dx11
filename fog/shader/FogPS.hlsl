#include "Fog.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
    static const float MAX_DEPTH = 1000;

    float4 depth = depthBuffer.Sample(defaultSampler, pin.TexCoord);
    float d = depth.x;
	float4 color =  backBuffer.Sample(defaultSampler, pin.TexCoord);
	float fogFactor = clamp((fogEnd - d) / (fogEnd - fogStart), 0, 1);
    color = fogFactor * color + (1.0f - fogFactor) * fogColor;
	return color;
}
