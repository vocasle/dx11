struct GSOutput
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

float4 main(GSOutput pin) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}