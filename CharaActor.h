#pragma once

#include "BlockAnimActor.h"

class CharaActor : public BlockAnimActor
{
public:
	CharaActor(class Scene* scene, const std::wstring& filePath,
		const std::vector<std::vector<UINT>>& indices, float radius,
		UINT lane, float interval, UINT wNum = 3, UINT hNum = 4,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d,
		float angle = 0.0f, float angleVel = 0.0f,
		bool centerFlag = true, bool ddsFlag = false);
	virtual ~CharaActor();

	Circle getCircle() const { return Circle(m_pos, m_radius); }

protected:
	float m_radius;
};

