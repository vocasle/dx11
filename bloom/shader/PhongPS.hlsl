#include "Common.hlsli"

static const float4 AMBIENT = float4(0.4f, 0.4f, 0.4f, 1.0f);

float4 main(VSOut In) : SV_TARGET
{
	const float4 diffuseSampled = diffuseTexture.Sample(defaultSampler, In.TexCoords);
	const float4 specularSampled = specularTexture.Sample(defaultSampler, In.TexCoords);
	const float4 glossSampled = glossTexture.Sample(defaultSampler, In.TexCoords);
	const float3 normalSampled = normalTexture.Sample(defaultSampler, In.TexCoords).xyz;
    float2 shadowUV = float2(In.ShadowPosH.x * 0.5f + 0.5f,
                                               -In.ShadowPosH.y * 0.5f + 0.5f);
	const float4 shadowSampled = shadowTexture.Sample(defaultSampler, shadowUV);
	float3 normal = NormalSampleToWorldSpace(normalSampled, normalize(In.NormalW), normalize(In.TangentW));

    //return float4(shadowSampled.xxx / In.ShadowPosH.z , 1.0f);
//	return float4(In.ShadowPosH.zzz, 1.0f);

	normal = normalize(normal);
	//normal = normalize(In.NormalW);

	//float3 L = normalize(dirLight.Position);
	//float diff = max(dot(L, normal), 0.0f);
	//return float4(diffuseSampled.rgb * diff, 1.0f);

	const float3 viewDir = normalize(cameraPosW - In.PosW);

	LightIntensity intensities[MAX_LIGHTS];
	intensities[0] = DirectionalLightIntensity(dirLight, normal, viewDir);

	const uint numPointLights = 4;
	for (uint i = 0; i < numPointLights; ++i)
	{
		intensities[1 + i] = PointLightIntensity(pointLights[i], normal, In.PosW, viewDir);
	}

//	intensities[5] = SpotLightIntensity(spotLights[0], normal, In.PosW, viewDir);
	float shadows[MAX_LIGHTS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	//shadows[0] = CalcShadowFactor(shadowSampler, shadowTexture, In.ShadowPosH);

	//const float diff = dot(normalize(normal), normalize(dirLight.Position));
	//return float4(diff, diff, diff, 1.0f);

	//if (In.ShadowPosH.z > shadowSampled.x)
	//{
	//	shadows[0] = 0.0f;
	//}
	shadows[0] = CalcShadowFactor(shadowSampler, shadowTexture, In.ShadowPosH);

	//return float4(shadowSampled.xyz /** 2.0f - 1.0f*/, 1.0f);


	const float4 emissive = ZERO_VEC4;
	const float4 emissiveSampled = ZERO_VEC4;
	Color fragmentColor = BlinnPhong(
		emissive,
		emissiveSampled,
		material.Diffuse,
		diffuseSampled,
		AMBIENT,
		specularSampled,
		glossSampled,
		material.Specular.a,
		normal,
		intensities,
		5
	);

	// calculate reflections
	float3 incident = -viewDir;
	float3 reflVector = reflect(incident, normal);
	float4 reflColor = envTexture.Sample(defaultSampler, reflVector);

	//fragmentColor += reflColor * material.Reflection;

	float4 color = fragmentColor.Emissive + /* shadows[0] * */ (fragmentColor.Diffuse + fragmentColor.Specular);
	return color;
}
