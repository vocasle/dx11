#include "Common.hlsli"

cbuffer LightingData : register(b0)
{
	PointLight PL;
	float3 CameraPos;
}

float4 main(VSOut In) : SV_TARGET
{
	Material sp;
	sp.Ambient = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	sp.Diffuse = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	sp.Specular = specularTexture.Sample(defaultSampler, In.TexCoords);

	const float3 L = normalize(PL.Position - In.PosW);
	const float3 N = normalize(In.NormalW);
	const float4 D = sp.Diffuse * PL.Diffuse;
	const float4 A = sp.Ambient * PL.Ambient;
	const float Atten = Attenuation(0.5f, 0.1f, 0.01f, length(L));

	float4 Kdiff = DiffuseLighting(D, A, Atten, N, L);

	float3 V = normalize(CameraPos - In.PosW);
	float3 H = L + V;
	H = H / length(L + V);

	float4 S = sp.Specular * PL.Specular;
	float4 Kspec = SpecularReflection(S, Atten, N, H, 32.0f);

	return Kdiff + Kspec;
}
