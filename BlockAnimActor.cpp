#include "BlockAnimActor.h"
#include "Scene.h"

BlockAnimActor::BlockAnimActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize, const XMFLOAT2& scale,
	float angle, float angleVel, bool centerFlag, bool ddsFlag)
	: BlockSpriteActor(scene, filePath, wNum, hNum, 0, shaderIndex,
		pos, vel, spriteSize, scale, angle, angleVel, centerFlag, ddsFlag)
	, m_animalIndices(indices)
	, m_laneIndex(lane)
	, m_animID(0)
	, m_time(0.0f)
	, m_interval(interval)
{
	if (!isEnabled()) return;

	setIndex(m_animalIndices[m_laneIndex][m_animID]);
}

BlockAnimActor::~BlockAnimActor()
{
}

void BlockAnimActor::update(float deltaTime)
{
	BlockSpriteActor::update(deltaTime);
	updateAnim(deltaTime);
}

void BlockAnimActor::changeLane(UINT lane)
{
	if (m_laneIndex == lane) return;

	m_laneIndex = lane;
	m_animID = 0;
	m_time = 0.0f;
	setIndex(m_animalIndices[m_laneIndex][m_animID]);
}

void BlockAnimActor::updateAnim(float deltaTime)
{
	m_time += deltaTime;
	if (m_time >= m_interval)
	{
		UINT s = (UINT)m_animalIndices[m_laneIndex].size();
		m_animID = (m_animID + 1) % s;
		setIndex(m_animalIndices[m_laneIndex][m_animID]);
		m_time = 0.0f;
	}
}