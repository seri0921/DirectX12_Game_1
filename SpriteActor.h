#pragma once

#include "Actor2d.h"

class SpriteActor : public Actor2d
{
public:
	SpriteActor(class Scene* scene, const std::wstring& filePath,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, XMFLOAT2* uvSize = nullptr,
		const XMFLOAT2& scale = Ones2d, float angle = 0.0f, float angleVel = 0.0f,
		const XMFLOAT2& uvPos = ZeroVec2d, const XMFLOAT2& uvVel = ZeroVec2d,
		bool centerFlag = false, bool ddsFlag = false);
	virtual ~SpriteActor();

	void setScale(XMFLOAT2 s) override;

	virtual void update(float deltaTime) override;
	virtual void draw() override;

	bool isEnabled() const { return m_enabled; }
	int setShader(int shaderIndex);

protected:
	std::wstring m_imgPath;
	bool m_enabled;
	XMFLOAT2 m_offset;
	ImageData m_imgData;
	int m_modelIndex;
	int m_shaderIndex;
	XMFLOAT2 m_spriteSize;
	XMFLOAT2 m_uvSize;
	XMFLOAT2 m_uvPos;
	XMFLOAT2 m_uvVel;
	bool m_center;

};

