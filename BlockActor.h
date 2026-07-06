#pragma once

#include "Actor2d.h"

class BlockActor : public Actor2d
{
public:
	BlockActor(class Scene* secene, float width, float height,
		COLORREF fillColor, COLORREF penColor,
		const Vector2d& pos = ZeroVec2d,
		const Vector2d& vel = ZeroVec2d);
	~BlockActor();

	void update(float deltaTime) override;
	void draw() override;

	Box getBox() const {
		return Box(m_pos - Vector2d(0.5 * m_width, 0.5f * m_height),
			m_width, m_height);
	}
private:
	float m_width;
	float m_height;
	COLORREF m_fillColor;
	COLORREF m_penColor;
};


