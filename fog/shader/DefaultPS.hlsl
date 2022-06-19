#include "Common.hlsli"

float4 main(VSOut In) : SV_TARGET
{
	return diffuseTexture.Sample(defaultSampler, In.TexCoords);
}
