#include "Common.hlsli"

cbuffer LightingData : register(b0)
{
	PointLight PL[4];
	float3 CameraPos;
}

float4 main(VSOut In) : SV_TARGET
{
	Material sp;
	sp.Ambient = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	sp.Diffuse = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	sp.Specular = specularTexture.Sample(defaultSampler, In.TexCoords);

	const float3 N = normalize(In.NormalW);
	float3 V = normalize(CameraPos - In.PosW);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		const float3 L = normalize(PL[i].Position - In.PosW);

		const float4 D = sp.Diffuse * PL[i].Diffuse;
		const float4 A = sp.Ambient * PL[i].Ambient;

		float4 Kdiff = DiffuseLighting(D, A, PL[i].Att, N, L);

		float3 H = L + V;
		H = H / length(L + V);
		float4 S = sp.Specular * PL[i].Specular;
		float4 Kspec = SpecularReflection(S, PL[i].Att, N, H, 32.0f);

		diffuse += Kdiff;
		specular += Kspec;
	}



	return diffuse + specular;
}
