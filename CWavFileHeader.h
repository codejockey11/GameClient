#pragma once

#include "framework.h"

class CWavFileHeader
{
public:

	BYTE m_chunkID[4];
	int  m_chunkSize;

	BYTE m_format[4];

	BYTE m_subchunk1ID[4];
	int  m_subchunk1Size;

	BYTE m_audioFormat[2];

	BYTE m_numChannels[2];

	int m_sampleRate;

	int m_byteRate;

	BYTE m_blockAlign[2];

	BYTE m_bitsPerSample[2];

	BYTE m_subchunk2ID[4];
	int  m_subchunk2Size;


	CWavFileHeader();
	~CWavFileHeader();
};