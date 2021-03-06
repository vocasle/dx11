struct VSIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
};

struct VSOut
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

cbuffer PerFrameConstants : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
}

VSOut main(VSIn In)
{
	VSOut Out;
	float4x4 pvw = mul(proj, view);
	pvw = mul(pvw, world);
	Out.Pos = mul(pvw, float4(In.Pos, 1.0f));
	Out.TexCoords = In.TexCoords;
	Out.Normal = mul(world, float4(In.Normal, 1.0f)).xyz;
	Out.PosW = mul(world, float4(In.Pos, 1.0f)).xyz;
	return Out;
}
