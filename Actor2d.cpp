#include "Actor2d.h"
#include "Scene.h"

Actor2d::Actor2d(Scene* scene, const XMFLOAT2& pos, const XMFLOAT2& vel)
	: Actor(scene)
	, m_pos(pos)
	, m_vel(vel)
{

}

Actor2d::~Actor2d()
{
}
