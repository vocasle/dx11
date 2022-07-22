struct PSIn
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> backBuffer : register(t0);
Texture2D<float4> blurHor : register(t1);
Texture2D<float4> blurVert : register(t2);
sampler defaultSampler : register(s0);


float4 main(PSIn pin) : SV_TARGET
{
    float4 hor = blurHor.Sample(defaultSampler, pin.TexCoord);
    float4 ver = blurVert.Sample(defaultSampler, pin.TexCoord);
    float4 color = backBuffer.Sample(defaultSampler, pin.TexCoord);

    //return hor;

    float3 result = color.rgb + hor.rgb + ver.rgb;
    return float4(result, 1.0f);
}
