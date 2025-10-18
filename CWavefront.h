#pragma once

#include "framework.h"

#include "../GameCommon/CLocal.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CToken.h"
#include "../GameCommon/CVec3f.h"
#include "../GameCommon/CVertexNT.h"

class CWavefront
{
public:

	enum
	{
		LINE_LENGTH = 132,
		MAX_VERTEX = 65536,
		MAX_NORMAL = 65536,
		MAX_UV = 65536,
		MAX_MATERIAL = 4
	};

	CLocal* m_local;
	CShaderMaterial* m_material;
	CString* m_name;
	CToken m_materialScript;
	CToken m_objectScript;
	CVec3f* m_normal;
	CVec3f* m_vertex;
	CVertexNT* m_meshvertex[CWavefront::MAX_MATERIAL];

	bool m_isInitialized;

	float* m_u;
	float* m_v;

	int m_materialCount;
	int m_meshvertexCount[CWavefront::MAX_MATERIAL];
	int m_normalCount;
	int m_uvCount;
	int m_vertexCount;

	CWavefront();
	CWavefront(CLocal* local, char* name);
	~CWavefront();

	void ProcessMaterialScript();
	void ProcessObjectScript();
};