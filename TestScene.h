#pragma once

#include "Scene.h"
#include "Actor.h"
#include "ColorAnimActor.h"

class TestScene : public Scene
{
public:
	TestScene(class Game* game);
	~TestScene();

	void update(float deltaTime) override;
	void draw() override;

private:

};

