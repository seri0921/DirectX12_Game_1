#pragma once

#include <cmath>

#include <DirectXMath.h>
using namespace DirectX;

XMFLOAT2 operator+ (const XMFLOAT2& a, const XMFLOAT2& b);
XMFLOAT2 operator- (const XMFLOAT2& a, const XMFLOAT2& b);
XMFLOAT2 operator* (const XMFLOAT2& a, float b);
XMFLOAT2 operator* (float a, const XMFLOAT2& b);
XMFLOAT2 operator* (const XMFLOAT2& a, const XMFLOAT2& b);

void operator+= (XMFLOAT2& a, const XMFLOAT2& b);
void operator-= (XMFLOAT2& a, const XMFLOAT2& b);
void operator*= (XMFLOAT2& a, float b);

float length(const XMFLOAT2& v);
float lengthSq(const XMFLOAT2& v);
XMFLOAT2 normalize(const XMFLOAT2& v);

XMFLOAT2 unitVecAngle(float theta);

float dot(const XMFLOAT2& a, const XMFLOAT2& b);
float cross(const XMFLOAT2& a, const XMFLOAT2& b);

XMFLOAT3 operator+ (const XMFLOAT3& a, const XMFLOAT3& b);
XMFLOAT3 operator- (const XMFLOAT3& a, const XMFLOAT3& b);
XMFLOAT3 operator* (const XMFLOAT3& a, float b);
XMFLOAT3 operator* (float a, const XMFLOAT3& b);

void operator+= (XMFLOAT3& a, const XMFLOAT3& b);
void operator-= (XMFLOAT3& a, const XMFLOAT3& b);
void operator*= (XMFLOAT3& a, float b);

extern const XMFLOAT2 ZeroVec2d;
extern const XMFLOAT2 Ones2d;
extern const XMFLOAT2 UnitVecX2d;
extern const XMFLOAT2 UnitVecY2d;

extern const XMFLOAT3 ZeroVec3d;
extern const XMFLOAT3 Ones3d;
extern const XMFLOAT3 UnitVecX3d;
extern const XMFLOAT3 UnitVecY3d;
extern const XMFLOAT3 UnitVecZ3d;


