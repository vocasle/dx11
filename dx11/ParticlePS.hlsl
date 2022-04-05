Texture2D gTexArray : register(t0);
sampler defaultSampler : register(s0);

struct GSOutput
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

float4 main(GSOutput pin) : SV_TARGET
{
	return gTexArray.Sample(defaultSampler, float3(pin.Tex, 0)) * pin.Color;
}