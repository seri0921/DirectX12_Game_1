#include "EnemyActor.h"
#include "Game.h"
#include "Scene.h"
#include "ShootingScene.h"
#include "PlayerActor.h"
#include "EnemyBulletActor.h"

EnemyActor::EnemyActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices, float radius,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize,
	const XMFLOAT2& scale, float angle, float angleVel, bool centerFlag, bool ddsFlag)
	: CharaActor(scene, filePath, indices, radius, lane, interval, wNum, hNum,
		shaderIndex, pos, vel, spriteSize, scale, angle, angleVel, centerFlag, ddsFlag)
{
	m_waitTime = (float)scene->getGame()->getRand(2, 4);
}

EnemyActor::~EnemyActor()
{
}

void EnemyActor::update(float deltaTime)
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
		return;
	}

	ShootingScene* scene = (ShootingScene*)m_scene;
	PlayerActor* player = scene->getPlayer();

	if (!player->isDead())
	{
		Circle p = player->getCircle();
		Circle e = getCircle();
		if (detectCircleCollision(p, e))
		{
			setDead();
			player->setDead();
			return;
		}
	}

	if (m_waitTime > 0.0f) m_waitTime -= deltaTime;

	if (m_waitTime <= 0.0f && !player->isDead())
	{
		XMFLOAT2 v = 200.0f * normalize(player->getPos() - m_pos);
		std::vector<UINT> indices{ 7, 6, 7, 8 };
		std::vector<std::vector<UINT>> anims{ indices };
		EnemyBulletActor* bullet = new EnemyBulletActor(m_scene,
			L"src\\pipo-hikarimono007.png", anims, 16.0f, 0, 1.0f, 3, 4,
			Renderer::Shader2DAddLoopPoint, m_pos, v);
		bullet->setLifeTime(0.1f);
		scene->addEnemyBullet(bullet);
		m_waitTime = (float)scene->getGame()->getRand(2, 4);
	}
}