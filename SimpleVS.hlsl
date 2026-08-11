#include "Simple.hlsli"

cbuffer cbuff : register(b0)
{
    matrix mat;
    matrix uvMat;
    float4 color;
}

VS_Output SimpleVS( float4 pos : POSITION, float2 uv : TEXCOORD ) 
{
    // 頂点シェーダの出力としてoutputを用意し、 sposとposに頂点バッファから取得した引数のposを保存
    
    VS_Output output;
    
    output.spos = mul(mat, pos);
    output.uv = mul(uvMat, float4(uv, 0.0f, 1.0f)).xy;
    output.color = color;
    
    return output;
}