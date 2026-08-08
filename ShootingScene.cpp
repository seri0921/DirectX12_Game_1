#include "ShootingScene.h"
#include "Game.h"
#include <exception>
#include "Renderer.h"

ShootingScene::ShootingScene(Game* game)
	: Scene(game)
{
	{
		// 背景ループテクスチャ
		XMFLOAT2 spriteSize =
			XMFLOAT2((float)m_game->getWidth(), (float)m_game->getHeight());
		XMFLOAT2 uvSize = XMFLOAT2(320.0f, 240.0f);
		m_back = std::make_unique<SpriteActor>(this, L"src\\back-s03b.png",
			Renderer::Shader2DLoopLinear,
			ZeroVec2d, ZeroVec2d, &spriteSize, &uvSize, Ones2d,
			0.0f, 0.0f, ZeroVec2d, XMFLOAT2(30.0f, 0.0f));
		if (!m_back->isEnabled()) throw std::exception();
	}

	{
		// プレイヤーを生成
		std::vector<UINT> indices{ 7,6,7,8 };
		std::vector<std::vector<UINT>> anims{ indices };
		m_player = std::make_unique<PlayerActor>(this, L"src\\pipo-airship01.png",
			anims, 16.0f, 0, 0.2f, 3, 4, Renderer::Shader2DAlphaLoopPoint,
			XMFLOAT2(60.0f, 240.0f));
		if (!m_player->isEnabled()) throw std::exception();
	}

	m_isRunning = true;
}

ShootingScene::~ShootingScene()
{
	releaseActors(m_playerBullets);
	releaseActors(m_playerBulletsTemp);
}

void ShootingScene::update(float deltaTime)
{
	// 更新処理
	m_back->update(deltaTime);
	m_player->update(deltaTime);
	updateActors(m_playerBullets, deltaTime);
	updateActors(m_actors, deltaTime);

	// 削除処理、追加処理
	removeActors(m_actors);
	moveIntoActors(m_actorsTemp, m_actors);
	removeActors(m_playerBullets);
	moveIntoActors(m_playerBulletsTemp, m_playerBullets);
}

void ShootingScene::draw()
{
	m_back->draw();
	m_player->draw();
	drawActors(m_playerBullets);
	drawActors(m_actors);
}