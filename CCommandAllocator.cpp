#include "CCommandAllocator.h"

/*
*/
CCommandAllocator::CCommandAllocator()
{
	memset(this, 0x00, sizeof(CCommandAllocator));
}

/*
*/
CCommandAllocator::CCommandAllocator(CGraphicsAdapter* graphicsAdapter)
{
	memset(this, 0x00, sizeof(CCommandAllocator));

	m_graphicsAdapter = graphicsAdapter;

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
	}
}

/*
*/
CCommandAllocator::~CCommandAllocator()
{
	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocator[i].Reset();
	}
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