#pragma once

#include <vector>
#include "GameUtil.h"

class Scene
{
public:
	Scene(class Game* game);
	virtual ~Scene();

	virtual void update(float deltaTime) = 0;
	virtual void draw() = 0;

	class Game* getGame() { return m_game; }
	bool isRunning() const { return m_isRunning; }

	void addActor(class Actor* actor);

protected:
	class Game* m_game;
	bool m_isRunning;
	std::vector<class Actor*> m_actors;
	std::vector<class Actor*> m_actorsTemp;
	std::vector<ImageData> m_imgLoadData;
	std::vector<SoundInfo> m_soundLoadData;

};

