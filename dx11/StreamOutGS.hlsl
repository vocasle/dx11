#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 InitPosW : POSITION;
	float3 InitVelW : VELOCITY;
	float2 SizeW : SIZE;
	float Age : AGE;
	uint Type : TYPE;
};

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
	point Particle p[1],
	inout PointStream<Particle> ptStream
)
{
	p[0].Age += gTimeStep;

	if (p[0].Type == PT_EMITTER)
	{
		if (p[0].Age > 0.005f)
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

			p[0].Age = 0.0f;
		}

		ptStream.Append(p[0]);
	}
	else
	{
		if (p[0].Age <= 1.0f)
		{
			ptStream.Append(p[0]);
		}
	}
}