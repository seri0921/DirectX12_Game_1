#pragma once

#include "BlockSpriteActor.h"
#include <vector>

class StringActor : public Actor2d
{
public:
	StringActor(class Scene* scene, const std::wstring& str, FontData& fontData,
		const XMFLOAT2& fontSize, int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		const XMFLOAT2& scale = Ones2d, float angle = 0.0f, float angleVel = 0.0f,
		float tracking = 1.0f, int maxStringLength = -1);
	virtual ~StringActor();

	virtual void update(float deltaTime) override;
	virtual void draw() override;

	virtual void setPos(XMFLOAT2 p) override;
	virtual void setScale(XMFLOAT2 s) override;
	virtual void setAngle(float angle) override;
	bool isEnabled() const { return m_enabled; }

	void setString(const std::wstring& str);
	int setShader(int shaderIndex);
	void setColorVector(XMFLOAT3 color, float alpha);

protected:
	bool m_enabled;
	std::vector<BlockSpriteActor*> m_string;
	size_t m_strLength;
	XMFLOAT2 m_fontSize;
	float m_tracking;
	std::vector<FontIndex> m_indices;

	void releaseActors();
	void setStringPos();
	int calcFontIndex(wchar_t c);

};

