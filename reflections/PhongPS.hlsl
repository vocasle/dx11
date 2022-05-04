#include "Common.hlsli"

float4 main(VSOut In) : SV_TARGET
{
	const float4 diffuseSampled = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	const float4 specularSampled = specularTexture.Sample(defaultSampler, In.TexCoords);
	const float4 glossSampled = glossTexture.Sample(defaultSampler, In.TexCoords);

	const float3 normal = normalize(In.NormalW);
	const float3 viewDir = normalize(cameraPosW - In.PosW);

	LightIntensity intensities[MAX_LIGHTS];
	//intensities[0] = DirectionalLightIntensity(dirLight, normal, viewDir);
	intensities[/*1*/0] = PointLightIntensity(pointLights[0], normal, In.PosW, viewDir);

	//return float4(In.NormalW.xyz * 0.5f + 0.5f, 1.0f);
	//return float4(intensities[0].intensity, 1.0f);
	const float4 emissive = ZERO_VEC4;
	const float4 emissiveSampled = ZERO_VEC4;
	const float4 fragmentColor = BlinnPhong(
		emissive,
		emissiveSampled,
		material.Diffuse,
		diffuseSampled,
		material.Ambient,
		material.Specular,
		glossSampled,
		128.0f,
		normal,
		intensities,
		1
	);

	return fragmentColor;
	//return float4(1.0f - glossSampled.rgb, 1.0f);
	//return float4(material.Specular.rgb * glossSampled.rgb, 1.0f);
}
