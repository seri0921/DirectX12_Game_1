#include "StringActor.h"
#include "Scene.h"
#include "GameUtil.h"

StringActor::StringActor(Scene* scene, const std::wstring& str, FontData& fontData,
	const XMFLOAT2& fontSize, int shaderIndex, const XMFLOAT2& pos, const XMFLOAT2& vel,
	const XMFLOAT2& scale, float angle, float angleVel, float tracking, int maxStringLength)
	: Actor2d(scene, pos, vel, scale, angle, angleVel)
	, m_enabled(false)
	, m_fontSize(fontSize)
	, m_tracking(tracking)
	, m_strLength(0)
	, m_indices(fontData.indices)
{
	m_strLength = wcslen(str.c_str());
	size_t maxLen = m_strLength;
	if (maxStringLength > 0 && (size_t)maxStringLength > m_strLength)
	{
		maxLen = (size_t)maxStringLength;
	}
	m_string.resize(maxLen);

	XMFLOAT2 fsize = m_fontSize * m_scale;
	for (size_t i = 0; i < m_string.size(); ++i)
	{
		XMFLOAT2 fpos = m_pos + ((fsize.x * (float)i * m_tracking) * getForward());
		BlockSpriteActor* actor = new BlockSpriteActor(m_scene,
			fontData.fontName, fontData.wNum, fontData.hNum, 0, shaderIndex, fpos, ZeroVec2d,
			&m_fontSize, m_scale, m_angle, 0.0f, false, fontData.ddsFlag);
		if (!actor->isEnabled())
		{
			delete actor;
			releaseActors();
			return;
		}
		m_string[i] = actor;
	}
	setString(str);

	m_enabled = true;
}

StringActor::~StringActor()
{
	releaseActors();
}

void StringActor::update(float deltaTime)
{
	if (!m_enabled) return;

	simulate(deltaTime);
	setStringPos();
}

void StringActor::draw()
{
	if (!m_enabled) return;

	for (size_t i = 0; i < m_strLength; ++i)
	{
		m_string[i]->draw();
	}
}

void StringActor::setPos(XMFLOAT2 p)
{
	m_pos = p;
	setStringPos();
}

void StringActor::setScale(XMFLOAT2 s)
{
	m_scale = s;
	for (size_t i = 0; i < m_strLength; ++i)
	{
		m_string[i]->setScale(s);
	}
	setStringPos();
}

void StringActor::setAngle(float angle)
{
	m_angle = angle;
	for (size_t i = 0; i < m_strLength; ++i)
	{
		m_string[i]->setAngle(angle);
	}
	setStringPos();
}

void StringActor::releaseActors()
{
	for (size_t i = 0; i < m_string.size(); ++i)
	{
		delete m_string[i];
	}
	m_string.clear();
	m_enabled = false;
}

void StringActor::setString(const std::wstring& str)
{
	m_strLength = wcslen(str.c_str());
	m_strLength = (m_strLength > m_string.size()) ?
		m_string.size() : m_strLength;
	for (size_t i = 0; i < m_strLength; ++i)
	{
		m_string[i]->setIndex(calcFontIndex(str[i]));
	}
}

void StringActor::setStringPos()
{
	XMFLOAT2 fsize = m_fontSize * m_scale;
	for (size_t i = 0; i < m_strLength; ++i)
	{
		XMFLOAT2 fpos = m_pos + ((fsize.x * (float)i * m_tracking) * getForward());
		m_string[i]->setPos(fpos);
		m_string[i]->setAngle(m_angle);
	}
}

int StringActor::calcFontIndex(wchar_t c)
{
	for (size_t i = 0; i < m_indices.size(); ++i)
	{
		if (!m_indices[i].isInRange(c)) continue;

		return m_indices[i].indexOffset + (int)(c - m_indices[i].start);
	}

	return 0;
}

int StringActor::setShader(int shaderIndex)
{
	if (!m_enabled) return Renderer::ShaderNone;
	if (m_string.size() == 0) return Renderer::ShaderNone;

	int oldIndex = m_string[0]->setShader(shaderIndex);
	for (size_t i = 1; i < m_string.size(); ++i)
	{
		m_string[i]->setShader(shaderIndex);
	}

	return oldIndex;
}

void StringActor::setColorVector(XMFLOAT3 color, float alpha)
{
	for (size_t i = 0; i < m_string.size(); i++)
	{
		m_string[i]->setColorVector(color, alpha);
	}
}