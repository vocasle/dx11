struct Particle
{
	float3 InitPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW : SIZE;
	float Age : AGE;
	uint Type : TYPE;
};

Particle main( Particle p )
{
	return p;
}