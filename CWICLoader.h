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

	BYTE* m_pixels32;

	int32_t m_frames;
	int32_t m_height;
	int32_t m_width;

	WICPixelFormatGUID m_pixelFormatGUID;

	WICRect m_wicRect;
	IWICBitmap* m_bitmap = NULL;
	IWICBitmapLock* m_pixels = NULL;
	int32_t m_cbBufferSize = 0;
	BYTE* m_pixelBuffer = NULL;
	BYTE* m_pixel;



	CWICLoader();
	CWICLoader(const char* filename);
	~CWICLoader();
};