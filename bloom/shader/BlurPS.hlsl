Texture2D<float4> brightessBuffer : register(t0);
sampler defaultSampler : register(s0);

cbuffer Constants : register(b0) {
    float width;
    float height;
    int isHorizontal;
    int mipLevel;
};

struct VSOut {
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4
main(VSOut pin)
    : SV_TARGET {
    const float dx = (1.0f / width);
    const float dy = (1.0f / height);

    static float weight[5] = {
        0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

    float3 result =
        brightessBuffer.Sample(defaultSampler, pin.TexCoord).rgb * weight[0];
    if (isHorizontal == 1) {
        for (int i = 1; i < 5; ++i) {
            result +=
                brightessBuffer
                    .Sample(defaultSampler, pin.TexCoord + float2(dx * i, 0))
                    .rgb *
                weight[i];

            result +=
                brightessBuffer
                    .Sample(defaultSampler, pin.TexCoord - float2(dx * i, 0))
                    .rgb *
                weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result +=
                brightessBuffer
                    .Sample(defaultSampler, pin.TexCoord + float2(0, dy * i))
                    .rgb *
                weight[i];

            result +=
                brightessBuffer
                    .Sample(defaultSampler, pin.TexCoord - float2(0, dy * i))
                    .rgb *
                weight[i];
        }
    }

    return float4(result, 1.0);
}
