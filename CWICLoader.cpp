#include "CWICLoader.h"

/*
*/
CWICLoader::CWICLoader()
{
	memset(this, 0x00, sizeof(CWICLoader));
}

/*
*/
CWICLoader::CWICLoader(const char* filename)
{
	memset(this, 0x00, sizeof(CWICLoader));

	m_name = new CString(filename);

	m_hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&m_factory);

	m_hr = m_factory->CreateDecoderFromFilename(m_name->GetWide(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_decoder);

	m_hr = m_decoder->GetFrameCount((uint32_t*)&m_frames);

	if (m_frames > 0)
	{
		m_hr = m_decoder->GetFrame(0, &m_bitmapFrame);

		if (m_hr == S_OK)
		{
			m_bitmapFrame->GetSize((uint32_t*)&m_width, (uint32_t*)&m_height);

			m_wicRect.Width = m_width;
			m_wicRect.Height = m_height;

			m_pixels32 = new BYTE[m_width * m_height * 4]();

			if (m_pixels32)
			{
				m_bitmapFrame->GetPixelFormat(&m_pixelFormatGUID);

				m_hr = m_factory->CreateBitmapFromSource(m_bitmapFrame, WICBitmapCacheOnDemand, &m_bitmap);

				m_hr = m_bitmap->Lock(&m_wicRect, WICBitmapLockRead, &m_pixels);

				m_hr = m_pixels->GetDataPointer((uint32_t*)&m_cbBufferSize, &m_pixelBuffer);

				m_pixel = m_pixelBuffer;

				for (int32_t i = 0; i < m_width * m_height * 4; i += 4)
				{
					m_pixels32[i + 2] = *m_pixel;
					m_pixel++;
					m_pixels32[i + 1] = *m_pixel;
					m_pixel++;
					m_pixels32[i + 0] = *m_pixel;
					m_pixel++;

					if (m_pixelFormatGUID == GUID_WICPixelFormat32bppBGRA)
					{
						m_pixels32[i + 3] = *m_pixel;

						m_pixel++;
					}
					else if (m_pixelFormatGUID == GUID_WICPixelFormat24bppBGR)
					{
						m_pixels32[i + 3] = 0xFF;
					}
				}

				m_isInitialized = true;
			}

			SAFE_RELEASE(m_pixels);
		}
	}
}

/*
*/
CWICLoader::~CWICLoader()
{
	SAFE_DELETE_ARRAY(m_pixels32);

	SAFE_RELEASE(m_bitmapFrame);
	SAFE_RELEASE(m_decoder);
	SAFE_RELEASE(m_factory);

	SAFE_DELETE(m_name);
}