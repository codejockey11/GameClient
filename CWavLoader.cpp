#include "CWavLoader.h"

/*
*/
CWavLoader::CWavLoader()
{
	memset(this, 0x00, sizeof(CWavLoader));
}

/*
*/
CWavLoader::CWavLoader(CErrorLog* errorLog, const char* name, const char* filename)
{
	memset(this, 0x00, sizeof(CWavLoader));

	m_errorLog = errorLog;

	m_name = new CString(name);

	m_err = fopen_s(&m_fWav, filename, "rb");

	if (m_err)
	{
		m_errorLog->WriteError(true, "CWavLoader::CWavLoader::Error:%i:%s\n", m_err, name);

		return;
	}

	fread_s(&m_header.m_chunkID, 4 * sizeof(BYTE), sizeof(BYTE), 4, m_fWav);
	fread_s(&m_header.m_chunkSize, sizeof(int32_t), sizeof(int32_t), 1, m_fWav);
	fread_s(&m_header.m_format, 4 * sizeof(BYTE), sizeof(BYTE), 4, m_fWav);
	fread_s(&m_header.m_subchunk1ID, 4 * sizeof(BYTE), sizeof(BYTE), 4, m_fWav);
	fread_s(&m_header.m_subchunk1Size, sizeof(int32_t), sizeof(int32_t), 1, m_fWav);
	fread_s(&m_header.m_audioFormat, 2 * sizeof(BYTE), sizeof(BYTE), 2, m_fWav);
	fread_s(&m_header.m_numChannels, 2 * sizeof(BYTE), sizeof(BYTE), 2, m_fWav);
	fread_s(&m_header.m_sampleRate, sizeof(int32_t), sizeof(int32_t), 1, m_fWav);
	fread_s(&m_header.m_byteRate, sizeof(int32_t), sizeof(int32_t), 1, m_fWav);
	fread_s(&m_header.m_blockAlign[2], 2 * sizeof(BYTE), sizeof(BYTE), 2, m_fWav);
	fread_s(&m_header.m_bitsPerSample, 2 * sizeof(BYTE), sizeof(BYTE), 2, m_fWav);

	if (m_header.m_subchunk1Size > 16)
	{
		fread_s(&m_garbage, 32, ((size_t)m_header.m_subchunk1Size - 16), 1, m_fWav);
	}

	fread_s(&m_header.m_subchunk2ID, 4 * sizeof(BYTE), sizeof(BYTE), 4, m_fWav);
	
	fread_s(&m_header.m_subchunk2Size, sizeof(int32_t), sizeof(int32_t), 1, m_fWav);

	m_data = new BYTE[m_header.m_subchunk2Size]();

	fread_s(m_data, m_header.m_subchunk2Size * sizeof(BYTE), sizeof(BYTE), (size_t)m_header.m_subchunk2Size, m_fWav);

	fclose(m_fWav);

	m_wfx.wFormatTag = m_header.m_audioFormat[0];
	m_wfx.nChannels = m_header.m_numChannels[0];
	m_wfx.nSamplesPerSec = m_header.m_sampleRate;
	m_wfx.nAvgBytesPerSec = m_header.m_byteRate;
	m_wfx.wBitsPerSample = m_header.m_bitsPerSample[0];
	m_wfx.nBlockAlign = (m_wfx.nChannels * m_wfx.wBitsPerSample) / 8;
	m_wfx.cbSize = 0;

	m_isInitialized = true;
}

/*
*/
CWavLoader::~CWavLoader()
{
	SAFE_DELETE_ARRAY(m_data);
	
	SAFE_DELETE(m_name);
}