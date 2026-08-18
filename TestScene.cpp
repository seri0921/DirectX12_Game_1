#include "TestScene.h"
#include "Game.h"
#include <exception>

TestScene::TestScene(Game* game)
	: Scene(game)
{
	XMFLOAT2 spriteSize =
		XMFLOAT2((float)m_game->getWidth(), (float)m_game->getHeight());
	XMFLOAT2 uvSize = XMFLOAT2(320.0f, 240.0f);

	m_back = std::make_unique<SpriteActor>(this, L"src\\back-s03b.png",
		Renderer::Shader2DLoopLinear,
		ZeroVec2d, ZeroVec2d, &spriteSize, &uvSize, Ones2d,
		0.0f, 0.0f, ZeroVec2d, XMFLOAT2(30.0f, 0.0f));
	if (!m_back->isEnabled()) throw std::exception();

	std::vector<UINT> indicesDown{ 1, 0, 1, 2 };
	std::vector<UINT> indicesLeft{ 4, 3, 4, 5 };
	std::vector<UINT> indicesRight{ 7, 6, 7, 8 };
	std::vector<UINT> indicesUp{ 10, 9, 10, 11 };
	std::vector<std::vector<UINT>> anims{ indicesDown, indicesLeft, indicesRight, indicesUp };
	m_ship = std::make_unique<BlockAnimActor>(this, L"src\\pipo-airship01.png",
		anims, 0, 0.2f, 3, 4, Renderer::Shader2DAlphaLoopPoint,
		XMFLOAT2(320.0f, 240.0f), ZeroVec2d, nullptr, Ones2d, 0.0f, 0.0f, true);
	if (!m_ship->isEnabled()) throw std::exception();

	for (int i = 0; i < 20; ++i)
	{
		m_lightShip[i] = std::make_unique<BlockSpriteActor>(this, L"src\\pipo-airship01.png",
			3, 4, 6, Renderer::Shader2DAddLoopPoint,
			XMFLOAT2(320.0f, 200.0f) + (10.0f * i) * UnitVecX2d, ZeroVec2d,
			nullptr, Ones2d, 0.0f, 0.0f, true);
	}

	{
		XMFLOAT2 s = XMFLOAT2(100.0f, 100.0f);
		m_unicolor = std::make_unique<SpriteActor>(this,
			L"src\\oreka.png", Renderer::Shader2DAlphaLoopPoint,
			XMFLOAT2(100.0f, 100.0f), ZeroVec2d, &s);
		if (!m_unicolor->isEnabled()) throw std::exception();
		m_unicolor->setColorVector(Ones3d, 0.0f);
	}

	m_isRunning = true;
}

TestScene::~TestScene()
{
}

SceneState TestScene::update(float deltaTime, Scene** newScene)
{
	const Keyboard& keyboard = m_game->getKeyboard();
	
	if (keyboard.isPressed(VK_DOWN)) m_ship->changeLane(0);
	if (keyboard.isPressed(VK_LEFT)) m_ship->changeLane(1);
	if (keyboard.isPressed(VK_RIGHT)) m_ship->changeLane(2);
	if (keyboard.isPressed(VK_UP)) m_ship->changeLane(3);

	m_back->update(deltaTime);
	m_ship->update(deltaTime);

	static float alpha = 0.0f;
	alpha += 0.001;
	alpha = (alpha > 1.0f) ? 1.0f : alpha;
	m_unicolor->setColorVector(Ones3d, alpha);

	return SceneState::Continue;
}

void TestScene::draw()
{
	m_back->draw();
	m_ship->draw();

	for (int i = 0; i < 20; ++i)
	{
		m_lightShip[i]->draw();
		m_unicolor->draw();
	}
}
