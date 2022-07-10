#include "Fog.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	//float d = length(cameraPosW - In.PosW);
	//float fogFactor = clamp((d - fogStart) / (fogEnd - fogStart), 0, 1);
	//color.rgb = lerp(color.rgb, fogColor * color.a, fogFactor);
	float4 color = 1.0f - backBuffer.Sample(defaultSampler, pin.TexCoord);
	return color;
}
