struct PSIn
{
	float4 PositionH : SV_POSITION;
	float3 NormalW 	: NORMAL;
	float2 TexCoord : TEXCOORD;
};


float4 main(PSIn pin) : SV_TARGET
{
	float3 lightPos = {0.0f, 10.0f, -10.0f};
	float3 lightDir = normalize(lightPos);

	float diff = max(dot(lightDir, pin.NormalW), 0.0f);

	return float4(diff,diff,diff,1.0f);
}
