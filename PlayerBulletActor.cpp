#include "PlayerBulletActor.h"
#include "Game.h"
#include "Scene.h"


PlayerBulletActor::PlayerBulletActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices, float radius,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize,
	const XMFLOAT2& scale, float angle, float angleVel, bool centerFlag, bool ddsFlag)
	: CharaActor(scene, filePath, indices, radius, lane, interval, wNum, hNum,
		shaderIndex, pos, vel, spriteSize, scale, angle, angleVel, centerFlag, ddsFlag)
{
}

PlayerBulletActor::~PlayerBulletActor()
{
}

void PlayerBulletActor::update(float deltaTime)
{
	if (m_dead) return;

	simulate(deltaTime);
	updateAnim(deltaTime);

	float width = (float)(m_scene->getGame()->getWidth());
	float height = (float)m_scene->getGame()->getHeight();
	if (((m_pos.x + m_radius < 0.0f) || (m_pos.x - m_radius > width))
		|| ((m_pos.y + m_radius < 0.0f) || (m_pos.y - m_radius > height)))
	{
		m_lifeTime -= deltaTime;
		if (m_lifeTime < 0.0f) m_dead = true;
	}
}