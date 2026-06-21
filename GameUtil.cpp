#include "GameUtil.h"

// 円と円の衝突判定
bool detectCircleCollision(Circle& c1, Circle& c2)
{
	Vector2d d = c2.pos - c1.pos;
	float th = c1.radius + c2.radius;
	if (lengthSq(d) < th * th) return true;
	return false;
}