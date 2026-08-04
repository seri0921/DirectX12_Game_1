#pragma once

#include "Actor.h"

class Actor2d : public Actor
{
public:
	Actor2d(class Scene* scene, const XMFLOAT2& pos, const XMFLOAT2& vel);
	virtual ~Actor2d();

	void setPos(XMFLOAT2 p) { m_pos = p; }
	XMFLOAT2 getPos() const { return m_pos; }
	void setVel(XMFLOAT2 v) { m_vel = v; }
	XMFLOAT2 getVel() const { return m_vel; }

protected:
	XMFLOAT2 m_pos;
	XMFLOAT2 m_vel;

};

