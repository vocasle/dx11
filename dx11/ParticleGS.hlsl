struct GSOutput
{
	float4 pos : SV_POSITION;
};

struct VSOut
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoords : TEXCOORDS;
	float3 PosW : POSITION;
};

[maxvertexcount(4)]
void main(
	point VSOut gin[1],
	inout TriangleStream< GSOutput > gout
)
{
	float4 v[4];
	v[0] = float4(gin[0].PosW + float3(1.0f, -1.0f, 0.0f), 1.0f);
	v[1] = float4(gin[0].PosW + float3(1.0f, 1.0f, 0.0f), 1.0f);
	v[2] = float4(gin[0].PosW + float3(-1.0f, -1.0f, 0.0f), 1.0f);
	v[3] = float4(gin[0].PosW + float3(-1.0f, 1.0f, 0.0f), 1.0f);

	GSOutput element;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		element.pos = v[i];
		gout.Append(element);
	}
}