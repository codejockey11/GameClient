#include "CVisibility.h"

/*
*/
CVisibility::CVisibility()
{
	memset(this, 0x00, sizeof(CVisibility));
}

/*
*/
CVisibility::CVisibility(UINT width, UINT height, UINT vertical, UINT sectorSize)
{
	memset(this, 0x00, sizeof(CVisibility));

	m_sector = new CSector(width, height, vertical, sectorSize);

	m_collectables = new CHeapArray(sizeof(CLinkList<CObject>), true, true, 2, m_sector->m_gridWidth, m_sector->m_gridHeight);

	m_statics = new CHeapArray(sizeof(CLinkList<CObject>), true, true, 2, m_sector->m_gridWidth, m_sector->m_gridHeight);
}

/*
*/
CVisibility::~CVisibility()
{
	for (UINT pz = 0; pz < m_sector->m_gridHeight; pz++)
	{
		for (UINT px = 0; px < m_sector->m_gridWidth; px++)
		{
			CLinkList<CObject>* collectables = (CLinkList<CObject>*)m_collectables->GetElement(2, px, pz);

			if (collectables->m_list != nullptr)
			{
				CLinkListNode<CObject>* lln = collectables->m_list;

				while (lln->m_object)
				{
					delete lln->m_object;

					lln = lln->m_next;
				}
			}

			CLinkList<CObject>* statics = (CLinkList<CObject>*)m_statics->GetElement(2, px, pz);

			if (statics->m_list != nullptr)
			{
				CLinkListNode<CObject>* lln = statics->m_list;

				while (lln->m_object)
				{
					delete lln->m_object;

					lln = lln->m_next;
				}
			}
		}
	}

	delete m_statics;
	delete m_collectables;
	delete m_sector;
}