Texture2D<float4> colorBuffer : register(t0);
sampler defaultSampler : register(s0);

struct VSOut
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 main(VSOut pin) : SV_TARGET
{
    float4 color = colorBuffer.Sample(defaultSampler, pin.TexCoord);
    //float brightness = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    if (length(color.rgb) < 1.0)
    {
        static const float4 black = float4(0, 0, 0, 1);
        return black;
    }
    return color;
}
