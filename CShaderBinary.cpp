#include "CShaderBinary.h"

/*
*/
CShaderBinary::CShaderBinary()
{
	memset(this, 0x00, sizeof(CShaderBinary));
}

/*
*/
CShaderBinary::CShaderBinary(CErrorLog* errorLog, int32_t shaderModel, const char* entryPoint, const char* version, const char* installPath, const char* shaderDirectory,
	const char* name, const char* filename)
{
	memset(this, 0x00, sizeof(CShaderBinary));

	m_errorLog = errorLog;

	m_entryPoint = new CString(entryPoint);

	m_version = new CString(version);

	m_shaderDirectory = new CString(shaderDirectory);

	m_name = new CString(name);

	m_filename = new CString(filename);

#ifdef _DEBUG
	m_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	if (shaderModel <= D3D_SHADER_MODEL::D3D_SHADER_MODEL_5_1)
	{
		m_hr = D3DCompileFromFile(m_filename->GetWide(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, m_entryPoint->m_text, m_version->m_text, m_flags, 0, &m_shader, &m_errors);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteComErrorMessage(true, "CShaderBinary::CShaderBinary::D3DCompileFromFile:", m_hr);

			if (m_errors)
			{
				m_errorLog->WriteError(true, "CShaderBinary::CShaderBinary::Compile Errors\n%s", (char*)m_errors->GetBufferPointer());

				SAFE_RELEASE(m_errors);
			}

			m_shader = nullptr;

			return;
		}

		m_isInitialized = true;

		return;
	}

	m_byteCodeFilename = new CString(installPath);

	m_byteCodeFilename->Append("cache/");
	m_byteCodeFilename->Append(m_name->m_text);
	m_byteCodeFilename->Append("_");
	m_byteCodeFilename->Append(m_version->m_text);
	m_byteCodeFilename->Append(".cso");

	if (!CShaderBinary::Load())
	{
		CShaderBinary::Compile();
	}

	SAFE_DELETE(m_byteCodeFilename);

	m_isInitialized = true;
}

/*
*/
CShaderBinary::~CShaderBinary()
{
	SAFE_DELETE(m_filename);
	SAFE_DELETE(m_name);
	SAFE_DELETE(m_shaderDirectory);
	SAFE_DELETE(m_version);
	SAFE_DELETE(m_entryPoint);

	SAFE_RELEASE(m_shader);
}

/*
*/
void CShaderBinary::Compile()
{
	DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), (void**)&m_utils);

	DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), (void**)&m_compiler);

	m_utils->CreateDefaultIncludeHandler(&m_includeHandler);

	LPCWSTR pszArgs[] =
	{
		L"-E", m_entryPoint->GetWide(),	// Entry point.
		L"-T", m_version->GetWide(),	// Target.
		L"-I", m_shaderDirectory->GetWide(), // include path to 'shader' directory
		L"-Qstrip_reflect"				// Strip reflection into a separate blob. 
	};

	m_utils->LoadFile(m_filename->GetWide(), nullptr, &m_source);

	m_sourceBuffer.Ptr = m_source->GetBufferPointer();
	m_sourceBuffer.Size = m_source->GetBufferSize();
	m_sourceBuffer.Encoding = DXC_CP_ACP;

	m_compiler->Compile(
		&m_sourceBuffer,
		pszArgs,
		_countof(pszArgs),
		m_includeHandler,
		__uuidof(IDxcResult),
		(void**)&m_results);

	m_results->GetOutput(DXC_OUT_ERRORS, __uuidof(IDxcBlobUtf8), (void**)&m_utfErrors, nullptr);

	if ((m_utfErrors) && (m_utfErrors->GetStringLength() > 0))
	{
		m_errorLog->WriteError(true, "CShaderBinary::CShaderBinary:Compile Errors\n%s", m_utfErrors->GetStringPointer());

		m_shader = nullptr;

		m_includeHandler.Release();
		m_utils.Release();
		m_compiler.Release();
		m_source.Release();
		m_results.Release();
		m_utfErrors.Release();
		m_shaderName.Release();

		return;
	}

	m_results->GetOutput(DXC_OUT_OBJECT, __uuidof(ID3DBlob), (void**)&m_shader, &m_shaderName);

	m_hr = D3DWriteBlobToFile(m_shader, m_byteCodeFilename->GetWide(), true);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteError(true, "CShaderBinary::Compile:Write Failed:\n%s", m_byteCodeFilename->m_text);
	}

	m_includeHandler.Release();
	m_utils.Release();
	m_compiler.Release();
	m_source.Release();
	m_results.Release();
	m_utfErrors.Release();
	m_shaderName.Release();
}

/*
*/
bool CShaderBinary::Load()
{
	m_hr = D3DReadFileToBlob(m_byteCodeFilename->GetWide(), &m_shader);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteError(true, "CShaderBinary::Load:Failed:%s\n", m_byteCodeFilename->m_text);

		m_shader = nullptr;

		return false;
	}

	return true;
}