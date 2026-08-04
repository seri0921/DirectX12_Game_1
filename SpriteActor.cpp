#include "SpriteActor.h"
#include "Game.h"
#include "Scene.h"
#include "Renderer.h"

SpriteActor::SpriteActor(Scene* scene, const std::wstring& filePath,
	const XMFLOAT2& pos, const XMFLOAT2& vel,
	XMFLOAT2* spriteSize, XMFLOAT2* uvSize,
	bool centerFlag, bool ddsFlag)
	: Actor2d(scene, pos, vel)
	, m_imgPath(filePath)
	, m_enabled(false)
	, m_offset(ZeroVec2d)
	, m_modelIndex(-1)
	, m_spriteSize(Ones2d)
	, m_uvSize(Ones2d)
	, m_center(centerFlag)
{
	Renderer* renderer = m_scene->getGame()->getRenderer();
	SpriteTransData d;
	m_modelIndex = renderer->allocateConstBuffer(&d, sizeof(SpriteTransData));
	if (m_modelIndex == -1)
	{
		return;
	}

	m_imgData = renderer->allocateShaderResource(filePath, false);
	if (m_imgData.imgIndex == -1)
	{
		renderer->releaseConstBuffer(m_modelIndex);
		return;
	}
	renderer->setMaterialSlot(m_modelIndex, 0, m_imgData);

	m_spriteSize = (spriteSize != nullptr) ? *spriteSize
		: XMFLOAT2((float)m_imgData.width, (float)m_imgData.height);
	m_offset = (m_center) ? ZeroVec2d
		: -0.5f * XMFLOAT2(m_spriteSize.x, m_spriteSize.y);
	m_uvSize = (uvSize != nullptr) ? *uvSize
		: XMFLOAT2((float)m_imgData.width, (float)m_imgData.height);

	m_enabled = true;
}

SpriteActor::~SpriteActor()
{
	if (m_modelIndex >= 0)
	{
		m_scene->getGame()->getRenderer()->releaseConstBuffer(m_modelIndex);
	}
}

void SpriteActor::update(float deltaTime)
{
	m_pos += m_vel * deltaTime;
}

void SpriteActor::draw()
{
	if (!m_visible) return;
	if (!m_enabled) return;

	XMFLOAT2 scale = XMFLOAT2(m_spriteSize.x, m_spriteSize.y);
	m_scene->getGame()->getRenderer()->drawSprite(
		m_modelIndex, m_imgData, m_pos, 0.0f,
		&scale, m_offset, ZeroVec2d, &m_uvSize);
}
