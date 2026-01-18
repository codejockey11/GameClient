#include "CVideoDevice.h"

/*
*/
CVideoDevice::CVideoDevice()
{
	memset(this, 0x00, sizeof(CVideoDevice));
}

/*
*/
CVideoDevice::CVideoDevice(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, GUID guid)
{
	memset(this, 0x00, sizeof(CVideoDevice));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_hr = m_graphicsAdapter->m_device->QueryInterface(__uuidof(ID3D12VideoDevice), (void**)&m_videoDevice);

	CVideoDevice::CheckDecoderSupport();
	CVideoDevice::CheckProcessorSupport();

	for (uint32_t i = 0; i < m_videoDecoderProfiles.ProfileCount; i++)
	{
		if (m_videoDecoderConfigurations[i].DecodeProfile == guid)
		{
			m_configuration = m_videoDecoderConfigurations[i];

			m_format = m_videoDecoderFormats[i].pOutputFormats[0];

			break;
		}

		if (i == (m_videoDecoderProfiles.ProfileCount - 1))
		{
			m_errorLog->WriteError(true, "CVideoDevice::CVideoDevice::No profiles found:%s\n", CVideoDevice::GetProfileName(guid));

			return;
		}
	}

	m_hr = m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
		__uuidof(ID3D12CommandAllocator), (void**)&m_decoderCommandAllocator);
	m_hr = m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
		__uuidof(ID3D12CommandAllocator), (void**)&m_processorCommandAllocator);

	m_hr = m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE, m_decoderCommandAllocator.Get(), nullptr,
		__uuidof(ID3D12VideoDecodeCommandList), (void**)&m_decoderCommandList);
	m_hr = m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS, m_processorCommandAllocator.Get(), nullptr,
		__uuidof(ID3D12VideoProcessCommandList), (void**)&m_processorCommandList);

	m_decoderCommandList->Close();
	m_processorCommandList->Close();

	m_textureDesc.MipLevels = 1;
	m_textureDesc.Format = m_graphicsAdapter->m_backBufferFormat;
	m_textureDesc.Width = m_graphicsAdapter->m_width;
	m_textureDesc.Height = m_graphicsAdapter->m_height;
	m_textureDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	m_textureDesc.DepthOrArraySize = 1;
	m_textureDesc.SampleDesc.Count = 1;
	m_textureDesc.SampleDesc.Quality = 0;
	m_textureDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_textureHeapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_textureHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_textureHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_textureHeapProperties.CreationNodeMask = 1;
	m_textureHeapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_textureHeapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_textureDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_texture);

	m_mpegBufferHeapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_mpegBufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_mpegBufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_mpegBufferHeapProperties.CreationNodeMask = 1;
	m_mpegBufferHeapProperties.VisibleNodeMask = 1;

	m_mpegBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_mpegBufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	m_mpegBufferDesc.Width = 1024;
	m_mpegBufferDesc.Height = 1;
	m_mpegBufferDesc.DepthOrArraySize = 1;
	m_mpegBufferDesc.MipLevels = 1;
	m_mpegBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_mpegBufferDesc.SampleDesc.Count = 1;
	m_mpegBufferDesc.SampleDesc.Quality = 0;
	m_mpegBufferDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_mpegBufferDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_mpegBufferHeapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_mpegBufferDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_mpegBuffer);

	m_errorLog->WriteError(true, "CVideoDevice::CVideoDevice::Profile Supported:%s\n", CVideoDevice::GetProfileName(guid));

	CVideoDevice::CreateVideoDecoder();
	CVideoDevice::CreateVideoProcessor();
	CVideoDevice::CreateHeapForProfile();

	//CVideoDevice::RecordDecoder();
	//CVideoDevice::RecordProcessor();
}

/*
*/
CVideoDevice::~CVideoDevice()
{
	for (uint32_t i = 0; i < m_videoDecoderProfiles.ProfileCount; i++)
	{
		SAFE_DELETE_ARRAY(m_videoDecoderFormats[i].pOutputFormats);
	}

	SAFE_DELETE_ARRAY(m_videoDecoderFormats);
	SAFE_DELETE_ARRAY(m_videoDecoderFormatCounts);
	SAFE_DELETE_ARRAY(m_videoDecoderConfigurations);
	SAFE_DELETE_ARRAY(m_guids);

	m_mpegBuffer.Reset();
	m_texture.Reset();
	m_processorCommandAllocator.Reset();
	m_processorCommandList.Reset();
	m_decoderCommandAllocator.Reset();
	m_decoderCommandList.Reset();
	m_videoDecoder.Reset();
	m_videoDecoderHeap.Reset();
	m_videoProcessor.Reset();
	m_videoDevice.Reset();
}

/*
*/
void CVideoDevice::CheckDecoderSupport()
{
	m_videoDecoderProfileCount = {};

	m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, (void*)&m_videoDecoderProfileCount, sizeof(m_videoDecoderProfileCount));

	m_videoDecoderProfiles = {};
	m_videoDecoderProfiles.ProfileCount = m_videoDecoderProfileCount.ProfileCount;

	m_guids = new GUID[m_videoDecoderProfileCount.ProfileCount]();
	m_videoDecoderConfigurations = new D3D12_VIDEO_DECODE_CONFIGURATION[m_videoDecoderProfileCount.ProfileCount]();
	m_videoDecoderFormatCounts = new D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT[m_videoDecoderProfileCount.ProfileCount]();
	m_videoDecoderFormats = new D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS[m_videoDecoderProfileCount.ProfileCount]();

	m_videoDecoderProfiles.pProfiles = m_guids;

	m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_PROFILES, (void*)&m_videoDecoderProfiles, sizeof(m_videoDecoderProfiles));

	m_errorLog->WriteError(true, "CVideoDevice::CheckDecoderSupport::D3D12_FEATURE_VIDEO_DECODE_FORMATS\n");

	for (uint32_t i = 0; i < m_videoDecoderProfiles.ProfileCount; i++)
	{
		m_videoDecoderConfigurations[i].DecodeProfile = m_guids[i];
		m_videoDecoderConfigurations[i].BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
		m_videoDecoderConfigurations[i].InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;

		m_videoDecoderFormatCounts[i].NodeIndex = 0;
		m_videoDecoderFormatCounts[i].Configuration = m_videoDecoderConfigurations[i];

		m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT, (void*)&m_videoDecoderFormatCounts[i], sizeof(m_videoDecoderFormatCounts[i]));

		m_videoDecoderFormats[i].NodeIndex = 0;
		m_videoDecoderFormats[i].Configuration = m_videoDecoderConfigurations[i];
		m_videoDecoderFormats[i].FormatCount = m_videoDecoderFormatCounts[i].FormatCount;

		m_DXGIFormats = new DXGI_FORMAT[m_videoDecoderFormats[i].FormatCount]();

		m_videoDecoderFormats[i].pOutputFormats = m_DXGIFormats;

		m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_FORMATS, (void*)&m_videoDecoderFormats[i], sizeof(m_videoDecoderFormats[i]));

		char* gname = CVideoDevice::GetProfileName(m_guids[i]);

		m_errorLog->WriteError(true, "Profile:%i %s\n", i, gname);

		for (uint32_t j = 0; j < m_videoDecoderFormats[i].FormatCount; j++)
		{
			char* n = m_graphicsAdapter->m_DXGINames->GetDXGIFormatName(m_DXGIFormats[j]);

			m_errorLog->WriteError(true, "Format:%i %s\n", j, n);
		}
	}
}

/*
*/
void CVideoDevice::CheckProcessorSupport()
{
	m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS, (void*)&m_videoProcessMaxInputStreams, sizeof(m_videoProcessMaxInputStreams));

	m_errorLog->WriteError(true, "CVideoDevice::CheckProcessorSupport::D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS:%i\n", m_videoProcessMaxInputStreams.MaxInputStreams);

	m_videoProcessSupport = {};

	m_videoProcessSupport.InputSample.Width = m_graphicsAdapter->m_width;
	m_videoProcessSupport.InputSample.Height = m_graphicsAdapter->m_height;
	m_videoProcessSupport.InputSample.Format.Format = m_graphicsAdapter->m_backBufferFormat;
	m_videoProcessSupport.InputSample.Format.ColorSpace = DXGI_COLOR_SPACE_TYPE::DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709;

	m_videoProcessSupport.InputFieldType = D3D12_VIDEO_FIELD_TYPE::D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST;
	m_videoProcessSupport.InputStereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT::D3D12_VIDEO_FRAME_STEREO_FORMAT_HORIZONTAL;
	m_videoProcessSupport.InputFrameRate.Numerator = 1;
	m_videoProcessSupport.InputFrameRate.Denominator = 1;

	m_videoProcessSupport.OutputFormat.Format = m_graphicsAdapter->m_backBufferFormat;
	m_videoProcessSupport.OutputFormat.ColorSpace = DXGI_COLOR_SPACE_TYPE::DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709;
	m_videoProcessSupport.OutputStereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT::D3D12_VIDEO_FRAME_STEREO_FORMAT_HORIZONTAL;
	m_videoProcessSupport.OutputFrameRate.Numerator = 1;
	m_videoProcessSupport.OutputFrameRate.Denominator = 1;

	m_hr = m_videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, (void*)&m_videoProcessSupport, sizeof(m_videoProcessSupport));
}

/*
*/
void CVideoDevice::CreateHeapForProfile()
{
	m_videoDecoderHeapDesc = {};

	m_videoDecoderHeapDesc.NodeMask = 0;
	m_videoDecoderHeapDesc.Configuration = m_configuration;
	m_videoDecoderHeapDesc.DecodeWidth = m_graphicsAdapter->m_width;
	m_videoDecoderHeapDesc.DecodeHeight = m_graphicsAdapter->m_height;
	m_videoDecoderHeapDesc.Format = m_format;
	m_videoDecoderHeapDesc.FrameRate.Numerator = 1;
	m_videoDecoderHeapDesc.FrameRate.Denominator = 1;
	m_videoDecoderHeapDesc.BitRate;
	m_videoDecoderHeapDesc.MaxDecodePictureBufferCount;

	m_hr = m_videoDevice->CreateVideoDecoderHeap(&m_videoDecoderHeapDesc, __uuidof(ID3D12VideoDecoderHeap), (void**)&m_videoDecoderHeap);
}

/*
*/
void CVideoDevice::CreateVideoDecoder()
{
	m_videoDecoderDesc = {};

	m_videoDecoderDesc.NodeMask = 0;
	m_videoDecoderDesc.Configuration = m_configuration;

	m_hr = m_videoDevice->CreateVideoDecoder(&m_videoDecoderDesc, __uuidof(ID3D12VideoDecoder), (void**)&m_videoDecoder);
}

/*
*/
void CVideoDevice::CreateVideoProcessor()
{
	m_videoProcessOutputStreamDesc = {};

	m_videoProcessOutputStreamDesc.Format = m_graphicsAdapter->m_backBufferFormat;
	m_videoProcessOutputStreamDesc.ColorSpace = DXGI_COLOR_SPACE_TYPE::DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709;
	m_videoProcessOutputStreamDesc.AlphaFillMode = D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE::D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE;
	m_videoProcessOutputStreamDesc.AlphaFillModeSourceStreamIndex = 0;
	m_videoProcessOutputStreamDesc.BackgroundColor[0] = 0.0f;
	m_videoProcessOutputStreamDesc.BackgroundColor[1] = 0.0f;
	m_videoProcessOutputStreamDesc.BackgroundColor[2] = 0.0f;
	m_videoProcessOutputStreamDesc.BackgroundColor[3] = 1.0f;
	m_videoProcessOutputStreamDesc.FrameRate = m_videoProcessSupport.OutputFrameRate;
	m_videoProcessOutputStreamDesc.EnableStereo = true;

	m_videoProcessInputStreamDesc = {};

	m_videoProcessInputStreamDesc.Format = m_graphicsAdapter->m_backBufferFormat;
	m_videoProcessInputStreamDesc.ColorSpace = DXGI_COLOR_SPACE_TYPE::DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709;
	m_videoProcessInputStreamDesc.SourceAspectRatio.Numerator = 16;
	m_videoProcessInputStreamDesc.SourceAspectRatio.Denominator = 9;
	m_videoProcessInputStreamDesc.DestinationAspectRatio.Numerator = 16;
	m_videoProcessInputStreamDesc.DestinationAspectRatio.Denominator = 9;
	m_videoProcessInputStreamDesc.FrameRate = m_videoProcessSupport.InputFrameRate;
	m_videoProcessInputStreamDesc.SourceSizeRange = m_videoProcessSupport.ScaleSupport.OutputSizeRange;
	m_videoProcessInputStreamDesc.DestinationSizeRange = m_videoProcessSupport.ScaleSupport.OutputSizeRange;
	m_videoProcessInputStreamDesc.EnableOrientation = false;
	m_videoProcessInputStreamDesc.FilterFlags = m_videoProcessSupport.FilterSupport;
	m_videoProcessInputStreamDesc.StereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT::D3D12_VIDEO_FRAME_STEREO_FORMAT_HORIZONTAL;
	m_videoProcessInputStreamDesc.FieldType = D3D12_VIDEO_FIELD_TYPE::D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST;
	m_videoProcessInputStreamDesc.DeinterlaceMode = m_videoProcessSupport.DeinterlaceSupport;
	m_videoProcessInputStreamDesc.EnableAlphaBlending = false;
	m_videoProcessInputStreamDesc.LumaKey = { false, 0.0f, 0.0f };
	m_videoProcessInputStreamDesc.NumPastFrames;
	m_videoProcessInputStreamDesc.NumFutureFrames;
	m_videoProcessInputStreamDesc.EnableAutoProcessing = m_videoProcessSupport.AutoProcessingSupport;

	m_inputStreamDescCount = 1;

	m_hr = m_videoDevice->CreateVideoProcessor(0, &m_videoProcessOutputStreamDesc, m_inputStreamDescCount, &m_videoProcessInputStreamDesc,
		__uuidof(ID3D12VideoProcessor), (void**)&m_videoProcessor);
}

/*
*/
char* CVideoDevice::GetProfileName(GUID g)
{
	struct GUIDNames
	{
		char name[64];
		GUID guid;
	};

	GUIDNames* l_GUIDNames;

	static GUIDNames GUIDNamesArray[] =
	{
		"D3D12_VIDEO_DECODE_PROFILE_MPEG1_AND_MPEG2", D3D12_VIDEO_DECODE_PROFILE_MPEG1_AND_MPEG2,
		"D3D12_VIDEO_DECODE_PROFILE_MPEG2", D3D12_VIDEO_DECODE_PROFILE_MPEG2,
		"D3D12_VIDEO_DECODE_PROFILE_H264", D3D12_VIDEO_DECODE_PROFILE_H264,
		"D3D12_VIDEO_DECODE_PROFILE_H264_STEREO_PROGRESSIVE", D3D12_VIDEO_DECODE_PROFILE_H264_STEREO_PROGRESSIVE,
		"D3D12_VIDEO_DECODE_PROFILE_H264_STEREO", D3D12_VIDEO_DECODE_PROFILE_H264_STEREO,
		"D3D12_VIDEO_DECODE_PROFILE_H264_MULTIVIEW", D3D12_VIDEO_DECODE_PROFILE_H264_MULTIVIEW,
		"D3D12_VIDEO_DECODE_PROFILE_VC1", D3D12_VIDEO_DECODE_PROFILE_VC1,
		"D3D12_VIDEO_DECODE_PROFILE_VC1_D2010", D3D12_VIDEO_DECODE_PROFILE_VC1_D2010,
		"D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_SIMPLE", D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_SIMPLE,
		"D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_ADVSIMPLE_NOGMC", D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_ADVSIMPLE_NOGMC,
		"D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN", D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN,
		"D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10", D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10,
		"D3D12_VIDEO_DECODE_PROFILE_VP9", D3D12_VIDEO_DECODE_PROFILE_VP9,
		"D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2", D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2,
		"D3D12_VIDEO_DECODE_PROFILE_VP8", D3D12_VIDEO_DECODE_PROFILE_VP8,
		"D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0", D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0,
		"D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE1", D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE1,
		"D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE2", D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE2,
		"D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2", D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2,
		"D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2_420", D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2_420
	};

	l_GUIDNames = GUIDNamesArray;

	int32_t d = 0;

	for (d = 0; d < _countof(GUIDNamesArray); d++)
	{
		if (l_GUIDNames[d].guid == g)
		{
			return l_GUIDNames[d].name;

			break;
		}
	}

	if (d == _countof(GUIDNamesArray))
	{
		m_errorLog->WriteError(true, "GUID:%i Unkown\n", g);
	}

	return nullptr;
}

/*
*/
void CVideoDevice::RecordDecoder()
{
	m_decoderCommandAllocator->Reset();

	m_decoderCommandList->Reset(m_decoderCommandAllocator.Get());

	m_videoDecoderOutputStreamArguments = {};

	m_videoDecoderOutputStreamArguments.pOutputTexture2D = m_texture.Get();
	m_videoDecoderOutputStreamArguments.OutputSubresource = 0;
	m_videoDecoderOutputStreamArguments.ConversionArguments.Enable = false;

	m_videoDecoderInputStreamArguments = {};

	m_videoDecodeFrameArgument = {};

	m_videoDecodeFrameArgument.Type = D3D12_VIDEO_DECODE_ARGUMENT_TYPE::D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS;

	m_videoDecoderInputStreamArguments.NumFrameArguments = 1;
	m_videoDecoderInputStreamArguments.FrameArguments[0] = m_videoDecodeFrameArgument;

	m_videoDecoderInputStreamArguments.ReferenceFrames.NumTexture2Ds = 1;
	m_videoDecoderInputStreamArguments.ReferenceFrames.ppTexture2Ds = m_texture.GetAddressOf();
	m_videoDecoderInputStreamArguments.ReferenceFrames.pSubresources = nullptr;
	m_videoDecoderInputStreamArguments.ReferenceFrames.ppHeaps = m_videoDecoderHeap.GetAddressOf();

	m_videoDecodeCompressedBitstream = {};

	void* pData;

	m_hr = m_mpegBuffer->Map(0, nullptr, &pData);

	m_mpegBuffer->Unmap(0, nullptr);

	m_videoDecodeCompressedBitstream.pBuffer = m_mpegBuffer.Get();
	m_videoDecodeCompressedBitstream.Offset = 0;
	m_videoDecodeCompressedBitstream.Size = 0;

	m_videoDecoderInputStreamArguments.CompressedBitstream = m_videoDecodeCompressedBitstream;
	m_videoDecoderInputStreamArguments.pHeap = m_videoDecoderHeap.Get();


	m_decoderCommandList->DecodeFrame(m_videoDecoder.Get(), &m_videoDecoderOutputStreamArguments, &m_videoDecoderInputStreamArguments);

	m_decoderCommandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_decoderCommandList.GetAddressOf());
}

/*
*/
void CVideoDevice::RecordProcessor()
{
	m_processorCommandAllocator->Reset();

	m_processorCommandList->Reset(m_processorCommandAllocator.Get());

	//m_processorCommandList->ProcessFrames(m_videoProcessor.Get(), &m_videoProcessOutputStreamArguments, m_inputStreamDescCount, &m_videoProcessInputStreamArguments);

	m_processorCommandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_processorCommandList.GetAddressOf());
}