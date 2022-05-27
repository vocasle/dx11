#include "Particle.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	float3 col = float3(sin(pin.Lifespan), 0.0f, pin.Lifespan / 10.0f);
	float4 sampled = diffuseTexture.Sample(defaultSampler, pin.TexCoords);
	if (length(sampled.rgb) > 0.01f)
	{
		sampled.rgb += col;
	}
	return sampled;
}
