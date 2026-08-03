#include "Simple.hlsli"

cbuffer cbuff : register(b0)
{
    matrix mat;
}

VS_Output SimpleVS( float4 pos : POSITION, float2 uv : TEXCOORD ) 
{
    // 頂点シェーダの出力としてoutputを用意し、 sposとposに頂点バッファから取得した引数のposを保存
    
    VS_Output output;
    
    output.spos = mul(mat, pos);
    output.uv = uv;
    
    return output;
}