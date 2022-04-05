struct PSIn
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

sampler defaultSampler : register(s0);
Texture2D<float4> defaultTexture : register(t0);

float4 main(PSIn In) : SV_TARGET
{
	return defaultTexture.Sample(defaultSampler, In.TexCoords);
}
