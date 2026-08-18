#include "ShootingTitleScene.h"
#include "Game.h"
#include <exception>
#include "Renderer.h"
#include "ShootingScene.h"

ShootingTitleScene::ShootingTitleScene(Game* game)
	: Scene(game)
	, m_selectIndex(0)
{
	// 背景色
	m_game->getRenderer()->setBackColor(ColorBlack);

	{
		// 文字列生成
		FontData fd;
		m_game->getRenderer()->getFontData(Renderer::BaseFont, fd);

		m_startUI = std::make_unique<StringActor>(this, L"Start",
			fd, 24.0f * Ones2d, Renderer::Shader2DAddLoopPoint,
			XMFLOAT2(290.0f, 200.0f), ZeroVec2d, Ones2d, 0.0f, 0.0f, 0.6f, -1);
		if (!m_startUI->isEnabled()) throw std::exception();
		m_startUI->setColorVector(ColorCyan, 1.0f);

		m_quitUI = std::make_unique<StringActor>(this, L"Quit",
			fd, 24.0f * Ones2d, Renderer::Shader2DAddLoopPoint,
			XMFLOAT2(300.0f, 240.0f), ZeroVec2d, Ones2d, 0.0f, 0.0f, 0.6f, -1);
		if (!m_quitUI->isEnabled()) throw std::exception();
		m_quitUI->setColorVector(ColorWhite, 1.0f);
	}

	m_isRunning = true;

}

ShootingTitleScene::~ShootingTitleScene()
{
}

SceneState ShootingTitleScene::update(float deltaTime, Scene** newScene)
{
	// 入力対応
	const Keyboard& keyboard = m_game->getKeyboard();
	if (keyboard.isPressed(VK_RETURN))
	{
		switch (m_selectIndex)
		{
		case 0:
			*newScene = new ShootingScene(m_game);
			return SceneState::Replace;

		case 1:
			return SceneState::Pop;
		}
	}
	else if (keyboard.isPressed(VK_DOWN)
		|| keyboard.isPressed(VK_UP))
	{
		m_selectIndex = (m_selectIndex + 1) % 2;
		switch (m_selectIndex)
		{
		case 0:
			m_startUI->setColorVector(ColorCyan, 1.0f);
			m_quitUI->setColorVector(ColorWhite, 1.0f);
			break;

		case 1:
			m_startUI->setColorVector(ColorWhite, 1.0f);
			m_quitUI->setColorVector(ColorCyan, 1.0f);
			break;
		}
	}

	// 更新処理
	m_startUI->update(deltaTime);
	m_quitUI->update(deltaTime);
	updateActors(m_actors, deltaTime);

	// 削除処理、追加処理
	removeActors(m_actors);
	moveIntoActors(m_actorsTemp, m_actors);

	return SceneState::Continue;

}

void ShootingTitleScene::draw()
{
	m_startUI->draw();
	m_quitUI->draw();
	drawActors(m_actors);
}