#include "Common.hlsli"

static const float AMBIENT = 0.1f;
static const float PRECISION = 0.000001f;

float4 main(VSOut In) : SV_TARGET
{
    const float2 uv = float2(In.PosW.w, In.NormalW.w);
    const float4 diffuseSampled = diffuseTexture.Sample(defaultSampler, uv);
    const float4 specularSampled = specularTexture.Sample(defaultSampler, uv);
    const float4 glossSampled = glossTexture.Sample(defaultSampler, uv);
    const float3 normalSampled = normalTexture.Sample(defaultSampler, uv).xyz;
    const float3 normal = NormalSampleToWorldSpace(normalSampled, normalize(In.NormalW.xyz), normalize(In.TangentW));
    const float3 viewDir = normalize(cameraPosW - In.PosW.xyz);

    float3 diffuseResult = {0,0,0};
    float3 specularResult = {0,0,0};

    LightIntensity intensities[2];

    intensities[0] = DirectionalLightIntensity(dirLight, normal, viewDir);
    intensities[1] = PointLightIntensity(pointLights[0], normal, In.PosW.xyz, viewDir);
    float3 lightDiffuseResult = {1,1,1};
    [unroll]
    for (int i = 0; i < 2; ++i) {
        float diff = max(dot(intensities[i].L, normal), 0);
        float3 diffuseColor = diffuseSampled.rgb * diff;
        float spec = pow(max(dot(normal, intensities[i].H), 0), 16);
        float3 specularColor = glossSampled.bbb * spec;

        // TODO: Find out how to affect the area near the light source with light color? May be via Bloom?
        diffuseResult += diffuseColor * intensities[i].intensity;
        specularResult += specularColor;
        lightDiffuseResult *= intensities[i].diffuse;
    }

    return float4(AMBIENT * lightDiffuseResult + diffuseResult + specularResult, 1);
}
