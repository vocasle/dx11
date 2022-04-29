struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
    float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
    float Range;
	float3 Att;
    float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
    float Range;
	float3 Direction;
    float Spot;
    float3 Att;
    float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

static const float4 ZERO_VEC4 = { 0.0f, 0.0f, 0.0f, 0.0f };

Material ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye)
{
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = -L.Direction;

    ambient = mat.Ambient * L.Ambient;

    float diffuseFactor = dot(lightVec, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    Material material;
    material.Ambient = ambient;
    material.Diffuse = diffuse;
    material.Specular = spec;

    return material;
}


Material ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye)
{
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    Material material;
    material.Ambient = ambient;
    material.Diffuse = diffuse;
    material.Specular = spec;

    float3 lightVec = L.Position - pos;

    float d = length(lightVec);

    if (d > L.Range)
        return material;

    lightVec /= d;

    ambient = mat.Ambient * L.Ambient;

    float diffuseFactor = dot(lightVec, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;

    material.Ambient = ambient;
    material.Diffuse = diffuse;
    material.Specular = spec;

    return material;
}


Material ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye)
{
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Material material;
    material.Ambient = ambient;
    material.Diffuse = diffuse;
    material.Specular = spec;

    float3 lightVec = L.Position - pos;

    float d = length(lightVec);

    if (d > L.Range)
        return material;

    lightVec /= d;

    ambient = mat.Ambient * L.Ambient;


    float diffuseFactor = dot(lightVec, normal);

    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    float att = spot / dot(L.Att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
    material.Ambient = ambient;
    material.Diffuse = diffuse;
    material.Specular = spec;

    return material;
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
    Texture2D shadowMap,
    float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    // Texel size.
    const float dx = SMAP_DX;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(samShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit /= 9.0f;
}

float3 NormalSampleToWorldSpace(float3 normalMapSample,
    float3 unitNormalW,
    float4 tangentW)
{
    // Restore each component of the normal vector read from [0, 1] to [-1, 1]
    float3 normalT = 2.0f * normalMapSample - 1.0f;

    // Construct tangent space in world coordinate system
    float3 N = unitNormalW;
    float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N); // Schmitt orthogonalization
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // Transform bump normal vector from tangent space to world coordinate system
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float DirectionalLightIntensity(DirectionalLight dl)
{
    return 1.0f;
}

float SpotLightIntensity(SpotLight sl, float3 surfPos)
{
    const float distance = length(sl.Position - surfPos);
    const float3 L = (sl.Position - surfPos) / distance;

    return pow(max(dot(-sl.Direction, L), 0.0f), sl.Spot) / (distance * distance);
}

float PointLightIntensity(PointLight pl, float3 surfPos)
{
    const float distance = length(pl.Position - surfPos);
    const float3 L = (pl.Position - surfPos) / distance;

    return 1.0f / (distance * distance);
}