#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeap.h"
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
	CHeap* m_byteCodeHeap;
	CString* m_entryPoint;
	CString* m_filename;
	CString* m_byteCodeFilename;
	CString* m_name;
	CString* m_shaderDirectory;
	CString* m_version;

	DxcBuffer m_sourceBuffer;

	errno_t m_err;

	FILE* m_file;

	HRESULT m_hr;

	ID3DBlob* m_errors;
	ID3DBlob* m_shader;

	int32_t m_flags;
	int32_t m_filePosition;
	int32_t m_fileSize;

	CShaderBinary();
	CShaderBinary(CErrorLog* errorLog, int32_t shaderModel, const char* entryPoint, const char* version, const char* installPath, const char* shaderDirectory,
		const char* name, const char* filename);
	~CShaderBinary();

	void Compile();
	bool Load();
};