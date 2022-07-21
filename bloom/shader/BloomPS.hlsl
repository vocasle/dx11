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

    float3 result = color.rgb + hor.rgb + ver.rgb;
    float exposure = 1.0f;
    result = float3(1, 1, 1) - exp(-result * exposure);

    float gamma = 2.2f;

    result = pow(result, 1.0 / gamma);

    return float4(result, 1.0f);
}
