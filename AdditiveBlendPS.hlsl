#include "Simple.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float4 AdditiveBlendPS(VS_Output input) : SV_TARGET
{
    float4 color = tex.Sample(smp, input.uv);
    color.rgb *= color.a;
    return color * input.color;
}