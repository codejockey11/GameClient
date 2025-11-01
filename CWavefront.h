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

	bool m_firstTime;
	bool m_isInitialized;

	char* m_mtllib;

	CLocal* m_local;
	CShaderMaterial* m_materials;
	CString* m_filename;
	CString* m_materialFilename;
	CString* m_name;
	CString* m_tokenb;
	CString* m_tokeng;
	CString* m_tokenr;
	CToken m_materialScript;
	CToken m_objectScript;
	CVec3f* m_normal;
	CVec3f* m_vertex;
	CVertexNT* m_meshvertex[CWavefront::MAX_MATERIAL];

	float* m_u;
	float* m_v;

	int m_faceCount;
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