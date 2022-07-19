struct PSIn
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> backBuffer : register(t0);
Texture2D<float4> depthBuffer : register(t1);
Texture2D<float4> brightessBuffer : register(t2);
sampler defaultSampler : register(s0);

float4 main(PSIn pin) : SV_TARGET
{
    float4 brightness = brightessBuffer.Sample(defaultSampler, pin.TexCoord);
    float4 color = backBuffer.Sample(defaultSampler, pin.TexCoord);

    return brightness;
}
