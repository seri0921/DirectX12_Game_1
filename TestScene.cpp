#include "TestScene.h"
#include "Game.h"
#include <exception>

TestScene::TestScene(Game* game)
	: Scene(game)
{
	XMFLOAT2 s = 200.0f * Ones2d;
	m_sprites[0] = std::make_unique<SpriteActor>(this, L"src\\oreka.png",
		ZeroVec2d, ZeroVec2d, &s);
	if (!m_sprites[0]->isEnebled()) throw std::exception();

	m_sprites[1] = std::make_unique<SpriteActor>(this, L"src\\oreka2.png",
		50.0f * Ones2d, ZeroVec2d, &s);
	if (!m_sprites[1]->isEnebled()) throw std::exception();

	m_sprites[2] = std::make_unique<SpriteActor>(this, L"src\\oreka3.png",
		100.0f * Ones2d, ZeroVec2d, &s);
	if (!m_sprites[2]->isEnebled()) throw std::exception();

	m_sprites[3] = std::make_unique<SpriteActor>(this, L"src\\oreka4.png",
		150.0f * Ones2d, ZeroVec2d, &s);
	if (!m_sprites[3]->isEnebled()) throw std::exception();

	m_sprites[4] = std::make_unique<SpriteActor>(this, L"src\\oreka5.png",
		200.0f * Ones2d, ZeroVec2d, &s);
	if (!m_sprites[4]->isEnebled()) throw std::exception();


	m_isRunning = true;
}

TestScene::~TestScene()
{
}

void TestScene::update(float deltaTime)
{
}

void TestScene::draw()
{
	for (int i = 0; i < 5; ++i)
	{
		m_sprites[i]->draw();
	}
}
