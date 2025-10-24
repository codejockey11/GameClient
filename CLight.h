#pragma once

#include "framework.h"

#include "../GameCommon/CVec3f.h"

class CLight
{
public:

	CVec3f m_direction;
	CVec3f m_position;
	CVec3f m_target;

	float m_bottom;
	float m_far;
	float m_left;
	float m_near;
	float m_radius;
	float m_right;
	float m_top;

	XMFLOAT3 m_orthoFrustumCenter;

	XMFLOAT4X4 m_proj;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_world;

	XMMATRIX m_mlightProj;
	XMMATRIX m_mview;

	XMVECTOR m_vlightPos;
	XMVECTOR m_vlightUp;
	XMVECTOR m_vtargetPos;

	CLight();
	~CLight();

	void Constructor(CVec3f* position, CVec3f* target, float radius);
	void InitForShadow();
};