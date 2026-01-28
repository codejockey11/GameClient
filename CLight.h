#pragma once

#include "framework.h"

#include "../GameCommon/CVec3f.h"

class CLight
{
public:

	enum
	{
		LINEAR = 0,
		POINT,
		DIRECTIONAL
	};

	int32_t m_type;

	CVec3f m_color;
	CVec3f m_direction;
	CVec3f m_position;
	CVec3f m_target;

	float m_radius;

	XMFLOAT4X4 m_proj;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_world;

	XMMATRIX m_mProj;
	XMMATRIX m_mView;

	XMVECTOR m_vPos;
	XMVECTOR m_vUp;
	XMVECTOR m_vTarget;

	CLight();
	~CLight();

	void Constructor(CVec3f* position, CVec3f* target, float radius);
	void InitForShadow();
};