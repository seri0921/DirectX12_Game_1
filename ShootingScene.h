#pragma once

#include "Scene.h"
#include "SpriteActor.h"
#include "PlayerActor.h"
#include "StringActor.h"

class ShootingScene : public Scene
{
public:
	ShootingScene(class Game* game);
	~ShootingScene();

	SceneState update(float deltaTime, Scene** newScene) override;
	void draw() override;

	void addPlayerBullet(Actor* bullet) { m_playerBulletsTemp.push_back(bullet); }
	void addEnemyBullet(Actor* bullet) { m_enemyBulletsTemp.push_back(bullet); }
	void addEnemy(Actor* enemy) { m_enemiesTemp.push_back(enemy); }

	PlayerActor* getPlayer() { return m_player.get(); }
	std::vector<Actor*>& getEnemies() { return m_enemies; }

	int getScore() const { return m_score; }
	void setScore(int score) { m_score = (score > 9999) ? 9999 : score; }

private:
	std::unique_ptr<SpriteActor> m_back;
	std::unique_ptr<PlayerActor> m_player;
	std::vector<Actor*> m_playerBullets;
	std::vector<Actor*> m_playerBulletsTemp;
	std::vector<Actor*> m_enemyBullets;
	std::vector<Actor*> m_enemyBulletsTemp;
	std::vector<Actor*> m_enemies;
	std::vector<Actor*> m_enemiesTemp;
	std::unique_ptr<StringActor> m_scoreString;
	int m_score;

	std::unique_ptr<StringActor> m_messageUI;
};

