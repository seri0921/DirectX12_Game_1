// 頂点シェーダからの出力、ピクセルシェーダーの入力
struct VS_Output
{
    // 頂点シェーダの出力としてVS_Output構造体を作成
    // sposはSV_POSITIONセマンティクスを指定したシステム用の位置座標
    // uvのセマンティクスはTEXCOORDに指定
    
    float4 spos : SV_Position;
    float2 uv : TEXCOORD;
};