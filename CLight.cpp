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

	m_vPos = XMLoadFloat3(&m_position.m_p);
	m_vTarget = XMLoadFloat3(&m_target.m_p);
	//m_vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_mView = XMMatrixLookAtLH(m_vPos, m_vTarget, m_vUp);

	XMStoreFloat4x4(&m_view, m_mView);

	float fieldOfView = 3.14159265358979323846f / 2.0f;
	float screenAspect = 1.0f;
	float SCREEN_DEPTH = 4096.0f;
	float SCREEN_NEAR = 1.0;

	m_mProj = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	XMStoreFloat4x4(&m_proj, m_mProj);
}