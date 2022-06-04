struct PSIn
{
	float4 PositionH : SV_POSITION;
};

float4 main(PSIn pin) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
