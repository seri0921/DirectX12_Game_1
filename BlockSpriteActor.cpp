#include "BlockSpriteActor.h"
#include "Scene.h"

BlockSpriteActor::BlockSpriteActor(Scene* scene, const std::wstring& filePath,
	UINT wNum, UINT hNum, UINT idx, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel,
	XMFLOAT2* spriteSize, const XMFLOAT2& scale, float angle, float angleVel,
	bool centerFlag, bool ddsFlag)
	: SpriteActor(scene, filePath, shaderIndex, pos, vel, spriteSize, nullptr,
		scale, angle, angleVel, ZeroVec2d, ZeroVec2d, centerFlag, ddsFlag)
	, m_wNum(wNum)
	, m_hNum(hNum)
	, m_index(idx)
{
	if (!isEnabled()) return;

	if (spriteSize == nullptr)
	{
		m_spriteSize = XMFLOAT2((float)(m_imgData.width / m_wNum),
			(float)(m_imgData.height / m_hNum));
		if (!m_center)
		{
			m_offset =
				-0.5f * XMFLOAT2(m_scale.x * m_spriteSize.x, m_scale.y * m_spriteSize.y);
		}
	}
	m_uvSize = XMFLOAT2((float)(m_imgData.width / m_wNum),
		(float)(m_imgData.height / m_hNum));
	UINT bx = m_index % m_wNum;
	UINT by = m_index / m_wNum;
	m_uvPos = XMFLOAT2((float)(bx * m_uvSize.x), (float)(by * m_uvSize.y));
}

BlockSpriteActor::~BlockSpriteActor()
{
}

void BlockSpriteActor::setIndex(UINT idx)
{
	m_index = idx;
	UINT bx = m_index % m_wNum;
	UINT by = m_index / m_wNum;
	m_uvPos = XMFLOAT2((float)(bx * m_uvSize.x), (float)(by * m_uvSize.y));
}