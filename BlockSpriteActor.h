#pragma once

#include "SpriteActor.h"

class BlockSpriteActor : public SpriteActor
{
public:
	BlockSpriteActor(class Scene* scene, const std::wstring& filePath,
		UINT wNum = 3, UINT hNum = 4, UINT idx = 0,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d, 
		float angle = 0.0f, float angleVel = 0.0f,
		bool centerFlag = false, bool ddsFlag = false);
	virtual ~BlockSpriteActor();

	UINT getIndex() const { return m_index; }
	void setIndex(UINT idx);

protected:
	UINT m_wNum;
	UINT m_hNum;
	UINT m_index;

};