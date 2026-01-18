#include "CVisibility.h"

/*
*/
CVisibility::CVisibility()
{
	memset(this, 0x00, sizeof(CVisibility));
}

/*
*/
CVisibility::CVisibility(int32_t width, int32_t depth, int32_t height, int32_t sectorSize)
{
	memset(this, 0x00, sizeof(CVisibility));

	m_sector = new CSector(width, depth, height, sectorSize);

	m_collectables = new CHeapArray(true, sizeof(CList), 3, m_sector->m_gridWidth, m_sector->m_gridDepth, m_sector->m_gridHeight);

	m_statics = new CHeapArray(true, sizeof(CList), 3, m_sector->m_gridWidth, m_sector->m_gridDepth, m_sector->m_gridHeight);
}

/*
*/
CVisibility::~CVisibility()
{
	for (int32_t y = 0; y < m_sector->m_gridHeight; y++)
	{
		for (int32_t z = 0; z < m_sector->m_gridDepth; z++)
		{
			for (int32_t x = 0; x < m_sector->m_gridWidth; x++)
			{
				m_list = (CList*)m_collectables->GetElement(3, x, z, y);

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

				m_list = (CList*)m_statics->GetElement(3, x, z, y);

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
	}

	SAFE_DELETE(m_statics);
	SAFE_DELETE(m_collectables);
	SAFE_DELETE(m_sector);
}