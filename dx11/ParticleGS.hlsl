struct GSOutput
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 TexCoords  : TEXCOORDS;
};

struct VSOut
{
	float3 PosW : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
};

cbuffer cbPerFrameConstants : register(b0)
{
	float4x4 proj;
	float4x4 worldInvTranspose;
	float4x4 world;
	float4x4 view;
	float3 camPosW;
}

[maxvertexcount(4)]
void main(
	point VSOut gin[1],
	inout TriangleStream<GSOutput> triStream
)
{

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = camPosW - gin[0].PosW;
	look.y = 0.0f; // y-axis aligned, so project to xz-plane
	look = normalize(look);
	float3 right = cross(up, look);

	float4 v[4];
	float halfWidth = 0.5f * gin[0].SizeW.x;
	float halfHeight = 0.5f * gin[0].SizeW.y;

	v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);


	GSOutput gout;

	const float4x4 projView = mul(proj, view);

	static const float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.PosH = mul(projView, v[i]);
		gout.TexCoords = gTexC[i];
		gout.Color = gin[0].Color;
		triStream.Append(gout);
	}
}