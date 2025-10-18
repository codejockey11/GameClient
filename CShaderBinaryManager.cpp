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

	m_psEntry = new CString("main");
	m_vsEntry = new CString("main");

	m_shaderBinaries = new CLinkList<CShaderBinary>();

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
	delete m_psEntry;
	delete m_vsEntry;
	delete m_psTarget;
	delete m_vsTarget;

	delete m_shaderBinaries;
}

/*
*/
CShaderBinary* CShaderBinaryManager::Create(const char* name, BYTE type)
{
	CShaderBinary* shader = CShaderBinaryManager::Get(name);

	if (shader)
	{
		return shader;
	}

	switch (type)
	{
	case CShaderBinaryManager::BinaryType::PIXEL:
	{
		shader = new CShaderBinary(m_errorLog, m_local, name, m_psEntry->m_text, m_psTarget->m_text, m_shaderModel.HighestShaderModel);

		if (shader->m_isInitialized)
		{
			m_shaderBinaries->Add(shader, shader->m_name->m_text);

			return shader;
		}

		break;
	}
	case CShaderBinaryManager::BinaryType::VERTEX:
	{
		shader = new CShaderBinary(m_errorLog, m_local, name, m_vsEntry->m_text, m_vsTarget->m_text, m_shaderModel.HighestShaderModel);

		if (shader->m_isInitialized)
		{
			m_shaderBinaries->Add(shader, shader->m_name->m_text);

			return shader;
		}

		break;
	}
	}

	m_errorLog->WriteError(true, "CShaderBinaryManager::Create:%s\n", name);

	delete shader;

	return nullptr;
}

/*
*/
void CShaderBinaryManager::Delete(const char* name)
{
	CLinkListNode<CShaderBinary>* lln = m_shaderBinaries->Search(name);

	if (lln)
	{
		m_shaderBinaries->Delete(lln);
	}
}

/*
*/
CShaderBinary* CShaderBinaryManager::Get(const char* name)
{
	CLinkListNode<CShaderBinary>* lln = m_shaderBinaries->Search(name);

	if (lln)
	{
		return lln->m_object;
	}

	return nullptr;
}