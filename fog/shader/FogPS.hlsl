#include "Fog.hlsli"

static const float e = 2.71828;
static const float zNear = 0.1;

float4 main(PSIn pin) : SV_TARGET
{
    float4 depth = depthBuffer.Sample(defaultSampler, pin.TexCoord);
	float4 color =  backBuffer.Sample(defaultSampler, pin.TexCoord);

	float d = (1.0 - zFar / zNear) * depth.x + (zFar / zNear);
	d = 1.0 / d;
	d = d * zFar;

	float fogFactor = exp2(-(d * fogDensity) * (d * fogDensity));

	float4 outColor = lerp(fogColor, color, fogFactor);

	return outColor;
}
