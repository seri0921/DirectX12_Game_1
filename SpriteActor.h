#pragma once

#include "Actor2d.h"

class SpriteActor : public Actor2d
{
public:
	SpriteActor(class Scene* scene, const std::wstring& filePath,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, XMFLOAT2* uvSize = nullptr,
		bool centerFlag = false, bool ddsFlag = false);
	virtual ~SpriteActor();

	virtual void update(float deltaTime) override;
	virtual void draw() override;

	bool isEnebled() const { return m_enabled; }

protected:
	std::wstring m_imgPath;
	bool m_enabled;
	XMFLOAT2 m_offset;
	ImageData m_imgData;
	int m_modelIndex;
	XMFLOAT2 m_spriteSize;
	XMFLOAT2 m_uvSize;
	bool m_center;

};

