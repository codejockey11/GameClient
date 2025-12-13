#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"

#include "CGraphicsAdapter.h"
#include "CShaderBinary.h"

class CShaderBinaryManager
{
public:

	enum BinaryType
	{
		PIXEL = 0,
		VERTEX
	};

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CList* m_shaderBinaries;
	CListNode* m_shaderBinaryNode;
	CLocal* m_local;
	CShaderBinary* m_shaderBinary;
	CString* m_filename;
	CString* m_shaderDirectory;
	CString* m_psEntry;
	CString* m_psTarget;
	CString* m_vsEntry;
	CString* m_vsTarget;

	D3D12_FEATURE_DATA_SHADER_MODEL m_shaderModel;
	D3D12_FEATURE_DATA_SHADER_MODEL m_shaderModelCheck;

	HRESULT m_hr;

	CShaderBinaryManager();
	CShaderBinaryManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local);
	~CShaderBinaryManager();

	CShaderBinary* Create(const char* installPath, const char* name, BYTE type);
	void Delete(const char* name);
	CShaderBinary* Get(const char* name);
};