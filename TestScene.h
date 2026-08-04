#pragma once

#include "Scene.h"
#include "Actor.h"
#include "ColorAnimActor.h"
#include "SpriteActor.h"

class TestScene : public Scene
{
public:
	TestScene(class Game* game);
	~TestScene();

	void update(float deltaTime) override;
	void draw() override;

private:
	std::unique_ptr<SpriteActor> m_sprites[5];
};

