#include "CXAudio2Error.h"

/*
*/
CXAudio2Error::CXAudio2Error()
{
	memset(this, 0x00, sizeof(CXAudio2Error));

	m_defaultError = new CXAudio2ErrorItem(0x00000000, "XUDIO2_UKNOWN_ERROR",
		"There is something wrong with the parameters of the call.");

	m_errors = new CLinkList<CXAudio2ErrorItem>();

	CXAudio2ErrorItem* xaudio2ei = new CXAudio2ErrorItem(0x88960001, "XAUDIO2_E_INVALID_CALL",
		"Returned by XAudio2 for certain API usage errors (invalid calls and so on) that are hard to avoid completely and should be handled by a title at runtime. (API usage errors that are completely avoidable, such as invalid parameters, cause an ASSERT in debug builds and undefined behavior in retail builds, so no error code is defined for them.)");
	CXAudio2Error::AddItem(xaudio2ei);
	
	xaudio2ei = new CXAudio2ErrorItem(0x88960002, "XAUDIO2_E_XMA_DECODER_ERROR",
		"The Xbox 360 XMA hardware suffered an unrecoverable error.");
	CXAudio2Error::AddItem(xaudio2ei);

	xaudio2ei = new CXAudio2ErrorItem(0x88960003, "XAUDIO2_E_XAPO_CREATION_FAILED",
		"An effect failed to instantiate.");
	CXAudio2Error::AddItem(xaudio2ei);

	xaudio2ei = new CXAudio2ErrorItem(0x88960004, "XAUDIO2_E_DEVICE_INVALIDATED",
		"An audio device became unusable through being unplugged or some other event.");
	CXAudio2Error::AddItem(xaudio2ei);
}

/*
*/
CXAudio2Error::~CXAudio2Error()
{
	delete m_defaultError;

	delete m_errors;
}

/*
*/
void CXAudio2Error::AddItem(CXAudio2ErrorItem* item)
{
	m_errors->Add(item, item->m_nbr);
}

/*
*/
CXAudio2ErrorItem* CXAudio2Error::GetError(int error)
{
	CLinkListNode<CXAudio2ErrorItem>* lln = m_errors->Search(error);

	if (lln)
	{
		return lln->m_object;
	}

	return m_defaultError;
}