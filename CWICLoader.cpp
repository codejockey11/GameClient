#include "CWICLoader.h"

/*
*/
CWICLoader::CWICLoader()
{
    memset(this, 0x00, sizeof(CWICLoader));
}

/*
*/
CWICLoader::CWICLoader(char* filename)
{
	memset(this, 0x00, sizeof(CWICLoader));

    m_name = new CString(filename);

    m_hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_factory));

    m_hr = m_factory->CreateStream(&m_stream);

    m_hr = m_factory->CreateDecoder(GUID_ContainerFormatPng, NULL, &m_decoder);

    m_hr = m_stream->InitializeFromFilename(m_name->GetWide(), GENERIC_READ);

    m_hr = m_decoder->Initialize(m_stream, WICDecodeMetadataCacheOnDemand);

    m_hr = m_decoder->GetFrameCount(&m_frames);

    if (m_frames > 0)
    {
        m_hr = m_decoder->GetFrame(0, &m_bitmapFrame);

        if (m_hr == S_OK)
        {
            m_bitmapFrame->GetSize(&m_width, &m_height);

            m_bitmapFrame->GetPixelFormat(&m_pixelFormatGUID);

            m_pixels32 = new unsigned char[m_width * m_height * 4]();
            
            m_wicRect.Width = m_width;
            m_wicRect.Height = m_height;

            if (m_pixels32)
            {
                m_bitmapFrame->CopyPixels(&m_wicRect, m_width * 4, m_width * m_height * 4, m_pixels32);

                if (m_pixelFormatGUID == GUID_WICPixelFormat32bppBGRA)
                {
                    for (unsigned int i = 0; i < m_width * m_height * 4; i += 4)
                    {
                        unsigned char p = m_pixels32[i];
                        m_pixels32[i] = m_pixels32[i + 2];
                        m_pixels32[i + 2] = p;
                    }
                }

                m_isInitialized = true;
            }
        }
    }
}

/*
*/
CWICLoader::~CWICLoader()
{
    delete[] m_pixels32;

    if (m_bitmapFrame)
    {
        m_bitmapFrame->Release();
    }

    if (m_decoder)
    {
        m_decoder->Release();
    }

    if (m_stream)
    {
        m_stream->Release();
    }

    if (m_factory)
    {
        m_factory->Release();
    }

    delete m_name;
}