struct VSIn
{
	float3 InitPosW : POSITION;
	float3 InitVelW : VELOCITY;
	float2 SizeW : SIZE;
	float Age : AGE;
};

struct VSOut
{
	float3 PosW : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
};

VSOut main(VSIn vin)
{
	static const float3 gAccelW = { 0.0f, 7.8f, 0.0f };

	VSOut vout = vin;

	const float t = vin.Age;

	vout.PosW = 0.1f * t * t * gAccelW + t * vin.InitVelW + vin.InitPosW;

	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

	return vout;
}