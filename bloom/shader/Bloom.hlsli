struct PSOut
{
    float4 Color : SV_TARGET0;
    float4 Brightness : SV_TARGET1;
};

float4 saturatePixel(float4 color)
{
    return float4(color.r * color.r, color.g * color.g, color.b * color.b, color.a);
}
