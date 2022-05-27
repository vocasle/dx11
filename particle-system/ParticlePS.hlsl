#include "Particle.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	const float x = pin.Lifespan / 10.0f;
	float3 col = float3(1.0f / (x + 1.0f) , x / 2.0f, x);
	float4 sampled = diffuseTexture.Sample(defaultSampler, pin.TexCoords);
	if (length(sampled.rgb) > 0.01f)
	{
		sampled.rgb += col;
	}
	return sampled;
}
