#include "Particle.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	return diffuseTexture.Sample(defaultSampler, pin.TexCoords) + float4(1.0f, 0.0f, 0.0f, 0.0f);
}
