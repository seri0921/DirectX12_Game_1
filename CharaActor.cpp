#include "CharaActor.h"
#include "Scene.h"

CharaActor::CharaActor(Scene* scene, const std::wstring& filePath,
	const std::vector<std::vector<UINT>>& indices, float radius,
	UINT lane, float interval, UINT wNum, UINT hNum, int shaderIndex,
	const XMFLOAT2& pos, const XMFLOAT2& vel, XMFLOAT2* spriteSize,
	const XMFLOAT2& scale, float angle, float angleVel,
	bool centerFlag, bool ddsFlag)
	: BlockAnimActor(scene, filePath, indices, lane, interval, wNum, hNum,
		shaderIndex, pos, vel, spriteSize, scale, angle, angleVel,
		centerFlag, ddsFlag)
	, m_radius(radius)
{
}

CharaActor::~CharaActor()
{
}
