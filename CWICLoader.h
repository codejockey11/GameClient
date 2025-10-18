#pragma once

#include "framework.h"

#include "../GameCommon/CString.h"

class CWICLoader
{
public:

	IWICBitmapDecoder* m_decoder;
	IWICBitmapFrameDecode* m_bitmapFrame;
	IWICImagingFactory* m_factory;
	IWICStream* m_stream;
	WICPixelFormatGUID m_pixelFormatGUID;

	CString* m_name;

	bool m_isInitialized;

	HRESULT m_hr;

	unsigned char* m_pixels32;

	unsigned int m_frames;
	unsigned int m_height;
	unsigned int m_width;

	WICRect m_wicRect;

	CWICLoader();
	CWICLoader(char* filename);
	~CWICLoader();
};