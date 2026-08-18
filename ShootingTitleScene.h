#pragma once

#include "Scene.h"
#include "StringActor.h"

class ShootingTitleScene : public Scene
{
public:
	ShootingTitleScene(class Game* game);
	~ShootingTitleScene();

	SceneState update(float deltaTime, Scene** newScene) override;
	void draw() override;

private:
	std::unique_ptr<StringActor> m_startUI;
	std::unique_ptr<StringActor> m_quitUI;
	int m_selectIndex;
};

