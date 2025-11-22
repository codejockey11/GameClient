#include "CBundle.h"

/*
*/
CBundle::CBundle()
{
	memset(this, 0x00, sizeof(CBundle));
}

/*
*/
CBundle::CBundle(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CBundle));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);
}

/*
*/
CBundle::~CBundle()
{
	for (int32_t i = 0; i < m_count; i++)
	{
		m_commandLists[i].Reset();
	}

	SAFE_DELETE(m_commandAllocator);
}

/*
*/
void CBundle::CloseAndExecute()
{
	for (int32_t i = 0; i < m_count; i++)
	{
		m_commandLists[i]->Close();
	}

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(m_count, m_commandListPointers);
}

/*
*/
ComPtr<ID3D12CommandList> CBundle::CreateCommandList()
{
	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->Close();

	m_commandLists[m_count] = m_commandList;

	m_commandListPointers[m_count] = m_commandList.Get();

	m_count++;

	if (m_count == CCommandListBundle::E_MAX_COMMAND_LIST_COUNT)
	{
		m_errorLog->WriteError(true, "CBundle::CreateCommandList:Command list count at maximum\n");
	}

	return m_commandList;
}

/*
*/
void CBundle::Reset()
{
	m_commandAllocator->Reset();

	for (int32_t i = 0; i < m_count; i++)
	{
		m_commandLists[i]->Reset(m_commandAllocator->Get(), nullptr);
	}
}