static const float4 ZERO_VEC4 = { 0.0f, 0.0f, 0.0f, 0.0f };
static const float4 ONE_VEC4 = { 1.0f, 1.0f, 1.0f, 1.0f };
static const uint MAX_LIGHTS = 8;

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
    float Radius;
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
    float4 Reflection;
};

struct LightIntensity
{
    float3 intensity;
    float3 L;
    float3 H;
    float3 diffuse;
};

LightIntensity DirectionalLightIntensity(DirectionalLight light, float3 normal, float3 viewDir)
{
    LightIntensity intensity;
    intensity.intensity = float3(1.0f, 1.0f, 1.0f);
    const float3 L = normalize(light.Position);
    const float3 H = normalize(L + viewDir);
    intensity.L = L;
    intensity.H = H;
    intensity.diffuse = light.Diffuse.rgb;
    return intensity;
}

LightIntensity PointLightIntensity(PointLight light, float3 normal, float3 surfPoint, float3 viewDir)
{
    LightIntensity intensity;
    const float distance = length(light.Position - surfPoint);
    const float atten = 1.0f / (distance * distance);
    intensity.intensity = float3(atten, atten, atten);
    const float3 L = normalize(light.Position - surfPoint);
    const float3 H = normalize(L + viewDir);
    intensity.L = L;
    intensity.H = H;
    intensity.diffuse = light.Diffuse.rgb;
    return intensity;
}

LightIntensity SpotLightIntensity(SpotLight light, float3 normal, float3 surfPoint, float3 viewDir)
{
    LightIntensity intensity;
    const float distance = length(light.Position - surfPoint);
    const float3 L = normalize(light.Position - surfPoint);
    const float atten = pow(max(dot(-light.Direction, L), 0.0f), light.Spot) / (distance * distance);
    intensity.intensity = float3(atten, atten, atten);
    const float3 H = normalize(L + viewDir);
    intensity.L = L;
    intensity.H = H;
    intensity.diffuse = light.Diffuse.rgb;
    return intensity;
}

// Blinn-Phong
// K = EM + DTA + Sum {Ci[DT(dot(N,Li) + SG(dot(N,Hi)^m]}
// E - emission color
// M - sampled color from emission map
// D - surface's diffuse reflection color
// T - sampled color from diffuse map
// A - ambient intensity
// Ci - light intensity
// N - normal in world space
// Li - unit vector that point from surface point towards i-th light source
// S - surface's specular color
// G - sampled color from gloss map
// Hi - halfway vector of i-th light source, i.e. H = Li + V / ||Li + V||, where V is unit direction vector to viewer from surface point
// m - specular exponent
float4 BlinnPhong(float4 E, 
    float4 M, 
    float4 D, 
    float4 T, 
    float4 A, 
    float4 S, 
    float4 G, 
    float m, 
    float3 N, 
    LightIntensity intensities[MAX_LIGHTS],
    float shadows[MAX_LIGHTS],
    uint numLights)
{
    float4 sum = ZERO_VEC4;
    const float4 EM = E * M;
    const float4 DT = D * T;
    const float4 SG = float4(S.rgb, 1.0f) * G;
    for (uint i = 0; i < numLights; ++i)
    {
        const float3 Ci = intensities[i].intensity;
        const float3 Li = normalize(intensities[i].L);
        const float3 Hi = normalize(intensities[i].H);
        const float3 lightDiffuse = intensities[i].diffuse;
        sum += float4(Ci, 1.0f) * (shadows[i] * DT * float4(lightDiffuse, 1.0f) * max(dot(N, Li), 0.0f) + SG * pow(max(dot(N, Hi), 0.0f), m));
    }
    return EM + DT * A + sum;
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

//---------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample,
    float3 unitNormalW,
    float3 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;
    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);
    return bumpedNormalW;
}