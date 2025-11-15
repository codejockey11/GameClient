#include "CVisibility.h"

/*
*/
CVisibility::CVisibility()
{
	memset(this, 0x00, sizeof(CVisibility));
}

/*
*/
CVisibility::CVisibility(UINT width, UINT depth, UINT height, UINT sectorSize)
{
	memset(this, 0x00, sizeof(CVisibility));

	m_sector = new CSector(width, depth, height, sectorSize);

	m_collectables = new CHeapArray(true, sizeof(CList), 2, m_sector->m_gridWidth, m_sector->m_gridDepth);

	m_statics = new CHeapArray(true, sizeof(CList), 2, m_sector->m_gridWidth, m_sector->m_gridDepth);
}

/*
*/
CVisibility::~CVisibility()
{
	for (UINT pz = 0; pz < m_sector->m_gridDepth; pz++)
	{
		for (UINT px = 0; px < m_sector->m_gridWidth; px++)
		{
			m_list = (CList*)m_collectables->GetElement(2, px, pz);

			if ((m_list) && (m_list->m_list))
			{
				m_node = m_list->m_list;

				while ((m_node) && (m_node->m_object))
				{
					m_object = (CObject*)m_node->m_object;

					SAFE_DELETE(m_object);

					m_node = m_list->Delete(m_node);
				}
			}

			m_list = (CList*)m_statics->GetElement(2, px, pz);

			if ((m_list) && (m_list->m_list))
			{
				m_node = m_list->m_list;

				while ((m_node) && (m_node->m_object))
				{
					m_object = (CObject*)m_node->m_object;

					SAFE_DELETE(m_object);

					m_node = m_list->Delete(m_node);
				}
			}
		}
	}

	SAFE_DELETE(m_statics);
	SAFE_DELETE(m_collectables);
	SAFE_DELETE(m_sector);
}