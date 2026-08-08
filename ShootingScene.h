#pragma once

#include "Scene.h"
#include "SpriteActor.h"
#include "PlayerActor.h"

class ShootingScene : public Scene
{
public:
	ShootingScene(class Game* game);
	~ShootingScene();

	void update(float deltaTime) override;
	void draw() override;

	void addPlayerBullet(Actor* bullet) { m_playerBulletsTemp.push_back(bullet); }

private:
	std::unique_ptr<SpriteActor> m_back;
	std::unique_ptr<PlayerActor> m_player;
	std::vector<Actor*> m_playerBullets;
	std::vector<Actor*> m_playerBulletsTemp;
};

