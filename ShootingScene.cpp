#include "ShootingScene.h"
#include "Game.h"
#include <exception>
#include "Renderer.h"
#include "EnemyActor.h"
#include "SoundSystem.h"

ShootingScene::ShootingScene(Game* game)
	: Scene(game)
{
	{
		// テクスチャの読み込み処理
		Renderer* ren = m_game->getRenderer();

		// 背景画像
		ImageData imgData = ren->allocateShaderResource(L"src\\back-s03b.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

		// 飛空艇
		imgData = ren->allocateShaderResource(L"src\\pipo-airship01.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

		// かぼちゃ（エネミー）
		imgData = ren->allocateShaderResource(L"src\\kabocha.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

		// 煙
		imgData = ren->allocateShaderResource(L"src\\pipo-charachip_smoke02a-s.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

		// 光の弾
		imgData = ren->allocateShaderResource(L"src\\pipo-hikarimono007.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

		// 爆発
		imgData = ren->allocateShaderResource(L"src\\pipo-mapeffect005.png");
		if (imgData.imgIndex == -1) throw std::exception();
		m_imgLoadData.push_back(imgData);

	}
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

	{
		// サウンドの読み込み
		SoundSystem* soundSystem = m_game->getSoundSystem();
		SoundInfo sinfo;

		sinfo = soundSystem->loadSoundFile(L"src\\maou_bgm_fantasy02.mp3");
		if (sinfo.index == -1) throw std::exception();
		m_soundLoadData.push_back(sinfo);
		sinfo = soundSystem->loadSoundFile(L"src\\maou_se_battle_gun05.mp3");
		if (sinfo.index == -1) throw std::exception();
		m_soundLoadData.push_back(sinfo);
		sinfo = soundSystem->loadSoundFile(L"src\\maou_se_battle_explosion06.mp3");
		if (sinfo.index == -1) throw std::exception();
		m_soundLoadData.push_back(sinfo);

		soundSystem->setBGMVolume(0.5f);
		soundSystem->setSEVolume(0.5f);
		soundSystem->setBGM(L"src\\maou_bgm_fantasy02.mp3", true);

	}

	m_isRunning = true;
}

ShootingScene::~ShootingScene()
{
	releaseActors(m_playerBullets);
	releaseActors(m_playerBulletsTemp);
	releaseActors(m_enemyBullets);
	releaseActors(m_enemyBulletsTemp);
	releaseActors(m_enemies);
	releaseActors(m_enemiesTemp);

	SoundSystem* soundSystem = m_game->getSoundSystem();
	soundSystem->stopBGM();
}

void ShootingScene::update(float deltaTime)
{
	// 敵の生成処理
	if (m_game->getRand() < 0.05)
	{
		float y = 30.0f + 420.0f * (float)m_game->getRand();
		std::vector<UINT> indices{ 4, 3, 4, 5 };
		std::vector<std::vector<UINT>> anims{ indices };
		EnemyActor* enemy = new EnemyActor(this,
			L"src\\kabocha.png", anims, 16.0f, 0, 0.2f, 3, 4,
			Renderer::Shader2DAlphaLoopPoint,
			XMFLOAT2((float)m_game->getWidth() + 16.0f, y), -200.0f * UnitVecX2d);
		if (!enemy->isEnabled()) throw std::exception();
		enemy->setLifeTime(2.0f);
		addEnemy(enemy);

	}
	// 更新処理
	m_back->update(deltaTime);
	m_player->update(deltaTime);
	updateActors(m_playerBullets, deltaTime);
	updateActors(m_enemyBullets, deltaTime);
	updateActors(m_enemies, deltaTime);
	updateActors(m_actors, deltaTime);

	// 削除処理、追加処理
	removeActors(m_actors);
	moveIntoActors(m_actorsTemp, m_actors);
	removeActors(m_playerBullets);
	moveIntoActors(m_playerBulletsTemp, m_playerBullets);
	removeActors(m_enemyBullets);
	moveIntoActors(m_enemyBulletsTemp, m_enemyBullets);
	removeActors(m_enemies);
	moveIntoActors(m_enemiesTemp, m_enemies);
}

void ShootingScene::draw()
{
	m_back->draw();
	m_player->draw();
	drawActors(m_enemies);
	drawActors(m_playerBullets);
	drawActors(m_enemyBullets);
	drawActors(m_actors);
}