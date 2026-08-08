#pragma once

#include "Scene.h"
#include "Actor.h"
#include "ColorAnimActor.h"
#include "SpriteActor.h"
#include "BlockSpriteActor.h"
#include "BlockAnimActor.h"

class TestScene : public Scene
{
public:
	TestScene(class Game* game);
	~TestScene();

	void update(float deltaTime) override;
	void draw() override;

private:
	std::unique_ptr<SpriteActor> m_back;
	std::unique_ptr<BlockAnimActor> m_ship;
	std::unique_ptr<BlockSpriteActor> m_lightShip[20];
};

