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
    static const float threshold = 1.0f;
    static const float4 mask = float4(0.2f, 0.2f, 0.2f, 1.0f);
    if (dot(color, mask) < threshold)
    {
        static const float4 black = float4(0, 0, 0, 1);
        return black;
    }
    return color;
}
