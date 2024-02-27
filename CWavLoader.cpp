#include "CWavLoader.h"

/*
*/
CWavLoader::CWavLoader()
{
	memset(this, 0x00, sizeof(CWavLoader));
}

/*
*/
CWavLoader::CWavLoader(CErrorLog* errorLog, const char* filename)
{
	memset(this, 0x00, sizeof(CWavLoader));

	m_errorLog = errorLog;

	m_filename = new CString(filename);

	FILE* file = {};

	errno_t err = fopen_s(&file, filename, "rb");

	if (err)
	{
		m_errorLog->WriteError(true, "CWavLoader::CWavLoader::Error:%i:%s\n", err, filename);

		return;
	}

	size_t bytesRead;

	bytesRead = fread_s(&m_header.m_chunkID, 4 * sizeof(BYTE), sizeof(BYTE), 4, file);
	bytesRead = fread_s(&m_header.m_chunkSize, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_header.m_format, 4 * sizeof(BYTE), sizeof(BYTE), 4, file);
	bytesRead = fread_s(&m_header.m_subchunk1ID, 4 * sizeof(BYTE), sizeof(BYTE), 4, file);
	bytesRead = fread_s(&m_header.m_subchunk1Size, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_header.m_audioFormat, 2 * sizeof(BYTE), sizeof(BYTE), 2, file);
	bytesRead = fread_s(&m_header.m_numChannels, 2 * sizeof(BYTE), sizeof(BYTE), 2, file);
	bytesRead = fread_s(&m_header.m_sampleRate, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_header.m_byteRate, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_header.m_blockAlign[2], 2 * sizeof(BYTE), sizeof(BYTE), 2, file);
	bytesRead = fread_s(&m_header.m_bitsPerSample, 2 * sizeof(BYTE), sizeof(BYTE), 2, file);

	if (m_header.m_subchunk1Size > 16)
	{
		char garbage[32];

		bytesRead = fread_s(&garbage, 32, ((size_t)m_header.m_subchunk1Size - 16), 1, file);
	}

	bytesRead = fread_s(&m_header.m_subchunk2ID, 4 * sizeof(BYTE), sizeof(BYTE), 4, file);
	
	// NumSamples * NumChannels * BitsPerSample / 8 size of the read of the subchunk following this number
	bytesRead = fread_s(&m_header.m_subchunk2Size, sizeof(int), sizeof(int), 1, file);

	m_data = (BYTE*)malloc(m_header.m_subchunk2Size);

	if (m_data)
	{
		bytesRead = fread_s(m_data, m_header.m_subchunk2Size * sizeof(BYTE), sizeof(BYTE), (size_t)m_header.m_subchunk2Size, file);
	}

	fclose(file);

	m_wfx.wFormatTag = m_header.m_audioFormat[0];
	m_wfx.nChannels = m_header.m_numChannels[0];
	m_wfx.nSamplesPerSec = m_header.m_sampleRate;
	m_wfx.nAvgBytesPerSec = m_header.m_byteRate;
	m_wfx.wBitsPerSample = m_header.m_bitsPerSample[0];
	m_wfx.nBlockAlign = (m_wfx.nChannels * m_wfx.wBitsPerSample) / 8;
	m_wfx.cbSize = 0;
}

/*
*/
CWavLoader::~CWavLoader()
{
	delete m_filename;

	if (m_data)
	{
		free(m_data);
	}
}