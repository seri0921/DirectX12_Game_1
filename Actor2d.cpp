#include "Actor2d.h"
#include "Scene.h"

Actor2d::Actor2d(Scene* scene, const XMFLOAT2& pos, const XMFLOAT2& vel,
	const XMFLOAT2& scale, float angle, float angleVel)
	: Actor(scene)
	, m_pos(pos)
	, m_vel(vel)
	, m_scale(scale)
	, m_angle(angle)
	, m_angleVel(angleVel)
{

}

Actor2d::~Actor2d()
{
}

XMFLOAT2 Actor2d::getForward() const
{
	float theta = XMConvertToRadians(m_angle);
	return unitVecAngle(theta);
}

XMFLOAT2 Actor2d::getUp() const
{
	float theta = XMConvertToRadians(m_angle - 90.0f);
	return unitVecAngle(theta);
}

void Actor2d::simulate(float deltaTime)
{
	m_pos += deltaTime * m_vel;
	m_angle += deltaTime * m_angleVel;
	m_angle = (m_angle > 360.0f) ? m_angle - 360.0f : m_angle;
	m_angle = (m_angle < 0.0f) ? m_angle + 360.0f : m_angle;
}