struct VSIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
};

struct VSOut
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

cbuffer PerFrameConstants : register(b0)
{
	float4x4 world;
	float4x4 worldViewProj;
	float4x4 worldInvTranspose;
}

VSOut main(VSIn In)
{
	VSOut Out;
	Out.PosH = mul(worldViewProj, float4(In.Pos, 1.0f));
	Out.TexCoords = In.TexCoords;
	Out.Normal = mul(worldInvTranspose, float4(In.Normal, 1.0f)).xyz;
	Out.PosW = mul(world, float4(In.Pos, 1.0f)).xyz;
	return Out;
}
