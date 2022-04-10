struct GSOutput
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
	float Age : AGE;
};

Texture2D defaultTexture : register(t0);
sampler defaultSampler : register(s0);

#define MAX_AGE 10.0f

float4 main(GSOutput pin) : SV_TARGET
{
	const float atten = pin.Age / MAX_AGE;
	return defaultTexture.Sample(defaultSampler, pin.TexCoords);
}