#include "Camera.h"

Camera::Camera()
	: m_inParam()
	, m_extParam()
{
}

Camera::Camera(CamInParam inParam, CamExtPram extParam)
	: m_inParam(inParam)
	, m_extParam(extParam)
{
}

Camera::~Camera()
{
}

XMMATRIX Camera::calcViewMatrix() const 
{
	return XMMatrixLookAtLH(XMLoadFloat3(&m_extParam.eye),
		XMLoadFloat3(&m_extParam.target), XMLoadFloat3(&m_extParam.up));
}

XMMATRIX Camera::calcProjMatrix(float w, float h) const
{
	return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_inParam.fovY),
		w / h, m_inParam.znear, m_inParam.zfar);
}
XMMATRIX Camera::calcViewProjMatrix(float w, float h) const
{
	XMMATRIX view = calcViewMatrix();
	XMMATRIX proj = calcProjMatrix(w, h);

	return view * proj;
}