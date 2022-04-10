struct GSOutput
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 TexCoords  : TEXCOORDS;
};

Texture2D defaultTexture : register(t0);
sampler defaultSampler : register(s0);

float4 main(GSOutput pin) : SV_TARGET
{
	return defaultTexture.Sample(defaultSampler, pin.TexCoords) /** pin.Color*/;
}