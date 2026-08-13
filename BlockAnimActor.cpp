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
	, m_animState(AnimState::Play)
	, m_loopNum(Loopinfinity)
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
	if (m_animState != AnimState::Play) return;

	m_time += deltaTime;
	if (m_time >= m_interval)
	{
		UINT s = (UINT)m_animalIndices[m_laneIndex].size();
		m_animID += 1;
		if (m_animID >= s)
		{
			m_animID = 0;
			if (m_loopNum != Loopinfinity)
			{
				m_loopNum -= 1;
				if (m_loopNum == 0)
				{
					m_animState = AnimState::Stop;
				}
			}
		}
		setIndex(m_animalIndices[m_laneIndex][m_animID]);
		m_time = 0.0f;
	}
}

void BlockAnimActor::setAnimState(int loopNum, bool startFlag)
{
	m_loopNum = loopNum;
	m_animID = 0;
	m_animState = (startFlag) ? AnimState::Play : AnimState::Pause;
}

void BlockAnimActor::start()
{
	if (m_animState != AnimState::Pause) return;

	m_animState = AnimState::Play;
}

void BlockAnimActor::pause()
{
	if (m_animState != AnimState::Play) return;

	m_animState = AnimState::Pause;
}

void BlockAnimActor::stop()
{
	m_animState = AnimState::Stop;
}