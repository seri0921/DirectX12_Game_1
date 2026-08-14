#include "Scene.h"
#include "Game.h"
#include "Actor.h"

Scene::Scene(Game* game)
	: m_game(game)
	, m_isRunning(false)
{
}

Scene::~Scene()
{
	releaseActors(m_actors);
	releaseActors(m_actorsTemp);
	for (int i = 0; i < m_imgLoadData.size(); ++i)
	{
		m_game->getRenderer()->releaseShaderResource(m_imgLoadData[i]);
	}
	for (int i = 0; i < m_soundLoadData.size(); ++i)
	{
		m_game->getSoundSystem()->releaseSoundFile(m_soundLoadData[i]);
	}
}

void Scene::addActor(Actor* actor)
{
	m_actorsTemp.push_back(actor);
}