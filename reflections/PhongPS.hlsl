#include "Common.hlsli"

float4 main(VSOut In) : SV_TARGET
{
	Material mat;
	mat.Ambient = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	mat.Diffuse = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	mat.Specular = specularTexture.Sample(defaultSampler, In.TexCoords);
	mat.Specular.w = material.Specular.w;

	float3 normal = normalTexture.Sample(defaultSampler, In.TexCoords).rgb;

	normal = NormalSampleToWorldSpace(normal, In.NormalW, In.TangentW);

	const float3 toEye = normalize(cameraPosW - In.PosW);

	Material material;
	material.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	material.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	material.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Only the first light casts a shadow
	float shadow[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	Material part;
	part = ComputeDirectionalLight(mat, dirLight, normal, toEye);
	shadow[0] = CalcShadowFactor(shadowSampler, shadowTexture, In.ShadowPosH);

	//return float4(shadow[0], shadow[0], shadow[0], 1.0f);

	material.Ambient += part.Ambient;
	material.Diffuse += part.Diffuse * shadow[0];
	material.Specular += part.Specular * shadow[0];


	for (int i = 0; i < 4; ++i)
	{
		part = ComputePointLight(mat, pointLights[i], In.PosW, normal, toEye);
		material.Ambient += part.Ambient;
		material.Diffuse += part.Diffuse;
		material.Specular += part.Specular;
	}

	//for (int i = 0; i < 2; ++i)
	//{
	//	resultColor += ComputeSpotLight(mat, spotLights[i], In.PosW, normal, toEye);
	//}

	return saturate(material.Ambient + material.Diffuse + material.Specular);
}
