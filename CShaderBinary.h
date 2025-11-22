#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"

class CShaderBinary
{
public:

	bool m_isInitialized;

	CComPtr<IDxcBlobEncoding> m_source;
	CComPtr<IDxcBlobUtf16> m_shaderName;
	CComPtr<IDxcBlobUtf8> m_utfErrors;
	CComPtr<IDxcCompiler3> m_compiler;
	CComPtr<IDxcIncludeHandler> m_includeHandler;
	CComPtr<IDxcResult> m_results;
	CComPtr<IDxcUtils> m_utils;

	CErrorLog* m_errorLog;
	CLocal* m_local;
	CString* m_entryPoint;
	CString* m_filename;
	CString* m_name;
	CString* m_version;

	DxcBuffer m_sourceBuffer;

	HRESULT m_hr;

	ID3DBlob* m_errors;
	ID3DBlob* m_shader;

	int32_t m_flags;

	CShaderBinary();
	CShaderBinary(CErrorLog* errorLog, CLocal* local, const char* name, const char* entryPoint, const char* version, int32_t shaderModel);
	~CShaderBinary();
};