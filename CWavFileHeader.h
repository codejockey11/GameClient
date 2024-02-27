#pragma once

#include "framework.h"

class CWavFileHeader
{
public:

	BYTE m_chunkID[4]; // Contains the letters "RIFF" in ASCII form
	int  m_chunkSize;

	BYTE m_format[4]; // Contains the letters "WAVE"

	BYTE m_subchunk1ID[4]; // Contains the letters "fmt "
	int  m_subchunk1Size; // 16 for PCM.This is the size of the rest of the Subchunk which follows this number.

	BYTE m_audioFormat[2]; // PCM = 1 (i.e.Linear quantization)

	BYTE m_numChannels[2];

	int m_sampleRate;

	int m_byteRate;

	BYTE m_blockAlign[2];

	BYTE m_bitsPerSample[2];

	BYTE m_subchunk2ID[4]; // Contains the letters "data" (0x64617461 big - endian form).
	int  m_subchunk2Size; // NumSamples * NumChannels * BitsPerSample / 8 size	of the read of the subchunk following this number


	CWavFileHeader();
	~CWavFileHeader();
};