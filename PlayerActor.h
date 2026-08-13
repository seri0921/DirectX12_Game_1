#pragma once

#include "CharaActor.h"

class PlayerActor : public CharaActor
{
public: 
	PlayerActor(class Scene* scene, const std::wstring& filePath,
		const std::vector<std::vector<UINT>>& indices, float radius,
		UINT lane, float interval, UINT wNum = 3, UINT hNum = 4,
		int shaderIndex = Renderer::Shader2DAlphaLoopPoint,
		const XMFLOAT2& pos = ZeroVec2d, const XMFLOAT2& vel = ZeroVec2d,
		XMFLOAT2* spriteSize = nullptr, const XMFLOAT2& scale = Ones2d,
		float angle = 0.0f, float angleVel = 0.0f,
		bool centerFlag = true, bool ddsFlag = false);
	~PlayerActor();

	void update(float deltaTime) override;
	void draw() override;
	void damage(float dm = 0.0f) override;

	void setWaitInterval(float interval) { m_waitInterval = interval; }
	bool isInvincibleMode() const { return m_invincibleMode; }

	enum class PlayerState
	{
		Playing, Dying
	};
	PlayerState getState() const { return m_state; }

private:
	float m_waitTime;
	float m_waitInterval;
	int m_HP;
	float m_invincibleTime;
	int m_invincibleCount;
	bool m_invincibleMode;
	PlayerState m_state;
	std::unique_ptr<BlockAnimActor> m_smoke;
	bool m_mouseMode;

	void updatePlaying(float deltaTime);
	void updateDying(float deltaTime);
};

