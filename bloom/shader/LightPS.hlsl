struct PSIn
{
    float4 PositionH : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer Constants : register(b0)
{
	float4 color;
};

float4 main(PSIn pin) : SV_TARGET
{
	return color;
}