#pragma once

#include "framework.h"

class CWavFileHeader
{
public:

	BYTE m_audioFormat[2];
	BYTE m_bitsPerSample[2];
	BYTE m_blockAlign[2];
	BYTE m_chunkID[4];
	BYTE m_format[4];
	BYTE m_numChannels[2];
	BYTE m_subchunk1ID[4];
	BYTE m_subchunk2ID[4];

	int m_chunkSize;
	int m_subchunk1Size;
	int m_subchunk2Size;
	int m_byteRate;
	int m_sampleRate;

	CWavFileHeader();
	~CWavFileHeader();
};