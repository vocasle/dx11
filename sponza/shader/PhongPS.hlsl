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

    const LightIntensity dirLightInt = DirectionalLightIntensity(dirLight, normal, viewDir);

    float diff = max(dot(dirLightInt.L, normal), 0);
    float3 diffuseColor = diffuseSampled.rgb * diff;
    float spec = pow(max(dot(normal, dirLightInt.H), 0), 16);
    float3 specularColor = glossSampled.bbb * spec;
    return float4(AMBIENT * dirLight.Diffuse.rgb + diffuseColor + specularColor, 1);
}
