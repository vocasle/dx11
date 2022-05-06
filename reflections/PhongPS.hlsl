#include "Common.hlsli"

float4 main(VSOut In) : SV_TARGET
{
	const float4 diffuseSampled = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	const float4 specularSampled = specularTexture.Sample(defaultSampler, In.TexCoords);
	const float4 glossSampled = glossTexture.Sample(defaultSampler, In.TexCoords);
	const float3 normalSampled = normalTexture.Sample(defaultSampler, In.TexCoords);
	float3 normal = 2.0f * normalSampled - 1.0f; // Uncompress each component from [0,1] to [-1,1].
	
	const float3 viewDir = normalize(cameraPosW - In.PosW);

	LightIntensity intensities[MAX_LIGHTS];
	intensities[0] = DirectionalLightIntensity(dirLight, normal, viewDir);

	const uint numPointLights = 4;
	for (uint i = 0; i < numPointLights; ++i)
	{
		intensities[1 + i] = PointLightIntensity(pointLights[i], normal, In.PosW, viewDir);
	}

	intensities[5] = SpotLightIntensity(spotLights[0], normal, In.PosW, viewDir);

	//return float4(normalize(In.PosW), 1.0f);

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
		6
	);

	return fragmentColor;
	//return float4(1.0f - glossSampled.rgb, 1.0f);
	//return float4(material.Specular.rgb * glossSampled.rgb, 1.0f);
}
