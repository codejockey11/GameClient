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

	int32_t m_chunkSize;
	int32_t m_subchunk1Size;
	int32_t m_subchunk2Size;
	int32_t m_byteRate;
	int32_t m_sampleRate;

	CWavFileHeader();
	~CWavFileHeader();
};