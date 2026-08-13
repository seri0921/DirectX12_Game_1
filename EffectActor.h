#pragma once

#include "BlockAnimActor.h"

class EffectActor : public BlockAnimActor
{
public:
	EffectActor(class Scene* scene, const std::wstring& filePath,
		const std::vector<std::vector<UINT>>& indices,
		UINT lane, float interval, UINT wNum = 3, UINT hNum = 4,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d,
		float angle = 0.0f, float angleVel = 0.0f,
		bool centerFlag = true, bool ddsFlag = false);
	~EffectActor();

	virtual void update(float deltaTime) override;
	virtual void draw() override;

	void setDelayTime(float time) { m_delayTime = time; }
	float getDelayTime() const { return m_delayTime; }

protected:
	float m_delayTime;

};

// 複数のエフェクトを生成
void createMultipleEffects(class Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices,
	UINT lane, float interval, UINT wNum = 3, UINT hNum = 4,
	int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
	UINT eNum = 1, float eInterval = 0.1f, float eVel = 100.0f,
	float eWidth = 32.0f, float eHeight = 32.0f,
	const XMFLOAT2& basePos = ZeroVec2d,
	XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d,
	bool centerFlag = false, bool ddsFlag = false);

