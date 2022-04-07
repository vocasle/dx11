struct VSIn
{
	float3 Pos : POSITION;
	float3 Velocity : VELOCITY;
	float Age : AGE;
	float2 Size : SIZE;
};

struct VSOut
{
	float3 Pos : POSITION;
	float3 Velocity : VELOCITY;
	float Age : AGE;
	float2 Size : SIZE;
};

VSOut main(VSIn vin)
{
	VSOut vout = vin;

	return vout;
}