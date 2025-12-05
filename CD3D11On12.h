#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CD3D11On12
{
public:

	CErrorLog* m_errorLog;

	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1DeviceContext1> m_d2dDeviceContext;
	ComPtr<ID2D1Factory3> m_d2dFactory;
#ifdef _DEBUG
	ComPtr<ID3D11Debug> m_d3d11Debug;
#endif
	ComPtr<ID3D11Device> m_d3d11Device;
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDWriteFactory> m_dWriteFactory;
	ComPtr<IDXGIDevice> m_dxgiDevice;

	D2D1_DEVICE_CONTEXT_OPTIONS m_deviceOptions;
	D2D1_FACTORY_OPTIONS m_d2dFactoryOptions;

	int32_t m_d3d11DeviceFlags;

	CD3D11On12();
	CD3D11On12(CErrorLog* errorLog, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue);
	~CD3D11On12();
};