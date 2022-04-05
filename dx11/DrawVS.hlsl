#include "ParticleSystem.hlsli"

cbuffer cbFixed : register(b0)
{
	// Net constant acceleration used to accerlate the particles.
	float3 gAccelW = { 0.0f, 7.8f, 0.0f };

	// Texture coordinates used to stretch texture over quad 
	// when we expand point particle into a quad.
	float2 gQuadTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};

VSOut main( Particle vin )
{
	VSOut vout;

	float t = vin.Age;

	// constant acceleration equation
	vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitVelW + vin.InitPosW;

	// fade color with time
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

	vout.SizeW = vin.SizeW;
	vout.Type = vin.Type;

	return vout;
}