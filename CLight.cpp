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

	m_vlightPos = XMLoadFloat3(&m_position.m_p);
	m_vtargetPos = XMLoadFloat3(&m_target.m_p);
	m_vlightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_mview = XMMatrixLookAtLH(m_vlightPos, m_vtargetPos, m_vlightUp);

	XMStoreFloat4x4(&m_view, m_mview);

	XMStoreFloat3(&m_orthoFrustumCenter, XMVector3TransformCoord(m_vtargetPos, m_mview));

	m_left = m_orthoFrustumCenter.x - m_radius;
	m_right = m_orthoFrustumCenter.x + m_radius;
	m_bottom = m_orthoFrustumCenter.y - m_radius;
	m_top = m_orthoFrustumCenter.y + m_radius;
	m_near = m_orthoFrustumCenter.z - m_radius;
	m_far = m_orthoFrustumCenter.z + m_radius;

	m_mlightProj = XMMatrixOrthographicOffCenterLH(m_left, m_right, m_bottom, m_top, m_near, m_far);
	
	float fieldOfView = 3.14159265358979323846f / 2.0f;
	float screenAspect = 1.0f;
	float SCREEN_DEPTH = 100000.0f;
	float SCREEN_NEAR = 1.0;

	m_mlightProj = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	XMStoreFloat4x4(&m_proj, m_mlightProj);
}