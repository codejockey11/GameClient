#pragma once

#include "framework.h"

#include "../GameCommon/CString.h"

class CWICLoader
{
public:

	bool m_isInitialized;

	CString* m_name;

	HRESULT m_hr;

	IWICBitmapDecoder* m_decoder;
	IWICBitmapFrameDecode* m_bitmapFrame;

	IWICImagingFactory* m_factory;

	IWICStream* m_stream;

	unsigned char* m_pixels32;

	unsigned int m_frames;
	unsigned int m_height;
	unsigned int m_width;

	WICPixelFormatGUID m_pixelFormatGUID;

	WICRect m_wicRect;

	CWICLoader();
	CWICLoader(char* filename);
	~CWICLoader();
};