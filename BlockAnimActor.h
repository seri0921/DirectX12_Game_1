#pragma once

#include "BlockSpriteActor.h"

class BlockAnimActor : public BlockSpriteActor
{
public:
	BlockAnimActor(class Scene* scene, const std::wstring& filePath,
		const std::vector<std::vector<UINT>>& indices,
		UINT lane, float interval,UINT wNum = 3, UINT hNum = 4,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d,
		float angle = 0.0f, float angleVel = 0.0f,
		bool centerFlag = false, bool ddsFlag = false);
	virtual ~BlockAnimActor();

	virtual void update(float deltaTime) override;
	void changeLane(UINT lane);

	enum class AnimState
	{
		Play, Pause, Stop
	};
	static const int Loopinfinity = -1;

	AnimState getAnimState() const { return m_animState; }
	void setAnimState(int loopNum, bool startFlag);
	void start();
	void pause();
	void stop();

protected:
	std::vector<std::vector<UINT>> m_animalIndices;
	UINT m_laneIndex;
	UINT m_animID;
	float m_time;
	float m_interval;
	int m_loopNum;
	AnimState m_animState;

	void updateAnim(float deltaTime);
};

