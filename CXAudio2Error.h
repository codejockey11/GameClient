#pragma once

#include "framework.h"

#include "CXAudio2ErrorItem.h"
#include "CLinkList.h"

class CXAudio2Error
{
public:

	CLinkList<CXAudio2ErrorItem>* m_errors;

	CXAudio2ErrorItem* m_defaultError;

	CXAudio2Error();
	~CXAudio2Error();

	void AddItem(CXAudio2ErrorItem* item);
	CXAudio2ErrorItem* GetError(int error);
};

