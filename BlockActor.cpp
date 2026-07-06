#include "BlockActor.h"
#include "Scene.h"
#include "Game.h"

BlockActor::BlockActor(Scene* scene, float width, float height,
	COLORREF fillColor, COLORREF penColor,
	const Vector2d& pos, const Vector2d& vel)
	: Actor2d(scene, pos, vel)
	, m_width(width)
	, m_height(height)
	, m_fillColor(fillColor)
	, m_penColor(penColor)
{
}

BlockActor::~BlockActor()
{
}

void BlockActor::update(float deltaTime)
{
}

void BlockActor::draw()
{
	Vector2d p = m_pos - Vector2d(0.5f * m_width, 0.5 * m_height);
	m_scene->getGame()->drawRect(Box(p, m_width, m_height),
		m_fillColor, m_penColor);
}