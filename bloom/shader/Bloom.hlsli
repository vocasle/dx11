struct PSOut
{
    float4 Color : SV_TARGET0;
    float4 Brightness : SV_TARGET1;
};

float4 saturatePixel(float4 color)
{
    const float3 threshold = float3(0.2126, 0.7152, 0.0722);
    const float brightess = dot(color.rgb, threshold);

    if (brightess > 0.5f)
    {
        return float4(color.rgb, 1.0f);
    }

    return float4(0, 0, 0, 1);
}
