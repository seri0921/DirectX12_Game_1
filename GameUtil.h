#pragma once

#include "GMath.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <dxgi.h>

// 頂点データ (uv座標対応)
struct VertexUV
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

// 画素データ（RGBA成分、8bit）
struct ColorRGBA
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	ColorRGBA() : r(0), g(0), b(0), a(255) {}
	ColorRGBA(unsigned char red, unsigned char green, unsigned char blue,
		unsigned char alpha) : r(red), g(green), b(blue), a(alpha) {}
	ColorRGBA(XMFLOAT3 color, float alpha = 1.0f);

};

// カラーベクトルの定数
extern const XMFLOAT3 ColorWhite;
extern const XMFLOAT3 ColorBlack;
extern const XMFLOAT3 ColorGray;
extern const XMFLOAT3 ColorRed;
extern const XMFLOAT3 ColorGreen;
extern const XMFLOAT3 ColorBlue;
extern const XMFLOAT3 ColorYellow;
extern const XMFLOAT3 ColorCyan;
extern const XMFLOAT3 ColorMagenta;

// 円の表現
struct Circle
{
	XMFLOAT2 pos;
	float radius;

	Circle() : pos(ZeroVec2d), radius(1.0f) {}
	Circle(XMFLOAT2 p, float r) : pos(p), radius(r) {}
};

// 矩形表現
struct Box
{
	XMFLOAT2 pos;
	float width;
	float height;

	Box() : pos(ZeroVec2d), width(1.0f), height(1.0f) {}
	Box(XMFLOAT2 p, float w, float h) : pos(p), width(w), height(h) {}
};

// 線分表現
struct Segment
{
	XMFLOAT2 start;
	XMFLOAT2 end;

	Segment() : start(ZeroVec2d), end(UnitVecX2d) {}
	Segment(const XMFLOAT2& s, const XMFLOAT2& e) : start(s), end(e) {}
};

// 点と円の衝突（内外）判定
bool detectPointToCircleCollision(XMFLOAT2& p, Circle& c);
// 点と矩形の衝突（内外）判定
bool detectPointToBoxCollision(XMFLOAT2& p, Box& box);
// 円と円の衝突判定
bool detectCircleCollision(Circle& c1, Circle& c2,
	XMFLOAT2* corr = nullptr, float* len = nullptr);
// 円と線分の衝突判定
bool detectCircleToSegmentCollision(Circle& c, Segment& seg);
// 矩形と矩形の衝突判定
bool detectBoxCollision(Box& b1, Box& b2);
// 矩形と円の衝突判定
bool detectBoxToCircleCollision(Box& b, Circle& c);

// 矩形を線分に分割
void separateBoxToSegments(const Box& rect, Segment& left, Segment& right,
	Segment& top, Segment& bottom);

// 画像情報構造体
struct ImageData
{
	int imgIndex;
	int width;
	int height;
	std::wstring filePath;
	DXGI_FORMAT format;

	ImageData() : imgIndex(-1), width(0), height(0),
		format(DXGI_FORMAT_R8G8B8A8_UNORM) {}
};

// スプライト変換データ構造体
struct SpriteTransData
{
	XMMATRIX posMat;
	XMMATRIX uvMat;
	XMFLOAT3 color;
	float alpha;

	SpriteTransData()
		: posMat(XMMatrixIdentity()), uvMat(XMMatrixIdentity()),
		  color(Ones3d), alpha(1.0f) {}
	SpriteTransData(XMMATRIX pos, XMMATRIX uv)
		: posMat(pos), uvMat(uv), color(Ones3d), alpha(1.0f) {}
};

// スプライト座標変換行列
XMMATRIX calcSpriteMatrix(float width, float height);
// スプライトモデル変換行列
XMMATRIX calcSpriteModelMatrix(XMFLOAT2 sclae,
	XMFLOAT2 pos, XMFLOAT2 offset, float theta);

// スプライトのuv座標変換行列
XMMATRIX calcSpriteUVMatrix(const XMFLOAT2& p1, float uw, float uh,
	float imgW, float imgH);
// スプライトのuv座標変換行列
XMMATRIX calcSpriteUVMatrix(const XMFLOAT2& p1, const XMFLOAT2& p2,
	float imgW, float imgH);

// 数値情報を出力ウィンドウに表示
void printNum(const wchar_t* str, int num);

// ランダムノイズ画像の生成
void createRandomNoizeImage(std::vector<ColorRGBA>& img, class Game* game);

// サウンドデータ
struct SoundData
{
	WAVEFORMATEX format;
	std::vector<BYTE> waveData;

	SoundData() : format{} {}
};

// サウンドの参照情報
struct SoundInfo
{
	int index;
	std::wstring filePath;

	SoundInfo() : index(-1) {}
};