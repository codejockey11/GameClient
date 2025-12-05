#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CVideoDevice
{
public:

	enum
	{
		E_MAX_GUID_STRING_LENGTH = 39
	};
	
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_gtos[E_MAX_GUID_STRING_LENGTH];

	ComPtr<ID3D12CommandAllocator> m_decoderCommandAllocator;
	ComPtr<ID3D12CommandAllocator> m_processorCommandAllocator;
	ComPtr<ID3D12Resource> m_mpegBuffer;
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12VideoDecodeCommandList> m_decoderCommandList;
	ComPtr<ID3D12VideoDecoder> m_videoDecoder;
	ComPtr<ID3D12VideoDecoderHeap> m_videoDecoderHeap;
	ComPtr<ID3D12VideoDevice> m_videoDevice;
	ComPtr<ID3D12VideoProcessCommandList> m_processorCommandList;
	ComPtr<ID3D12VideoProcessor> m_videoProcessor;

	D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT* m_videoDecoderFormatCounts;
	D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS* m_videoDecoderFormats;
	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT m_videoDecoderProfileCount;
	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES m_videoDecoderProfiles;

	D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS m_videoProcessMaxInputStreams;
	D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT m_videoProcessSupport;

	D3D12_HEAP_PROPERTIES m_mpegBufferHeapProperties;
	D3D12_HEAP_PROPERTIES m_textureHeapProperties;

	D3D12_RESOURCE_DESC m_mpegBufferDesc;
	D3D12_RESOURCE_DESC m_textureDesc;

	D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM m_videoDecodeCompressedBitstream;
	D3D12_VIDEO_DECODE_CONFIGURATION m_configuration;
	D3D12_VIDEO_DECODE_CONFIGURATION* m_videoDecoderConfigurations;
	D3D12_VIDEO_DECODE_FRAME_ARGUMENT m_videoDecodeFrameArgument;
	D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS m_videoDecoderInputStreamArguments;
	D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS m_videoDecoderOutputStreamArguments;

	D3D12_VIDEO_DECODER_DESC m_videoDecoderDesc;
	D3D12_VIDEO_DECODER_HEAP_DESC m_videoDecoderHeapDesc;

	D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS m_videoProcessInputStreamArguments;
	D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC m_videoProcessInputStreamDesc;
	D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS m_videoProcessOutputStreamArguments;
	D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC m_videoProcessOutputStreamDesc;

	DXGI_FORMAT m_format;

	DXGI_FORMAT* m_DXGIFormats;

	GUID* m_guids;

	HRESULT m_hr;

	int32_t m_inputStreamDescCount;

	CVideoDevice();
	CVideoDevice(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, GUID guid);
	~CVideoDevice();

	void CheckDecoderSupport();
	void CheckProcessorSupport();
	void CreateHeapForProfile();
	void CreateVideoDecoder();
	void CreateVideoProcessor();
	char* GetProfileName(GUID g);
	void RecordDecoder();
	void RecordProcessor();
};