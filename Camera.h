#pragma once

#include "GMath.h"

// カメラの内部パラメータ
struct CamInParam
{
	float fovY;
	float znear;
	float zfar;

	CamInParam() : fovY(90.0f), znear(0.01f), zfar(1000.0f) {}
	CamInParam(float fv, float inear, float ifar)
		: fovY(fv), znear(inear), zfar(ifar) {}
};

// カメラの外部パラメータ
struct CamExtPram
{
	XMFLOAT3 eye;
	XMFLOAT3 target;
	XMFLOAT3 up;

	CamExtPram() : eye(ZeroVec3d), target(Ones3d), up(UnitVecY3d) {}
	CamExtPram(XMFLOAT3 ieye, XMFLOAT3 itarget, XMFLOAT3 iup)
		: eye(ieye), target(itarget), up(iup) {}
};

// カメラクラス
class Camera
{
public:
	Camera();
	Camera(CamInParam inParam, CamExtPram extParam);
	~Camera();

	CamInParam getInParam() const { return m_inParam; }
	void setInParam(CamInParam inParam) { m_inParam = inParam; }
	void setFovY(float fovY) { m_inParam.fovY = fovY; }
	void setNear(float inear) { m_inParam.znear = inear; }
	void setFar(float ifar) { m_inParam.zfar = ifar; }

	CamExtPram getExParam() const { return m_extParam; }
	void setExtParam(CamExtPram extParam) { m_extParam = extParam; }
	void setEye(XMFLOAT3 eye) { m_extParam.eye = eye; }
	void setTarget(XMFLOAT3 target) { m_extParam.target = target; }
	void setUp(XMFLOAT3 up) { m_extParam.up = up; }

	XMMATRIX calcViewMatrix() const;
	XMMATRIX calcProjMatrix(float w, float h) const;
	XMMATRIX calcViewProjMatrix(float w, float h) const;

private:
	CamInParam m_inParam;
	CamExtPram m_extParam;
};
