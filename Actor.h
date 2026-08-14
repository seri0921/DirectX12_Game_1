#pragma once

#include "GameUtil.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <Windows.h>
#include <vector>
#include <memory>
#include "Renderer.h"
#include "SoundSystem.h"

class Actor
{
public:
	Actor(class Scene* scene);
	virtual ~Actor();

	virtual void update(float deltaTime) = 0;
	virtual void draw() = 0;
	void setDead() { m_dead = true; }
	bool isDead() const { return m_dead; }
	void setVisible(bool visible) { m_visible = visible; }
	bool isVisible() const { return m_visible; }

	void setMaxSpeed(float speed) { m_maxSpeed = speed; }
	float getMaxSpeed() const { return m_maxSpeed; }
	void setLifeTime(float life) { m_lifeTime = life; }
	float getLifeTime() const { return m_lifeTime; }

	virtual void damage(float dm = 0.0f) {}

protected:
	class Scene* m_scene;
	bool m_dead;
	bool m_visible;

	float m_maxSpeed;
	float m_lifeTime;

};

void updateActors(std::vector<Actor*>& actors, float deltaTime);
void drawActors(std::vector<Actor*>& actors);
void releaseActors(std::vector<Actor*>& actors);
void removeActors(std::vector<Actor*>& actors);
void moveIntoActors(std::vector<Actor*>& temp, std::vector<Actor*>& actors);

