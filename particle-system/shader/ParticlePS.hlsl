#include "Particle.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	float4 sampled = diffuseTexture.Sample(defaultSampler, pin.TexCoords) * float4(color, 1);
	return sampled;
}
