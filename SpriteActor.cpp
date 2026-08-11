#include "SpriteActor.h"
#include "Game.h"
#include "Scene.h"
#include "Renderer.h"

SpriteActor::SpriteActor(Scene* scene, const std::wstring& filePath,
	int shaderIndex, const XMFLOAT2& pos, const XMFLOAT2& vel,
	XMFLOAT2* spriteSize, XMFLOAT2* uvSize,
	const XMFLOAT2& scale, float angle, float angleVel,
	const XMFLOAT2& uvPos, const XMFLOAT2& uvVel,
	bool centerFlag, bool ddsFlag)
	: Actor2d(scene, pos, vel, scale, angle, angleVel)
	, m_imgPath(filePath)
	, m_enabled(false)
	, m_offset(ZeroVec2d)
	, m_modelIndex(-1)
	, m_shaderIndex(shaderIndex)
	, m_spriteSize(Ones2d)
	, m_uvSize(Ones2d)
	, m_center(centerFlag)
	, m_color(Ones3d)
	, m_alpha(1.0f)
	, m_uvPos(uvPos)
	, m_uvVel(uvVel)
{
	Renderer* renderer = m_scene->getGame()->getRenderer();
	SpriteTransData d;
	m_modelIndex = renderer->allocateConstBuffer(&d, sizeof(SpriteTransData));
	if (m_modelIndex == -1)
	{
		return;
	}

	m_imgData = renderer->allocateShaderResource(filePath, ddsFlag);
	if (m_imgData.imgIndex == -1)
	{
		renderer->releaseConstBuffer(m_modelIndex);
		return;
	}
	renderer->setMaterialSlot(m_modelIndex, 0, m_imgData);

	m_spriteSize = (spriteSize != nullptr) ? *spriteSize
		: XMFLOAT2((float)m_imgData.width, (float)m_imgData.height);
	m_offset = (m_center) ? ZeroVec2d
		: -0.5f * XMFLOAT2(m_scale.x * m_spriteSize.x, m_scale.y * m_spriteSize.y);
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
	simulate(deltaTime);
	m_uvPos += deltaTime * m_uvVel;
}

void SpriteActor::draw()
{
	if (!m_visible) return;
	if (!m_enabled) return;

	XMFLOAT2 scale = XMFLOAT2(m_scale.x * m_spriteSize.x, m_scale.y * m_spriteSize.y);
	m_scene->getGame()->getRenderer()->drawSprite(
		m_modelIndex, m_shaderIndex, m_imgData, m_pos, m_angle,
		&scale, m_offset, m_uvPos, &m_uvSize, m_color, m_alpha);
}

void SpriteActor::setScale(XMFLOAT2 s)
{
	m_scale = s;

	m_offset = (m_center) ? ZeroVec2d
		: -0.5f * XMFLOAT2(m_scale.x * m_spriteSize.x, m_scale.y * m_spriteSize.y);
}

int SpriteActor::setShader(int shaderIndex)
{
	int s = m_shaderIndex;
	m_shaderIndex = shaderIndex;

	return s;
}

void SpriteActor::setColorVector(XMFLOAT3 color, float alpha)
{
	m_color = color;
	m_alpha = alpha;
}

XMFLOAT4 SpriteActor::getColorVector() const
{
	XMFLOAT4 color = { m_color.x, m_color.y, m_color.z, m_alpha };
	return color;
}