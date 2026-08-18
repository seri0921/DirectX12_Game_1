#include "GameUtil.h"
#include "Game.h"

// カラーベクトルの定数
const XMFLOAT3 ColorWhite(1.0f, 1.0f, 1.0f);
const XMFLOAT3 ColorBlack(0.0f, 0.0f, 0.0f);
const XMFLOAT3 ColorGray(0.5f, 0.5f, 0.5f);
const XMFLOAT3 ColorRed(1.0f, 0.0f, 0.0f);
const XMFLOAT3 ColorGreen(0.0f, 1.0f, 0.0f);
const XMFLOAT3 ColorBlue(0.0f, 0.0f, 1.0f);
const XMFLOAT3 ColorYellow(1.0f, 1.0f, 0.0f);
const XMFLOAT3 ColorCyan(0.0f, 1.0f, 1.0f);
const XMFLOAT3 ColorMagenta(1.0f, 0.0f, 1.0f);

ColorRGBA::ColorRGBA(XMFLOAT3 color, float alpha)
	: r((unsigned char)(255.0f * color.x))
	, g((unsigned char)(255.0f * color.y))
	, b((unsigned char)(255.0f * color.z))
	, a((unsigned char)(255.0f * alpha))
{
}

// 点と円の衝突（内外）判定
bool detectPointToCircleCollision(XMFLOAT2& p, Circle& c)
{
	XMFLOAT2 d = p - c.pos;
	if (lengthSq(d) <= c.radius * c.radius) return true;
	return false;
}

// 点と矩形の衝突（内外）判定
bool detectPointToBoxCollision(XMFLOAT2& p, Box& box)
{
	XMFLOAT2 p1 = box.pos;
	XMFLOAT2 p2 = box.pos + XMFLOAT2(box.width, box.height);
	if (p.x < p1.x) return false;
	if (p.x > p2.x) return false;
	if (p.y < p1.y) return false;
	if (p.y > p2.y) return false;
	return true;
}

// 円と円の衝突判定
bool detectCircleCollision(Circle& c1, Circle& c2,
	XMFLOAT2* corr, float* len)
{
	XMFLOAT2 d = c2.pos - c1.pos;
	float th = c1.radius + c2.radius;
	if (lengthSq(d) < th * th)
	{
		if (corr != nullptr) *corr = normalize(d);
		if (len != nullptr) *len = th - length(d);

		return true;
	}
	return false;
}

// 円と線分の衝突判定
bool detectCircleToSegmentCollision(Circle& c, Segment& seg)
{
	XMFLOAT2 p = seg.end - seg.start;
	XMFLOAT2 q = c.pos - seg.start;
	float d = std::abs(cross(p, q) / length(p));
	if (d > c.radius) return false;

	XMFLOAT2 w = c.pos - seg.end;
	float t = dot(p, q) * dot(-1.0f * p, w);
	if (t >= 0.0f) return true;

	float csq = c.radius * c.radius;
	if (lengthSq(q) <= csq) return true;
	if (lengthSq(w) <= csq) return true;
	return false;
}

// 矩形を線分に分割
void separateBoxToSegments(const Box& rect, Segment& left, Segment& right,
	Segment& top, Segment& bottom)
{
	XMFLOAT2 p1 = rect.pos;
	XMFLOAT2 p2 = rect.pos + XMFLOAT2(rect.width, 0.0f);
	XMFLOAT2 p3 = rect.pos + XMFLOAT2(0.0f, rect.height);
	XMFLOAT2 p4 = rect.pos + XMFLOAT2(rect.width, rect.height);

	left = Segment(p1, p3);
	right = Segment(p2, p4);
	top = Segment(p1, p2);
	bottom = Segment(p3, p4);
}

void printNum(const wchar_t* str, int num)
{
	static wchar_t buf[1024];
	wsprintfW(buf, str, num);
	OutputDebugStringW(buf);
}

// ランダムノイズ画像の生成
void createRandomNoizeImage(std::vector<ColorRGBA>& img, Game* game)
{
	// ColorRGBAのRGBカラー成分を0～255の一様乱数で設定
	for (ColorRGBA& color : img)
	{
		color.r = (unsigned char)game->getRand(0, 255);
		color.g = (unsigned char)game->getRand(0, 255);
		color.b = (unsigned char)game->getRand(0, 255);
		color.a = 255;
	}
}

XMMATRIX calcSpriteMatrix(float width, float height)
{
	// スプライト座標変換行列

	XMMATRIX mat = XMMatrixIdentity();
	mat.r[0].m128_f32[0] = 2.0f / width;
	mat.r[1].m128_f32[1] = -2.0f / height;
	mat.r[3].m128_f32[0] = -1.0f;
	mat.r[3].m128_f32[1] = 1.0f;

	return mat;
}

XMMATRIX calcSpriteModelMatrix(XMFLOAT2 scale,
	XMFLOAT2 pos, XMFLOAT2 offset, float theta)
{
	// スプライトモデル変換行列

	float t = XMConvertToRadians(theta);
	float s = XMScalarSin(t);
	float c = XMScalarCos(t);
	XMMATRIX mat = XMMatrixIdentity();
	mat.r[0].m128_f32[0] = scale.x * c;
	mat.r[0].m128_f32[1] = scale.x * s;
	mat.r[1].m128_f32[0] = scale.y * s;
	mat.r[1].m128_f32[1] = -scale.y * c;
	mat.r[3].m128_f32[0] = pos.x - offset.x * c + offset.y * s;
	mat.r[3].m128_f32[1] = pos.y - offset.x * s - offset.y * c;

	return mat;
}

XMMATRIX calcSpriteUVMatrix(const XMFLOAT2& p1, float uw, float uh,
	float imgW, float imgH)
{
	// スプライトのuv座標変換行列

	XMMATRIX mat = XMMatrixIdentity();
	float iw = 1.0f / imgW;
	float ih = 1.0f / imgH;
	mat.r[0].m128_f32[0] = uw * iw;
	mat.r[1].m128_f32[1] = uh * ih;
	mat.r[3].m128_f32[0] = p1.x * iw;
	mat.r[3].m128_f32[1] = p1.y * ih;

	return mat;
}

XMMATRIX calcSpriteUVMatrix(const XMFLOAT2& p1, const XMFLOAT2& p2,
	float imgW, float imgH)
{
	return calcSpriteUVMatrix(p1, p2.x - p1.x, p2.y - p1.y, imgW, imgH);
}

// 矩形と矩形の衝突判定
bool detectBoxCollision(Box& b1, Box& b2)
{
	XMFLOAT2 p1min = b1.pos;
	XMFLOAT2 p1max = b1.pos + XMFLOAT2(b1.width, b1.height);
	XMFLOAT2 p2min = b2.pos;
	XMFLOAT2 p2max = b2.pos + XMFLOAT2(b2.width, b2.height);

	if (p1max.x < p2min.x) return false;
	if (p2max.x < p1min.x) return false;
	if (p1max.y < p2min.y) return false;
	if (p2max.y < p1min.y) return false;
	return true;
}

// 矩形と円の衝突判定
bool detectBoxToCircleCollision(Box& b, Circle& c)
{
	XMFLOAT2 bmin = b.pos;
	XMFLOAT2 bmax = b.pos + XMFLOAT2(b.width, b.height);
	XMFLOAT2 s;
	s.x = (c.pos.x > bmax.x) ? bmax.x
		: (bmin.x > c.pos.x) ? bmin.x : c.pos.x;
	s.y = (c.pos.y > bmax.y) ? bmax.y
		: (bmin.y > c.pos.y) ? bmin.y : c.pos.y;

	return lengthSq(s - c.pos) <= (c.radius * c.radius);
}

// 文字コードのインデックス対応
bool FontIndex::isInRange(wchar_t c)
{
	return (start <= c) && (c <= end);
}