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

	/*
		D3D_SHADER_MODEL_5_1	= 0x51,
		D3D_SHADER_MODEL_6_0	= 0x60,
		D3D_SHADER_MODEL_6_1	= 0x61,
		D3D_SHADER_MODEL_6_2	= 0x62,
		D3D_SHADER_MODEL_6_3	= 0x63,
		D3D_SHADER_MODEL_6_4	= 0x64,
		D3D_SHADER_MODEL_6_5	= 0x65,
		D3D_SHADER_MODEL_6_6	= 0x66,
		D3D_SHADER_MODEL_6_7	= 0x67,
		D3D_SHADER_MODEL_6_8	= 0x68,
		D3D_SHADER_MODEL_6_9	= 0x69
	*/

	for (int32_t i = 0; i < 11; i++)
	{
		switch (i)
		{
		case 0:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_5_1;

			break;
		}
		case 1:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_0;

			break;
		}
		case 2:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_1;

			break;
		}
		case 3:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2;

			break;
		}
		case 4:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_3;

			break;
		}
		case 5:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4;

			break;
		}
		case 6:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_5;

			break;
		}
		case 7:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_6;

			break;
		}
		case 8:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_7;

			break;
		}
		case 9:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_8;

			break;
		}
		case 10:
		{
			m_shaderModelCheck.HighestShaderModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_9;

			break;
		}
		}

		m_hr = m_graphicsAdapter->m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &m_shaderModelCheck, sizeof(m_shaderModelCheck));

		if (m_hr == S_OK)
		{
			m_shaderModel = m_shaderModelCheck;

			m_errorLog->WriteError(true, "CShaderBinaryManager::CShaderBinaryManager:CheckFeatureSupport:shaderModel:0x%0x\n", m_shaderModel.HighestShaderModel);
		}
	}

	switch (m_shaderModel.HighestShaderModel)
	{
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_5_1:
	{
		m_psTarget = new CString("ps_5_1");
		m_vsTarget = new CString("vs_5_1");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_0:
	{
		m_psTarget = new CString("ps_6_0");
		m_vsTarget = new CString("vs_6_0");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_1:
	{
		m_psTarget = new CString("ps_6_1");
		m_vsTarget = new CString("vs_6_1");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2:
	{
		m_psTarget = new CString("ps_6_2");
		m_vsTarget = new CString("vs_6_2");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_3:
	{
		m_psTarget = new CString("ps_6_3");
		m_vsTarget = new CString("vs_6_3");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4:
	{
		m_psTarget = new CString("ps_6_4");
		m_vsTarget = new CString("vs_6_4");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_5:
	{
		m_psTarget = new CString("ps_6_5");
		m_vsTarget = new CString("vs_6_5");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_6:
	{
		m_psTarget = new CString("ps_6_6");
		m_vsTarget = new CString("vs_6_6");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_7:
	{
		m_psTarget = new CString("ps_6_7");
		m_vsTarget = new CString("vs_6_7");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_8:
	{
		m_psTarget = new CString("ps_6_8");
		m_vsTarget = new CString("vs_6_8");

		break;
	}
	case D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_9:
	{
		m_psTarget = new CString("ps_6_9");
		m_vsTarget = new CString("vs_6_9");

		break;
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

	CListNode* node = m_shaderBinaries->m_list;

	while ((node) && (node->m_object))
	{
		CShaderBinary* shaderBinary = (CShaderBinary*)node->m_object;

		SAFE_DELETE(shaderBinary);

		node = m_shaderBinaries->Delete(node);
	}

	SAFE_DELETE(m_shaderBinaries);
}

/*
*/
CShaderBinary* CShaderBinaryManager::Create(const char* installPath, const char* name, BYTE type)
{
	CShaderBinary* shaderBinary = CShaderBinaryManager::Get(name);

	if (shaderBinary)
	{
		return shaderBinary;
	}

	m_errorLog->WriteError(true, "CShaderBinaryManager::Create:%s\n", name);

	CString* filename = new CString(m_shaderDirectory->m_text);

	filename->Append(name);

	switch (type)
	{
	case CShaderBinaryManager::BinaryType::PIXEL:
	{
		shaderBinary = new CShaderBinary(m_errorLog, m_shaderModel.HighestShaderModel, m_psEntry->m_text, m_psTarget->m_text, installPath, m_shaderDirectory->m_text, name, filename->m_text);

		break;
	}
	case CShaderBinaryManager::BinaryType::VERTEX:
	{
		shaderBinary = new CShaderBinary(m_errorLog, m_shaderModel.HighestShaderModel, m_vsEntry->m_text, m_vsTarget->m_text, installPath, m_shaderDirectory->m_text, name, filename->m_text);

		break;
	}
	}

	SAFE_DELETE(filename);

	if (shaderBinary->m_isInitialized)
	{
		m_shaderBinaries->Add(shaderBinary, name);

		return shaderBinary;
	}

	SAFE_DELETE(shaderBinary);

	m_errorLog->WriteError(true, "CShaderBinaryManager::Create::Failed\n");

	return nullptr;
}

/*
*/
void CShaderBinaryManager::Delete(const char* name)
{
	CListNode* node = m_shaderBinaries->Search(name);

	if ((node) && (node->m_object))
	{
		CShaderBinary* shaderBinary = (CShaderBinary*)node->m_object;

		SAFE_DELETE(shaderBinary);

		m_shaderBinaries->Delete(node);
	}
}

/*
*/
CShaderBinary* CShaderBinaryManager::Get(const char* name)
{
	CListNode* node = m_shaderBinaries->Search(name);

	if ((node) && (node->m_object))
	{
		return (CShaderBinary*)node->m_object;
	}

	return nullptr;
}