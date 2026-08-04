#include "GMath.h"
#include <cfloat>

const XMFLOAT2 ZeroVec2d(0.0f, 0.0f);
const XMFLOAT2 Ones2d(1.0f, 1.0f);
const XMFLOAT2 UnitVecX2d(1.0f, 0.0f);
const XMFLOAT2 UnitVecY2d(0.0f, 1.0f);

const XMFLOAT3 ZeroVec3d(0.0f, 0.0f, 0.0f);
const XMFLOAT3 Ones3d(1.0f, 1.0f, 1.0f);
const XMFLOAT3 UnitVecX3d(1.0f, 0.0f, 0.0f);
const XMFLOAT3 UnitVecY3d(0.0f, 1.0f, 0.0f);
const XMFLOAT3 UnitVecZ3d(0.0f, 0.0f, 1.0f);

const float PI = 3.14159265f;

XMFLOAT2 operator+ (const XMFLOAT2& a, const XMFLOAT2& b)
{
	return XMFLOAT2(a.x + b.x, a.y + b.y);
}

XMFLOAT2 operator- (const XMFLOAT2& a, const XMFLOAT2& b)
{
	return XMFLOAT2(a.x - b.x, a.y - b.y);
}

XMFLOAT2 operator* (const XMFLOAT2& a, float b)
{
	return XMFLOAT2(a.x * b, a.y * b);
}

XMFLOAT2 operator* (float a, const XMFLOAT2& b)
{
	return XMFLOAT2(a * b.x, a * b.y);
}

void operator+= (XMFLOAT2& a, const XMFLOAT2& b)
{
	a.x += b.x;
	a.y += b.y;
}

void operator-= (XMFLOAT2& a, const XMFLOAT2& b)
{
	a.x -= b.x;
	a.y -= b.y;
}

void operator*= (XMFLOAT2& a, float b)
{
	a.x *= b;
	a.y *= b;
}

float length(const XMFLOAT2& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

float lengthSq(const XMFLOAT2& v)
{
	return v.x * v.x + v.y * v.y;
}

XMFLOAT2 normalize(const XMFLOAT2& v)
{
	float len = length(v);
	if (len < FLT_EPSILON) return XMFLOAT2(0.0f, 0.0f);

	float inLen = 1.0f / len;
	return inLen * v;
}

XMFLOAT2 unitVecAngle(float theta)
{
	return XMFLOAT2(std::cos(theta), std::sin(theta));
}

float dot(const XMFLOAT2& a, const XMFLOAT2& b)
{
	return a.x * b.x + a.y * b.y;
}

float cross(const XMFLOAT2& a, const XMFLOAT2& b)
{
	return a.x * b.y - a.y * b.x;
}

XMFLOAT3 operator+ (const XMFLOAT3& a, const XMFLOAT3& b)
{
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT3 operator- (const XMFLOAT3& a, const XMFLOAT3& b)
{
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

XMFLOAT3 operator* (const XMFLOAT3& a, float b)
{
	return XMFLOAT3(a.x * b, a.y * b, a.z * b);
}

XMFLOAT3 operator* (float a, const XMFLOAT3& b)
{
	return XMFLOAT3(a * b.x, a * b.y, a * b.z);
}

void operator+= (XMFLOAT3& a, const XMFLOAT3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

void operator-= (XMFLOAT3& a, const XMFLOAT3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

void operator*= (XMFLOAT3& a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
}
