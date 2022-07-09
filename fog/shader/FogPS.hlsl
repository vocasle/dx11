#include "Fog.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	//float d = length(cameraPosW - In.PosW);
	//float fogFactor = clamp((d - fogStart) / (fogEnd - fogStart), 0, 1);
	//color.rgb = lerp(color.rgb, fogColor * color.a, fogFactor);
	return fogColor;
}