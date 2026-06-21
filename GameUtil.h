#pragma once

#include "GMath.h"

// 円の表現
struct Circle
{
	Vector2d pos;
	float radius;

	Circle() : pos(ZeroVec2d), radius(1.0f) {}
	Circle(Vector2d p, float r) : pos(p), radius(r) {}
};

bool detectCircleCollision(Circle& c1, Circle& c2);

