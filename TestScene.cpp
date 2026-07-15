#include "TestScene.h"
#include "Game.h"
#include <exception>

TestScene::TestScene(Game* game)
	: Scene(game)
{
	m_colorAnim = std::make_unique<ColorAnimActor>(this,
		0.0f, 0.0f, 0.0f, 0.5f, 0.2f, -1.0f);
	m_isRunning = true;
}

TestScene::~TestScene()
{
}

void TestScene::update(float deltaTime)
{
	m_colorAnim->update(deltaTime);
}

void TestScene::draw()
{
}
