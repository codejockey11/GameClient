#include "CCommandListGroup.h"

/*
*/
CCommandListGroup::CCommandListGroup()
{
	memset(this, 0x00, sizeof(CCommandListGroup));
}

/*
*/
CCommandListGroup::CCommandListGroup(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CCommandListGroup));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;
}

/*
*/
CCommandListGroup::~CCommandListGroup()
{

}

/*
*/
void CCommandListGroup::Add(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_commandLists[m_count] = commandList;

	m_commandListPointers[m_count] = commandList.Get();

	m_count++;

	if (m_count == CCommandListGroup::E_MAX_LISTS)
	{
		m_errorLog->WriteError(true, "CCommandListGroup::Add::Command list at maximum\n");
	}
}

/*
*/
void CCommandListGroup::Clear()
{
	m_count = 0;
}

/*
*/
void CCommandListGroup::Close()
{
	for (int32_t i = 0; i < m_count; i++)
	{
		m_commandLists[i]->Close();
	}
}

/*
*/
void CCommandListGroup::ExecuteCommandLists()
{
	CCommandListGroup::Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(m_count, (ID3D12CommandList* const*)m_commandListPointers);
}