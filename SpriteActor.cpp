#include "SpriteActor.h"
#include "Game.h"
#include "Scene.h"

SpriteActor::SpriteActor(Scene* scene, const std::wstring& filePath,
	const Vector2d& pos, const Vector2d& vel, bool centerFlag)
	: Actor2d(scene, pos, vel)
	, m_imgPath(filePath)
	, m_enabled(false)
	, m_offset(ZeroVec2d)
{
	int w, h;
	m_enabled = m_scene->getGame()->loadSprite(m_imgPath, w, h);
	if (!m_enabled) return;

	if (centerFlag)
	{
		m_offset = Vector2d(-0.5f * w, -0.5f * h);
	}
}

SpriteActor::~SpriteActor()
{
}

void SpriteActor::update(float deltaTime)
{
	m_pos += m_vel * deltaTime;
}

void SpriteActor::draw()
{
	m_scene->getGame()->drawSprite(m_imgPath, m_pos, m_offset);
}
