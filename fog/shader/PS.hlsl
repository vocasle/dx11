#include "Common.hlsli"

float4 main(PSIn pin) : SV_TARGET
{
	return diffuseMap.Sample(anisSam, pin.TexCoord);	
}
