struct GSOutput
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

Texture2D defaultTexture : register(t0);
sampler defaultSampler : register(s0);

float4 main(GSOutput pin) : SV_TARGET
{
	return defaultTexture.Sample(defaultSampler, pin.TexCoords) * float4(0.9450980392156863f, 0.3673469387755102f, 0.1333333333333333f, 1.0f);
}