#include "ParticleSystem.hlsli"


cbuffer cbPerFrame : register(b0)
{
	float3 gCamPosW;
	float3 gEmitPosW;
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;
}

Texture1D gRandomTex : register(t0);
sampler defaultSampler : register(s0);

float3 RandomUnitVec3(float offset)
{
	const float u = gGameTime + offset;
	float3 v = gRandomTex.SampleLevel(defaultSampler, u, 0).xyz;
	return normalize(v);
}

[maxvertexcount(2)]
void main(
	point Particle gin[1],
	inout PointStream<Particle> ptStream
)
{
	gin[0].Age += gTimeStep;

	if (gin[0].Type == PT_EMITTER)
	{
		if (gin[0].Age > 0.005f)
		{
			float3 vRandom = RandomUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

			Particle part;
			part.InitPosW = gEmitPosW;
			part.InitVelW = 4.0f * vRandom;
			part.SizeW = float2(3.0f, 3.0f);
			part.Age = 0.0f;
			part.Type = PT_FLARE;

			ptStream.Append(part);

			gin[0].Age = 0.0f;
		}

		ptStream.Append(gin[0]);
	}
	else
	{
		if (gin[0].Age <= 1.0f)
		{
			ptStream.Append(gin[0]);
		}
	}
}