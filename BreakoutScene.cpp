#include "BreakoutScene.h"
#include "Game.h"
#include "BallActor.h"
#include "BlockActor.h"
#include <execution>

const float BreakoutScene::BallRadius = 10.0f;
const float BreakoutScene::BallAngle = -30.0f;
const float BreakoutScene::BallSpeed = 700.0f;
const COLORREF BreakoutScene::BallColor = RGB(255, 255, 255);
const float BreakoutScene::PaddleYPos = 600.0f;

BreakoutScene::BreakoutScene(Game* game)
	: Scene(game)
	, m_state(GameState::Ready)
{
	{
		// ボール生成
		BallActor* ball = new BallActor(this, BallRadius, BallColor,
			ZeroVec2d, unitVecAngle(deg2rad(BallAngle)) * BallSpeed);
		m_balls.push_back(ball);

		ball = new BallActor(this, 10.0f, RGB(255, 30, 255),
			Vector2d(400.0f, 520.0f), Vector2d(-80.0f, -180.0f));
		m_balls.push_back(ball);
	}

	{
		// パドル生成
		float xPos = m_game->getWidth() * 0.5f;
		m_paddle = std::make_unique<PaddleActor>(this, 120.0f, 30.0f,
			RGB(255, 30, 30), RGB(128, 128, 128), Vector2d(240.0f, 600.0f));
	}

	{
		// ブロック生成
		float bw = 50.0f, bh = 30.0f;
		int nx = 9, ny = 9;
		Vector2d base(15.0f, 20.0f);
		for (int  j = 0; j < ny; ++j)
		{
			for (int i = 0; i < nx; ++i)
			{
				float x = base.x + bw * (i + 0.5f);
				float y = base.y + bh * (j + 0.5f);
				BlockActor* block = new BlockActor(this, bw, bh,
					RGB(30, 255, 30), RGB(128, 128, 128), Vector2d(x, y));
				m_blocks.push_back(block);
			}
		}
	}

	{
		// 背景画像の読み込み
		m_backImage = std::make_unique<SpriteActor>(this, L"src\\st.bmp");
		//if (!m_backImage->isEnebled()) throw std::exception();
	}
	m_isRunning = true;
}



BreakoutScene::~BreakoutScene()
{
	releaseActors(m_balls);
	releaseActors(m_blocks);
}

void BreakoutScene::update(float deltaTime)
{
	const Keyboard& keyboard = m_game->getKeyboard();

	// 状態遷移
	if (m_state == GameState::Ready)
	{
		if (keyboard.isPressed(VK_SPACE)) m_state = GameState::Play;
	}
	else if (m_state == GameState::Play)
	{
		if (m_balls.size() == 0) m_state = GameState::Over;
		else if (m_blocks.size() == 0) m_state = GameState::Clear;
	}
	else if (m_state == GameState::Over)
	{
		if (keyboard.isPressed(VK_SPACE))
		{
			m_state = GameState::Ready;
			float xPos = m_game->getWidth() * 0.5f;
			m_paddle->setPos(Vector2d(xPos, PaddleYPos));
			BallActor* ball = new BallActor(this, BallRadius, BallColor,
				ZeroVec2d, unitVecAngle(deg2rad(BallAngle)) * BallSpeed);
			m_balls.push_back(ball);
		}
	}
	else if (m_state == GameState::Clear)
	{
		if (keyboard.isPressed(VK_SPACE))
		{
			m_isRunning = false;
			return;
		}
	}

	m_paddle->update(deltaTime);
	updateActors(m_balls, deltaTime);
	m_backImage->update(deltaTime);

	removeActors(m_balls);
	removeActors(m_blocks);
}

void BreakoutScene::draw()
{
	m_backImage->draw();
	m_paddle->draw();
	drawActors(m_balls);
	drawActors(m_blocks);

	if (m_state == GameState::Ready)
	{
		m_game->drawString(L"Press space key", Vector2d(120.0f, 450.0f),
			RGB(255, 255, 255), 35);
	}
	else if (m_state == GameState::Clear)
	{
		m_game->drawString(L"Game Clear!", Vector2d(180.0f, 420.0f),
			RGB(255, 255, 30), 35);
		m_game->drawString(L"Press space key", Vector2d(185.0f, 460.0f),
			RGB(255, 255, 35), 24);
	}
	else if (m_state == GameState::Over)
	{
		m_game->drawString(L"Game Over!", Vector2d(180.0f, 420.0f),
			RGB(255, 30, 30), 35);
		m_game->drawString(L"Press space key", Vector2d(185.0f, 460.0f),
			RGB(255, 30, 35), 24);
	}

	int n = (int)m_balls.size();
	m_game->drawString(L"ボールの個数: %d", n,
		ZeroVec2d, RGB(0, 0, 0), 20);
}
