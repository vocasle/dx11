#include "Fog.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	float fogFactor = clamp((fogEnd - pin.PosH.z) / (fogEnd - fogStart), 0, 1);

//	return float4(fogFactor, fogFactor, fogFactor, 1.0f);
	float4 color =  backBuffer.Sample(defaultSampler, pin.TexCoord);

	float f = pow(2.7f, -(1.0f * pin.PosH.z));
	color = f * color + (1 - f) * fogColor;

//	color.rgb = lerp(color.rgb, (fogColor * color.a).rgb, fogFactor);
	return color;
}
