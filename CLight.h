#pragma once

#include "framework.h"

#include "../GameCommon/CVec3f.h"

class CLight
{
public:

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	CVec3f m_position;
	CVec3f m_target;
	CVec3f m_direction;

	float m_radius;

	CLight();
	~CLight();

	void Constructor(CVec3f* position, CVec3f* target, float radius);
	void InitForShadow();
};