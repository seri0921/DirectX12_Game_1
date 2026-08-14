#include "PlayerActor.h"
#include "Scene.h"
#include "Game.h"
#include "ShootingScene.h"
#include "PlayerBulletActor.h"
#include <exception>
#include "EffectActor.h"

PlayerActor::PlayerActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices, float radius,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize, const XMFLOAT2& scale,
	float angle, float angleVel, bool centerFlag, bool ddsFlag)
	: CharaActor(scene, filePath, indices, radius, lane, interval, wNum, hNum,
		shaderIndex, pos, vel, spriteSize, scale, angle, angleVel, centerFlag, ddsFlag)
	, m_waitTime(0.0f)
	, m_waitInterval(0.2f)
	, m_HP(150)
	, m_invincibleTime(0.0f)
	, m_invincibleCount(0)
	, m_invincibleMode(false)
	, m_state(PlayerState::Playing)
	, m_mouseMode(true)
{
	m_maxSpeed = 200.0f;

	{
		// 墜落時の煙を生成
		std::vector<UINT> indices{ 4,3,4,5 };
		std::vector<std::vector<UINT>> anims{ indices };
		XMFLOAT2 size(88.0f, 64.0f);
		m_smoke = std::make_unique<BlockAnimActor>(m_scene,
			L"src\\pipo-charachip_smoke02a-s.png", anims, 0, 0.5f,
			3, 4, Renderer::Shader2DAlphaLoopPoint, ZeroVec2d,
			ZeroVec2d, &size, Ones2d, 0.0f, 0.0f, true);
		if (!m_smoke->isEnabled()) throw std::exception();
	}
}

PlayerActor::~PlayerActor()
{
}

void PlayerActor::update(float deltaTime)
{
	if (m_dead) return;

	switch (m_state)
	{
	case PlayerState::Playing:
		updatePlaying(deltaTime);
		break;

	case PlayerState::Dying:
		updateDying(deltaTime);
		break;
	}

}

void PlayerActor::draw()
{
	if (m_dead) return;

	if (m_state == PlayerState::Dying)
	{
		m_smoke->setPos(m_pos - XMFLOAT2(14.0f, 28.0f));
		m_smoke->draw();
	}
	SpriteActor::draw();
}

void PlayerActor::damage(float dm)
{
	if (m_invincibleMode) return;

	m_HP -= 50;
	if (m_HP <= 0)
	{
		m_state = PlayerState::Dying;
		m_vel = XMFLOAT2(50.0, 200.0f);
		std::vector<UINT> indices{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		std::vector<std::vector<UINT>> anims{ indices };
		XMFLOAT2 size(32.0f, 32.0f);
		createMultipleEffects(m_scene, L"src\\pipo-mapeffect005.png",
			anims, 0, 0.06f, 10, 1, Renderer::Shader2DAlphaLoopPoint,
			5, 0.08f, 20.0f, 16.0f, 16.0f, m_pos, &size, Ones2d, true);
	}
	else
	{
		m_invincibleTime = 3.0f;
		m_invincibleCount = 0;
		m_invincibleMode = true;
		setColorVector(ColorRed, 0.5f);
	}
}

void PlayerActor::updatePlaying(float deltaTime)
{
	const Keyboard& keyboard = m_scene->getGame()->getKeyboard();
	const Mouse& mouse = m_scene->getGame()->getMouse();
	float width = (float)m_scene->getGame()->getWidth();
	float height = (float)m_scene->getGame()->getHeight();

	int kx = 0, ky = 0;
	if (keyboard.isDown(VK_RIGHT)) kx++;
	if (keyboard.isDown(VK_LEFT)) kx--;
	if (keyboard.isDown(VK_DOWN)) ky++;
	if (keyboard.isDown(VK_UP)) ky--;
	if (m_mouseMode)
	{
		static float K = 400.0f;
		static float C = 2.0f * std::sqrt(K);
		m_vel += -deltaTime * (K * (m_pos - mouse.getPos()) + C * m_vel);
		if (length(m_vel) > m_maxSpeed)
		{
			m_vel = m_maxSpeed * normalize(m_vel);
		}
	}
	else
	{
		m_vel = normalize(XMFLOAT2((float)kx, (float)ky)) * m_maxSpeed;

	}
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
	if (m_waitTime <= 0.0f && (keyboard.isDown('Z') || mouse.isDown(VK_LBUTTON)))
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

		SoundSystem* soundSystem = m_scene->getGame()->getSoundSystem();
		soundSystem->setSoundEffect(L"src\\maou_se_battle_gun05.mp3");
	}

	if (m_invincibleTime > 0.0f)
	{
		m_invincibleTime -= deltaTime;
		m_invincibleCount += 1;
		if (m_invincibleTime <= 0.0f)
		{
			m_invincibleTime = 0.0f;
			m_visible = true;
			m_invincibleMode = false;
			setColorVector(ColorWhite, 1.0f);
		}
		else
		{
			if (m_invincibleCount % 15 == 0)
			{
				m_visible = !m_visible;
			}
		}
	}
}

void PlayerActor::updateDying(float deltaTime)
{
	simulate(deltaTime);
	m_smoke->update(deltaTime);

	float width = (float)m_scene->getGame()->getWidth();
	float height = (float)m_scene->getGame()->getHeight();
	if (((m_pos.x + m_radius < 0.0f) || (m_pos.x - m_radius > width))
		|| ((m_pos.y + m_radius < 0.0f) || (m_pos.y + m_radius > height)))
	{
		setDead();
		m_state = PlayerState::Playing;
	}
}