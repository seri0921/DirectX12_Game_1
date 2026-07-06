#pragma once

#include "Scene.h"
#include "Actor.h"
#include "PaddleActor.h"
#include "SpriteActor.h"

class BreakoutScene : public Scene
{
public:
	BreakoutScene(class Game* game);
	~BreakoutScene();

	void update(float deltaTime) override;
	void draw() override;

	PaddleActor* getPaddle() { return m_paddle.get(); }
	std::vector<Actor*>& getBlocks() { return m_blocks; }

	enum class GameState
	{
		Ready, Play, Over, Clear
	};
	GameState getState() const { return m_state; }

private:
	static const float BallRadius;
	static const float BallAngle;
	static const float BallSpeed;
	static const COLORREF BallColor;
	static const float PaddleYPos;

	std::vector<Actor*> m_balls;
	std::unique_ptr<PaddleActor> m_paddle;
	std::vector<Actor*> m_blocks;
	std::unique_ptr<SpriteActor> m_backImage;
	GameState m_state;

};

typedef BreakoutScene::GameState State;

