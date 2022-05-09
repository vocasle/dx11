#include "Common.hlsli"

static const float4 AMBIENT = float4(0.1f, 0.1f, 0.1f, 1.0f);

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
	float shadows[MAX_LIGHTS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	shadows[0] = CalcShadowFactor(shadowSampler, shadowTexture, In.ShadowPosH);

	const float4 emissive = ZERO_VEC4;
	const float4 emissiveSampled = ZERO_VEC4;
	const float4 fragmentColor = BlinnPhong(
		emissive,
		emissiveSampled,
		material.Diffuse,
		diffuseSampled,
		AMBIENT,
		material.Specular,
		glossSampled,
		128.0f,
		normal,
		intensities,
		shadows,
		6
	);

	return fragmentColor;
}
