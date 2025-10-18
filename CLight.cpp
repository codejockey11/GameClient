#include "CLight.h"

/*
*/
CLight::CLight()
{
	memset(this, 0x00, sizeof(CLight));
}

/*
*/
CLight::~CLight()
{

}

/*
*/
void CLight::Constructor(CVec3f* position, CVec3f* target, float radius)
{
	memset(this, 0x00, sizeof(CLight));

	m_position = position->m_p;

	m_target = target->m_p;

	m_direction = m_position - m_target;

	m_direction.Normalize();

	m_radius = radius;
}

/*
*/
void CLight::InitForShadow()
{
	XMStoreFloat4x4(&m_world, XMMatrixIdentity());

	XMVECTOR lightPos = XMLoadFloat3(&m_position.m_p);
	XMVECTOR targetPos = XMLoadFloat3(&m_target.m_p);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMVECTOR lookAtVector = XMLoadFloat3(&m_direction.m_p);

	view = XMMatrixLookAtLH(lightPos, lookAtVector, lightUp);

	XMStoreFloat4x4(&m_view, view);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS = {};

	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, view));

	// Ortho frustum in light space encloses scene.
	// mSceneBounds.Radius = 1024;
	float l = sphereCenterLS.x - 32768.0f;
	float b = sphereCenterLS.y - 32768.0f;
	float n = sphereCenterLS.z - 32768.0f;
	float r = sphereCenterLS.x + 32768.0f;
	float t = sphereCenterLS.y + 32768.0f;
	float f = sphereCenterLS.z + 32768.0f;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	float fieldOfView = 3.14159265358979323846f / 2.0f;
	float screenAspect = 1.0f;
	float SCREEN_DEPTH = 32768.0f;
	float SCREEN_NEAR = 1.0f;

	lightProj = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	XMStoreFloat4x4(&m_proj, lightProj);
}