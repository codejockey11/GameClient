#include "CShaderBinaryManager.h"

/*
*/
CShaderBinaryManager::CShaderBinaryManager()
{
	memset(this, 0x00, sizeof(CShaderBinaryManager));
}

/*
*/
CShaderBinaryManager::CShaderBinaryManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local)
{
	memset(this, 0x00, sizeof(CShaderBinaryManager));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_shaderDirectory = new CString(m_local->m_installPath->m_text);

	m_shaderDirectory->Append("shaders/");

	m_psEntry = new CString("main");
	m_vsEntry = new CString("main");

	m_shaderBinaries = new CList();

	m_shaderModel.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_6;

	m_hr = m_graphicsAdapter->m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &m_shaderModel, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL));

	if (m_hr == S_OK)
	{
		switch (m_shaderModel.HighestShaderModel)
		{
		case D3D_SHADER_MODEL_5_1:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_5_1\n");

			m_psTarget = new CString("ps_5_1");
			m_vsTarget = new CString("vs_5_1");

			break;
		}
		case D3D_SHADER_MODEL_6_0:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_0\n");

			m_psTarget = new CString("ps_6_0");
			m_vsTarget = new CString("vs_6_0");

			break;
		}
		case D3D_SHADER_MODEL_6_1:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_1\n");

			m_psTarget = new CString("ps_6_1");
			m_vsTarget = new CString("vs_6_1");

			break;
		}
		case D3D_SHADER_MODEL_6_2:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_2\n");

			m_psTarget = new CString("ps_6_2");
			m_vsTarget = new CString("vs_6_2");

			break;
		}
		case D3D_SHADER_MODEL_6_3:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_3\n");

			m_psTarget = new CString("ps_6_3");
			m_vsTarget = new CString("vs_6_3");

			break;
		}
		case D3D_SHADER_MODEL_6_4:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_4\n");

			m_psTarget = new CString("ps_6_4");
			m_vsTarget = new CString("vs_6_4");

			break;
		}
		case D3D_SHADER_MODEL_6_5:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_5\n");

			m_psTarget = new CString("ps_6_5");
			m_vsTarget = new CString("vs_6_5");

			break;
		}
		case D3D_SHADER_MODEL_6_6:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_6\n");

			m_psTarget = new CString("ps_6_6");
			m_vsTarget = new CString("vs_6_6");

			break;
		}
		case D3D_SHADER_MODEL_6_7:
		{
			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager::D3D_SHADER_MODEL:D3D_SHADER_MODEL_6_7\n");

			m_psTarget = new CString("ps_6_7");
			m_vsTarget = new CString("vs_6_7");

			break;
		}
		}
	}
}

/*
*/
CShaderBinaryManager::~CShaderBinaryManager()
{
	SAFE_DELETE(m_psEntry);
	SAFE_DELETE(m_vsEntry);
	SAFE_DELETE(m_psTarget);
	SAFE_DELETE(m_vsTarget);
	SAFE_DELETE(m_shaderDirectory);

	m_shaderBinaryNode = m_shaderBinaries->m_list;

	while ((m_shaderBinaryNode) && (m_shaderBinaryNode->m_object))
	{
		m_shaderBinary = (CShaderBinary*)m_shaderBinaryNode->m_object;

		SAFE_DELETE(m_shaderBinary);

		m_shaderBinaryNode = m_shaderBinaries->Delete(m_shaderBinaryNode);
	}

	SAFE_DELETE(m_shaderBinaries);
}

/*
*/
CShaderBinary* CShaderBinaryManager::Create(const char* installPath, const char* name, BYTE type)
{
	m_errorLog->WriteError(true, "CShaderBinaryManager::Create:%s\n", name);

	m_shaderBinary = CShaderBinaryManager::Get(name);

	if (m_shaderBinary)
	{
		m_errorLog->WriteError(true, "CShaderBinaryManager::Create:Returned\n");

		return m_shaderBinary;
	}

	m_filename = new CString(m_shaderDirectory->m_text);

	m_filename->Append(name);

	switch (type)
	{
	case CShaderBinaryManager::BinaryType::PIXEL:
	{
		m_shaderBinary = new CShaderBinary(m_errorLog, m_shaderModel.HighestShaderModel, m_psEntry->m_text, m_psTarget->m_text, installPath, m_shaderDirectory->m_text, name, m_filename->m_text);

		break;
	}
	case CShaderBinaryManager::BinaryType::VERTEX:
	{
		m_shaderBinary = new CShaderBinary(m_errorLog, m_shaderModel.HighestShaderModel, m_vsEntry->m_text, m_vsTarget->m_text, installPath, m_shaderDirectory->m_text, name, m_filename->m_text);

		break;
	}
	}

	SAFE_DELETE(m_filename);

	if (m_shaderBinary->m_isInitialized)
	{
		m_errorLog->WriteError(true, "CShaderBinaryManager::Create::Completed\n");

		m_shaderBinaries->Add(m_shaderBinary, name);

		return m_shaderBinary;
	}

	SAFE_DELETE(m_shaderBinary);

	m_errorLog->WriteError(true, "CShaderBinaryManager::Create::Failed\n");

	return nullptr;
}

/*
*/
void CShaderBinaryManager::Delete(const char* name)
{
	m_shaderBinaryNode = m_shaderBinaries->Search(name);

	if (m_shaderBinaryNode)
	{
		m_shaderBinary = (CShaderBinary*)m_shaderBinaryNode->m_object;

		SAFE_DELETE(m_shaderBinary);

		m_shaderBinaries->Delete(m_shaderBinaryNode);
	}
}

/*
*/
CShaderBinary* CShaderBinaryManager::Get(const char* name)
{
	m_shaderBinaryNode = m_shaderBinaries->Search(name);

	if (m_shaderBinaryNode)
	{
		return (CShaderBinary*)m_shaderBinaryNode->m_object;
	}

	return nullptr;
}