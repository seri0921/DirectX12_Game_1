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

	SceneState update(float deltaTime, Scene** newScene) override;
	void draw() override;

private:
	std::unique_ptr<SpriteActor> m_back;
	std::unique_ptr<BlockAnimActor> m_ship;
	std::unique_ptr<BlockSpriteActor> m_lightShip[20];
	std::unique_ptr<SpriteActor> m_unicolor;
};

