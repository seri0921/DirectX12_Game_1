#include "PlayerActor.h"
#include "Scene.h"
#include "Game.h"
#include "ShootingScene.h"
#include "PlayerBulletActor.h"
#include <exception>

PlayerActor::PlayerActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices, float radius,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize, const XMFLOAT2& scale,
	float angle, float angleVel, bool centerFlag, bool ddsFlag)
	: CharaActor(scene, filePath, indices, radius, lane, interval, wNum, hNum,
		shaderIndex, pos, vel, spriteSize, scale, angle, angleVel, centerFlag, ddsFlag)
	, m_waitTime(0.0f)
	, m_waitInterval(0.2f)
{
	m_maxSpeed = 200.0f;
}

PlayerActor::~PlayerActor()
{
}

void PlayerActor::update(float deltaTime)
{
	if (m_dead) return;

	const Keyboard& keyboard = m_scene->getGame()->getKeyboard();
	float width = (float)m_scene->getGame()->getWidth();
	float height = (float)m_scene->getGame()->getHeight();

	int kx = 0, ky = 0;
	if (keyboard.isDown(VK_RIGHT)) kx++;
	if (keyboard.isDown(VK_LEFT)) kx--;
	if (keyboard.isDown(VK_DOWN)) ky++;
	if (keyboard.isDown(VK_UP)) ky--;
	m_vel = normalize(XMFLOAT2((float)kx, (float)ky)) * m_maxSpeed;
	simulate(deltaTime);
	BlockAnimActor::updateAnim(deltaTime);

	if (m_pos.x - m_radius < 0.0f) m_pos.x = m_radius;
	if (m_pos.x + m_radius > width) m_pos.x = width - m_radius;
	if (m_pos.y - m_radius < 0.0f) m_pos.y = m_radius;
	if (m_pos.y + m_radius > height) m_pos.y = height - m_radius;

	if (m_waitTime > 0.0f)
	{
		m_waitTime -= deltaTime;
		if (m_waitTime < 0.0f) m_waitTime = 0.0f;
	}
	if (m_waitTime <= 0.0f && keyboard.isDown('Z'))
	{
		std::vector<UINT> indices{ 1,0,1,2 };
		std::vector<std::vector<UINT>> anims{ indices };
		PlayerBulletActor* bullet = new PlayerBulletActor(m_scene,
			L"src\\pipo-hikarimono007.png", anims, 16.0f, 0, 1.0f, 3, 4,
			Renderer::Shader2DAlphaLoopPoint, m_pos + UnitVecX2d * 20.0f,
			UnitVecX2d * 300.0f);
		if (!bullet->isEnabled()) throw std::exception();
		bullet->setLifeTime(0.1f);
		ShootingScene* scene = (ShootingScene*)m_scene;
		scene->addPlayerBullet(bullet);
		m_waitTime = m_waitInterval;
	}
}

void PlayerActor::draw()
{
	SpriteActor::draw();
}