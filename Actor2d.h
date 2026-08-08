#pragma once

#include "Actor.h"

class Actor2d : public Actor
{
public:
	Actor2d(class Scene* scene, const XMFLOAT2& pos, const XMFLOAT2& vel,
		const XMFLOAT2& scale = Ones2d, float angle = 0.0f, float angelVel = 0.0f);
	virtual ~Actor2d();

	virtual void setPos(XMFLOAT2 p) { m_pos = p; }
	XMFLOAT2 getPos() const { return m_pos; }
	virtual void setVel(XMFLOAT2 v) { m_vel = v; }
	XMFLOAT2 getVel() const { return m_vel; }

	virtual void setScale(XMFLOAT2 s) { m_scale = s; }
	XMFLOAT2 getScale() const { return m_scale; }
	virtual void setAngle(float angle) { m_angle = angle; }
	float getAngle() const { return m_angle; }
	virtual void setAngleVel(float avel) { m_angleVel = avel; }
	float getAngleVel() const { return m_angleVel; }

	XMFLOAT2 getForward() const;
	XMFLOAT2 getUp() const;
	void simulate(float deltaTime);

protected:
	XMFLOAT2 m_pos;
	XMFLOAT2 m_vel;
	XMFLOAT2 m_scale;
	float m_angle;
	float m_angleVel;

};

