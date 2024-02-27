#pragma once

#include "framework.h"

#include "CString.h"

class CXAudio2ErrorItem
{
public:

	CString* m_msg;
	CString* m_name;

	UINT m_nbr;

	CXAudio2ErrorItem();
	CXAudio2ErrorItem(UINT nbr, const char* name, const char* message);
	~CXAudio2ErrorItem();
};

