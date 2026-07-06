#include "PaddleActor.h"
#include "Scene.h"
#include "Game.h"
#include "BreakoutScene.h"

PaddleActor::PaddleActor(Scene* scene, float width, float height,
	COLORREF fillColor, COLORREF penColor,
	const Vector2d& pos, const Vector2d& vel)
	: Actor2d(scene, pos, vel)
	, m_width(width)
	, m_height(height)
	, m_fillColor(fillColor)
	, m_penColor(penColor)
{
	m_maxSpeed = 256.0f;
}

PaddleActor::~PaddleActor()
{
}

void PaddleActor::update(float deltaTime)
{
	{
		BreakoutScene* scene = (BreakoutScene*)m_scene;
		State state = scene->getState();
		if (state == State::Clear || state == State::Over) return;
	}

	Game* game = m_scene->getGame();
	int width = game->getWidth();
	const Keyboard& keyboard = game->getKeyboard();

	int vx = 0;
	if (keyboard.isDown(VK_RIGHT)) vx += 1;
	if (keyboard.isDown(VK_LEFT))  vx -= 1;
	m_vel = UnitVecX2d * (m_maxSpeed * vx);
	m_pos += m_vel * deltaTime;

	float halfWidth = m_width * 0.5;
	if (m_pos.x - halfWidth < 0.0f) m_pos.x = halfWidth;
	if (m_pos.x + halfWidth > width) m_pos.x = width - halfWidth;
}

void PaddleActor::draw()
{
	{
		BreakoutScene* scene = (BreakoutScene*)m_scene;
		State state = scene->getState();
		if (state == State::Clear || state == State::Over) return;
	}

	Vector2d p = m_pos - Vector2d(0.5f * m_width, 0.5 * m_height);
	m_scene->getGame()->drawRect(Box(p, m_width, m_height),
		m_fillColor, m_penColor);
}