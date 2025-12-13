#pragma once

#include "framework.h"

#include "../GameCommon/CLocal.h"
#include "../GameCommon/CScript.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
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
	CScript m_materialScript;
	CScript m_objectScript;
	CShaderMaterial* m_materials;
	CString* m_name;
	CString* m_tokenb;
	CString* m_tokeng;
	CString* m_tokenr;
	CVec3f* m_normal;
	CVec3f* m_vertex;
	CVertexNT* m_meshvertex[CWavefront::MAX_MATERIAL];

	float* m_u;
	float* m_v;

	int32_t m_faceCount;
	int32_t m_materialCount;
	int32_t m_meshvertexCount[CWavefront::MAX_MATERIAL];
	int32_t m_normalCount;
	int32_t m_uvCount;
	int32_t m_vertexCount;

	CWavefront();
	CWavefront(const char* name, const char* filename);
	~CWavefront();

	void InitializeMaterialScript(const char* filename);
	void Load();
	void ProcessMaterialScript();
	void ProcessObjectScript();
};