#include "Common.hlsli"

float4 main(VSOut In) : SV_TARGET
{
	Material mat;
	mat.Ambient = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	mat.Diffuse = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	mat.Specular = specularTexture.Sample(defaultSampler, In.TexCoords);
	mat.Specular.w = material.Specular.w;

	const float3 normal = normalize(In.NormalW);
	const float3 toEye = normalize(cameraPosW - In.PosW);

	float4 resultColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	resultColor += ComputeDirectionalLight(mat, dirLight, normal, toEye);

	//for (int i = 0; i < 4; ++i)
	//{
	//	resultColor += ComputePointLight(mat, pointLights[i], In.PosW, normal, toEye);
	//}

	//for (int i = 0; i < 2; ++i)
	//{
	//	resultColor += ComputeSpotLight(mat, spotLights[i], In.PosW, normal, toEye);
	//}

	return saturate(resultColor);
}
