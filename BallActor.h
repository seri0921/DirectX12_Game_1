#pragma once

#include "Actor2d.h"

class BallActor : public Actor2d
{
public:
	BallActor(class Scene* scene, float radius, COLORREF color,
		const Vector2d& pos = ZeroVec2d, const Vector2d& vel = ZeroVec2d);
	~BallActor();

	void update(float deltaTime) override;
	void draw() override;

	Circle getCircle() const { return Circle(m_pos, m_radius); }

private:
	float m_radius;
	COLORREF m_color;

};

