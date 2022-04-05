#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 InitPosW : POSITION;
	float3 InitVelW : VELOCITY;
	float2 SizeW : SIZE;
	float Age : AGE;
	uint Type : TYPE;
};

struct VSOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint   Type  : TYPE;
};