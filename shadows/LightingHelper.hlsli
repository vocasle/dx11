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

float Attenuation(float Kc, float Kl, float Kq, float d)
{
	return 1.0f / (Kc + Kl * d + Kq * d * d);
}

float4 DiffuseLighting(float4 D, float4 A, float Atten, float3 N, float3 L)
{
	float4 Kdiff = D * A + D * Atten * max(dot(N, L), 0);
	return Kdiff;
}

float4 SpecularReflection(float4 S, float Atten, float3 N, float3 H, float shininess)
{
	float4 Kspec = S * Atten * pow(max(dot(N, H), 0), shininess);
	return Kspec;
}