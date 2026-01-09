#include "CCommandAllocator.h"

/*
*/
CCommandAllocator::CCommandAllocator()
{
	memset(this, 0x00, sizeof(CCommandAllocator));
}

/*
*/
CCommandAllocator::CCommandAllocator(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t bufferCount)
{
	memset(this, 0x00, sizeof(CCommandAllocator));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_bufferCount = bufferCount;

	m_commandAllocator = new ComPtr<ID3D12CommandAllocator>[m_bufferCount]();

	for (int32_t i = 0; i < m_bufferCount; i++)
	{
		m_hr = m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
			__uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator[i]);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteComErrorMessage(true, "CCommandAllocator::CCommandAllocator:", m_hr);
		}
	}
}

/*
*/
CCommandAllocator::~CCommandAllocator()
{
	for (int32_t i = 0; i < m_bufferCount; i++)
	{
		m_commandAllocator[i].Reset();
	}

	SAFE_DELETE_ARRAY(m_commandAllocator);
}

/*
*/
ComPtr<ID3D12GraphicsCommandList> CCommandAllocator::CreateCommandList()
{
	m_hr = m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr,
		__uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CCommandAllocator::CreateCommandList:", m_hr);
	}

	m_commandList->Close();

	return m_commandList;
}

/*
*/
ID3D12CommandAllocator* CCommandAllocator::Get()
{
	return m_commandAllocator[m_graphicsAdapter->m_backbufferIndex].Get();
}

/*
*/
void CCommandAllocator::Reset()
{
	m_commandAllocator[m_graphicsAdapter->m_backbufferIndex]->Reset();
}