#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CDXGINames
{
public:

	struct DXGINames
	{
		char s_name[64];

		DXGI_FORMAT s_number;
	};

	DXGINames* m_DXGINames;

	int32_t m_maxNames;

	CDXGINames();
	~CDXGINames();

	char* GetDXGIFormatName(DXGI_FORMAT format);
};