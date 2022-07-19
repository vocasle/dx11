struct PSIn
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> backBuffer : register(t0);
Texture2D<float4> depthBuffer : register(t1);
Texture2D<float4> brightessBuffer : register(t2);
sampler defaultSampler : register(s0);

cbuffer Constants : register(b0)
{
    float width;
    float height;
    float2 pad;
};

float4 main(PSIn pin) : SV_TARGET
{
    float4 brightness = brightessBuffer.Sample(defaultSampler, pin.TexCoord);
    float4 color = backBuffer.Sample(defaultSampler, pin.TexCoord);

    const float dx = 1.0f / width;
    const float dy = 1.0f / height;

    float4 result = {0,0,0,0};

    static const int r = 10;
    static const float divisor = (2 * r + 1) * (2 * r + 1);

    for (int y = -r; y <= r; ++y)
    {
        for (int x = -r; x <= r; ++x)
        {
            const float2 tc = pin.TexCoord + float2(dx * x, dy * y);
            result += brightessBuffer.Sample(defaultSampler, tc);
        }
    }

    float4 blurColor = result / divisor;

    return color + blurColor;
}
