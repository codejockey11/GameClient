#include "CGraphicsAdapter.h"

/*
*/
CGraphicsAdapter::CGraphicsAdapter()
{
	memset(this, 0x00, sizeof(CGraphicsAdapter));
}

/*
*/
CGraphicsAdapter::CGraphicsAdapter(CErrorLog* errorLog, HWND hWnd, int width, int height, bool windowed, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
{
	memset(this, 0x00, sizeof(CGraphicsAdapter));

	m_errorLog = errorLog;

	m_hWnd = hWnd;

	m_width = width;
	m_height = height;

	m_windowed = windowed;

	m_backBufferFormat = backBufferFormat;
	m_depthStencilFormat = depthStencilFormat;

	m_posX = (GetSystemMetrics(SM_CXSCREEN) - m_width) / 2;
	m_posY = ((GetSystemMetrics(SM_CYSCREEN) - m_height) / 2) - 20;

	GetWindowRect(GetDesktopWindow(), &m_desktop);

	m_middle.x = m_posX + (m_width / 2);
	m_middle.y = m_posY + (m_height / 2);

	m_dpi = (float)GetDpiForWindow(m_hWnd);

	m_widthRatio = ((float)m_desktop.right / (float)m_width);
	m_heightRatio = ((float)m_desktop.bottom / (float)m_height);

	m_vsyncEnabled = false;

	m_swapChainViewport = new CViewport(0.0f, 0.0f, (float)m_width, (float)m_height);

#if defined(_DEBUG)
	m_hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController));

	m_debugController->EnableDebugLayer();

	m_factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif	

	CGraphicsAdapter::DXGIFactory();
	CGraphicsAdapter::GetHardwareAdapter();
	CGraphicsAdapter::CreateDevice();

	// following section from github.com/jpvanoosten/LearningDirectX12/blob/master/Tutorial2/src/Application.cpp
#ifdef _DEBUG

	m_device->QueryInterface(IID_PPV_ARGS(&m_infoQueue));

	// Suppress whole categories of messages
	//D3D12_MESSAGE_CATEGORY Categories[] = {};

	D3D12_MESSAGE_SEVERITY Severities[] =
	{
		D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_INFO
	};

	// Suppress individual messages by their ID
	D3D12_MESSAGE_ID DenyIds[] =
	{
		D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE   // I'm really not sure how to avoid this message.
	};

	D3D12_INFO_QUEUE_FILTER NewFilter = {};

	//NewFilter.DenyList.NumCategories = _countof(Categories);
	//NewFilter.DenyList.pCategoryList = Categories;

	NewFilter.DenyList.NumSeverities = _countof(Severities);
	NewFilter.DenyList.pSeverityList = Severities;

	NewFilter.DenyList.NumIDs = _countof(DenyIds);
	NewFilter.DenyList.pIDList = DenyIds;

	m_infoQueue->PushStorageFilter(&NewFilter);
#endif

	m_sampleQualityLevels.Format = m_backBufferFormat;
	m_sampleQualityLevels.SampleCount = 4;
	m_sampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	m_sampleQualityLevels.NumQualityLevels = 0;

	m_hr = m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, (void*)&m_sampleQualityLevels, sizeof(m_sampleQualityLevels));

	if (m_hr == S_OK)
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::CreateDevice::D3D12_FEATURE::D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS:%i\n", m_sampleQualityLevels.NumQualityLevels);
	}
	else
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::CreateDevice::D3D12_FEATURE::D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS:None\n");
	}

	CGraphicsAdapter::CreateCommandQueue();
	CGraphicsAdapter::CreateSwapChain();
#ifdef DX11on12
	CGraphicsAdapter::Create11on12Device();
#endif
	CGraphicsAdapter::CreateRenderTargets();


	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"CGraphicsAdapter::CommandList");

	m_commandList->Close();


#ifndef DX11on12
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_eofCommandList));

	m_eofCommandList->SetName(L"CGraphicsAdapter::EOFCommandList");

	m_eofCommandList->Close();
#endif

	CGraphicsAdapter::CreateFence();

	for (int i = 0; i < CCommandListBundle::E_MAX_BUNDLES; i++)
	{
		m_commandListBundle[i] = new CCommandListBundle(m_errorLog);
	}

	m_descriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_color = new CColor();
}

/*
*/
CGraphicsAdapter::~CGraphicsAdapter()
{
	CGraphicsAdapter::WaitForGPU();

	for (int i = 0; i < CCommandListBundle::E_MAX_BUNDLES; i++)
	{
		delete m_commandListBundle[i];
	}

	delete m_adapterDescription;

	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, nullptr);
	}

	CloseHandle(m_fenceEvent);

	m_fenceEvent = 0;
	m_fence.Reset();

	m_commandList.Reset();
#ifndef DX11on12
	m_eofCommandList.Reset();
#endif

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();

		delete m_swapChainDepthBuffers[i];
		delete m_swapChainRenderTargets[i];
	}

#ifdef DX11on12
	delete m_d3d11On12;
#endif

	m_swapChain.Reset();
	m_commandQueue.Reset();
	m_infoQueue.Reset();
	m_device.Reset();
	m_adapterOutput.Reset();
	m_adapter.Reset();
	m_factory.Reset();

	delete m_swapChainViewport;

#ifdef _DEBUG
	m_debugController.Reset();

	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS::DXGI_DEBUG_RLO_DETAIL);
		pDebug->Release();
	}
#endif

	delete m_color;
}

/*
*/
void CGraphicsAdapter::BundleCommandList(int bundleNumber, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (bundleNumber >= CCommandListBundle::E_MAX_BUNDLES)
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::BundleCommandList::Bundle count exceeds maximum\n");

		return;
	}

	m_commandListBundle[bundleNumber]->Add(commandList);
}

/*
*/
void CGraphicsAdapter::Create11on12Device()
{
#ifdef DX11on12
	m_d3d11On12 = new CD3D11On12(m_errorLog, m_device, m_commandQueue);
#endif
}

/*
*/
void CGraphicsAdapter::CreateCommandQueue()
{
	m_commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	m_commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	m_commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	m_commandQueueDesc.NodeMask = 0;

	m_hr = m_device->CreateCommandQueue(&m_commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_commandQueue);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CommandQueue::CreateCommandQueue:", m_hr);
	}
}

/*
*/
void CGraphicsAdapter::CreateDevice()
{
	m_hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), (void**)&m_device);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CreateDevice::D3D12CreateDevice:", m_hr);
	}
}

/*
*/
void CGraphicsAdapter::CreateFence()
{
	m_hr = m_device->CreateFence(m_fenceValues[m_backbufferIndex], D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CreateFence::CreateFence:", m_hr);

		return;
	}

	m_fence->SetName(L"CGraphicsAdapter::Fence");

	m_fenceValues[m_backbufferIndex]++;

	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
}

/*
*/
void CGraphicsAdapter::CreateRenderTargets()
{
	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_swapChainRenderTargets[i] = new CRenderTarget(m_errorLog, m_device);

		m_swapChainRenderTargets[i]->SwapChainTarget(m_swapChain, i);

#ifdef DX11on12
		m_swapChainRenderTargets[i]->WrapResource(m_dpi, m_d3d11On12->m_d3d11On12Device, m_d3d11On12->m_d2dDeviceContext);
#endif

		m_swapChainDepthBuffers[i] = new CDepthBuffer(m_errorLog, m_device, m_width, m_height);

		m_swapChainDepthBuffers[i]->Create();
	}
}

/*
*/
void CGraphicsAdapter::CreateSwapChain()
{
	m_swapChain.Reset();

	m_swapChainDesc = {};

	m_swapChainDesc.BufferCount = CGraphicsAdapter::E_BACKBUFFER_COUNT;
	m_swapChainDesc.BufferDesc.Height = m_height;
	m_swapChainDesc.BufferDesc.Width = m_width;
	m_swapChainDesc.BufferDesc.Format = m_backBufferFormat;
	m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	m_swapChainDesc.OutputWindow = m_hWnd;
	m_swapChainDesc.Windowed = m_windowed;

	if (m_vsyncEnabled)
	{
		m_swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		m_swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	else
	{
		m_swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		m_swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	m_swapChainDesc.SampleDesc.Count = 1;
	m_swapChainDesc.SampleDesc.Quality = 0;
	m_swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	m_swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_hr = m_factory->CreateSwapChain(m_commandQueue.Get(), &m_swapChainDesc, (IDXGISwapChain**)m_swapChain.GetAddressOf());

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CreateSwapChain::CreateSwapChain:", m_hr);
	}
}

/*
*/
void CGraphicsAdapter::DXGIFactory()
{
	m_hr = CreateDXGIFactory2(m_factoryFlags, IID_IDXGIFactory4, (void**)&m_factory);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DXGIFactory::CreateDXGIFactory2:", m_hr);
	}
}

/*
*/
void CGraphicsAdapter::DeviceInformation()
{
	m_hr = m_adapter->GetDesc(&m_adapterDesc);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::GetDesc:", m_hr);

		return;
	}

	m_adapterDescription = new CString(m_adapterDesc.Description);

	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::Name:%s\n", m_adapterDescription->m_text);

	// 1M = 1,048,576 = 1024 * 1024
	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::Memory:%dM\n", (int)(m_adapterDesc.DedicatedVideoMemory / 1048576));


	m_hr = m_adapter->EnumOutputs(0, &m_adapterOutput);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::EnumOutputs:", m_hr);
	}

	m_hr = m_adapterOutput->GetDisplayModeList(m_backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &m_numModes, nullptr);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::GetDisplayModeList for numModes:", m_hr);
	}
	else
	{
		if (m_numModes > 0)
		{
			m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::DXGI_FORMAT:%s\n", CGraphicsAdapter::GetDXGIFormatName(m_backBufferFormat));

			m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::Number of modes:%i\n", m_numModes);

			// Warning C6385 Reading invalid data from 'm_displayModeList'.
			// + 1 removes the warning
			m_displayModeList = new DXGI_MODE_DESC[m_numModes + 1]();

			m_hr = m_adapterOutput->GetDisplayModeList(m_backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &m_numModes, m_displayModeList);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::GetDisplayModeList:", m_hr);
			}
			else
			{
				UINT prevWidth = 0;
				UINT prevHeight = 0;

				for (UINT i = 0; i < m_numModes; i++)
				{
					// Warning C6385
					if ((m_displayModeList[i].Width != prevWidth) && (m_displayModeList[i].Height != prevHeight))
					{
						if ((m_displayModeList[i].RefreshRate.Numerator / m_displayModeList[i].RefreshRate.Denominator) == 60)
						{
							m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::m:%003i w:%i h:%i n:%i d:%i hz:%i\n",
								i,
								m_displayModeList[i].Width,
								m_displayModeList[i].Height,
								m_displayModeList[i].RefreshRate.Numerator,
								m_displayModeList[i].RefreshRate.Denominator,
								m_displayModeList[i].RefreshRate.Numerator / m_displayModeList[i].RefreshRate.Denominator
							);

							prevWidth = m_displayModeList[i].Width;
							prevHeight = m_displayModeList[i].Height;
						}
					}
				}

				delete[] m_displayModeList;
			}
		}
	}


	m_hr = m_factory->CheckFeatureSupport(DXGI_FEATURE::DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_tearingSupport, (UINT)sizeof(UINT));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DXGIFactory::CheckFeatureSupport:", m_hr);
	}

	if (m_tearingSupport)
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::DXGIFactory::DXGI_FEATURE::DXGI_FEATURE_PRESENT_ALLOW_TEARING\n");
	}
	else
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::DXGIFactory::DXGI_FEATURE::DXGI_FEATURE_PRESENT_ALLOW_TEARING False\n");
	}
}

/*
*/
void CGraphicsAdapter::End2DDraw()
{
#ifdef DX11on12
	m_d3d11On12->m_d2dDeviceContext->EndDraw();

	m_d3d11On12->m_d3d11On12Device->ReleaseWrappedResources(m_swapChainRenderTargets[m_backbufferIndex]->m_wrappedBackBuffer.GetAddressOf(), 1);

	m_d3d11On12->m_d3d11DeviceContext->Flush();
#endif
}

/*
*/

void CGraphicsAdapter::ExecuteCommandLists()
{
	for (int i = 0; i < CCommandListBundle::E_MAX_BUNDLES; i++)
	{
		m_commandListBundle[i]->Close();

		m_commandQueue->ExecuteCommandLists(m_commandListBundle[i]->m_count, (ID3D12CommandList* const*)m_commandListBundle[i]->m_commandListPointers);
	}
}

/*
*/
void CGraphicsAdapter::GetHardwareAdapter()
{
	for (UINT ai = 0; ; ai++)
	{
		if (DXGI_ERROR_NOT_FOUND == m_factory->EnumAdapters1(ai, &m_adapter))
		{
			break;
		}

		m_hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);

		if (SUCCEEDED(m_hr))
		{
			m_errorLog->WriteError(true, "CGraphicsAdapter::GetHardwareAdapter:D3D12CreateDevice:D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 supported\n");

			CGraphicsAdapter::DeviceInformation();

			return;
		}
	}

	if (m_adapter == 0)
	{
		m_errorLog->WriteError(true, "CGraphicsAdapter::GetHardwareAdapter::D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 not supported\n");
	}
}

/*
*/
void CGraphicsAdapter::Init2DDraw()
{
#ifdef DX11on12
	m_d3d11On12->m_d3d11On12Device->AcquireWrappedResources(m_swapChainRenderTargets[m_backbufferIndex]->m_wrappedBackBuffer.GetAddressOf(), 1);

	m_d3d11On12->m_d2dDeviceContext->SetTarget(m_swapChainRenderTargets[m_backbufferIndex]->m_d2dRenderTarget.Get());
	m_d3d11On12->m_d2dDeviceContext->BeginDraw();
	m_d3d11On12->m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
#endif
}

/*
*/
void CGraphicsAdapter::MoveToNextFrame()
{
#ifndef DX11on12
	m_swapChainRenderTargets[m_backbufferIndex]->SetPostDrawBarrier();
	m_swapChainDepthBuffers[m_backbufferIndex]->SetPostDrawBarrier();

	m_eofCommandList->Reset(m_commandAllocators[m_backbufferIndex].Get(), nullptr);

	m_eofCommandList->ResourceBarrier(1, &m_swapChainRenderTargets[m_backbufferIndex]->m_barrier);

	m_eofCommandList->Close();

	m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_eofCommandList.GetAddressOf());
#endif

	if (m_vsyncEnabled)
	{
		m_swapChain->Present(1, 0);
	}
	else
	{
		m_swapChain->Present(0, 0);
	}

	m_currentFenceValue = m_fenceValues[m_backbufferIndex];

	m_hr = m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::MoveToNextFrame::m_commandQueue->Signal:", m_hr);
	}

	m_backbufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[m_backbufferIndex])
	{
		m_hr = m_fence->SetEventOnCompletion(m_fenceValues[m_backbufferIndex], m_fenceEvent);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::MoveToNextFrame::m_fence->SetEventOnCompletion:", m_hr);
		}

		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_fenceValues[m_backbufferIndex] = m_currentFenceValue + 1;
}

/*
*/
void CGraphicsAdapter::ResetAndClearTargets()
{
	for (int i = 0; i < CCommandListBundle::E_MAX_BUNDLES; i++)
	{
		m_commandListBundle[i]->Clear();
	}

	m_commandAllocators[m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_backbufferIndex].Get(), nullptr);

	m_swapChainRenderTargets[m_backbufferIndex]->SetPreDrawBarrier();
	m_swapChainDepthBuffers[m_backbufferIndex]->SetPreDrawBarrier();

	m_commandList->ResourceBarrier(1, &m_swapChainRenderTargets[m_backbufferIndex]->m_barrier);

	m_commandList->ClearRenderTargetView(m_swapChainRenderTargets[m_backbufferIndex]->m_handle, m_color->AshGray->GetFloatArray(), 0, nullptr);

	m_commandList->ClearDepthStencilView(m_swapChainDepthBuffers[m_backbufferIndex]->m_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandListBundle[0]->Add(m_commandList);
}

/*
*/
void CGraphicsAdapter::ToggleFullScreenWindow()
{
	if (m_windowed)
	{
		m_menu = GetMenu(m_hWnd);

		SetMenu(m_hWnd, NULL);

		GetWindowRect(m_hWnd, &m_windowRect);

		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);

		if (m_swapChain)
		{
			IDXGIOutput* pOutput;

			m_swapChain->GetContainingOutput(&pOutput);

			DXGI_OUTPUT_DESC Desc;

			pOutput->GetDesc(&Desc);

			m_fullScreenWindowRect = Desc.DesktopCoordinates;

			pOutput->Release();
			pOutput = 0;
		}

		SetWindowPos(m_hWnd, HWND_TOPMOST,
			m_fullScreenWindowRect.left, m_fullScreenWindowRect.top, m_fullScreenWindowRect.right, m_fullScreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
	else
	{
		SetMenu(m_hWnd, m_menu);

		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);

		SetWindowPos(m_hWnd, HWND_NOTOPMOST,
			m_windowRect.left, m_windowRect.top, m_windowRect.right - m_windowRect.left, m_windowRect.bottom - m_windowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(m_hWnd, SW_NORMAL);
		SetFocus(m_hWnd);
	}

	GetWindowRect(GetDesktopWindow(), &m_desktopRect);

	m_windowed = !m_windowed;
}

/*
*/
void CGraphicsAdapter::WaitForGPU()
{
	if (!m_fence)
	{
		return;
	}

	m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_backbufferIndex]);

	m_fence->SetEventOnCompletion(m_fenceValues[m_backbufferIndex], m_fenceEvent);

	WaitForSingleObject(m_fenceEvent, INFINITE);

	m_fenceValues[m_backbufferIndex]++;
}

/*
*/
char* CGraphicsAdapter::GetDXGIFormatName(DXGI_FORMAT f)
{
	struct DXGINames
	{
		char name[64];
		int number;
	};

	DXGINames* l_DXGINames;

	static DXGINames DXGINamesArray[] =
	{
		"DXGI_FORMAT_UNKNOWN", 0,
		"DXGI_FORMAT_R32G32B32A32_TYPELESS", 1,
		"DXGI_FORMAT_R32G32B32A32_FLOAT", 2,
		"DXGI_FORMAT_R32G32B32A32_UINT", 3,
		"DXGI_FORMAT_R32G32B32A32_SINT", 4,
		"DXGI_FORMAT_R32G32B32_TYPELESS", 5,
		"DXGI_FORMAT_R32G32B32_FLOAT", 6,
		"DXGI_FORMAT_R32G32B32_UINT", 7,
		"DXGI_FORMAT_R32G32B32_SINT", 8,
		"DXGI_FORMAT_R16G16B16A16_TYPELESS", 9,
		"DXGI_FORMAT_R16G16B16A16_FLOAT", 10,
		"DXGI_FORMAT_R16G16B16A16_UNORM", 11,
		"DXGI_FORMAT_R16G16B16A16_UINT", 12,
		"DXGI_FORMAT_R16G16B16A16_SNORM", 13,
		"DXGI_FORMAT_R16G16B16A16_SINT", 14,
		"DXGI_FORMAT_R32G32_TYPELESS", 15,
		"DXGI_FORMAT_R32G32_FLOAT", 16,
		"DXGI_FORMAT_R32G32_UINT", 17,
		"DXGI_FORMAT_R32G32_SINT", 18,
		"DXGI_FORMAT_R32G8X24_TYPELESS", 19,
		"DXGI_FORMAT_D32_FLOAT_S8X24_UINT", 20,
		"DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS", 21,
		"DXGI_FORMAT_X32_TYPELESS_G8X24_UINT", 22,
		"DXGI_FORMAT_R10G10B10A2_TYPELESS", 23,
		"DXGI_FORMAT_R10G10B10A2_UNORM", 24,
		"DXGI_FORMAT_R10G10B10A2_UINT", 25,
		"DXGI_FORMAT_R11G11B10_FLOAT", 26,
		"DXGI_FORMAT_R8G8B8A8_TYPELESS", 27,
		"DXGI_FORMAT_R8G8B8A8_UNORM", 28,
		"DXGI_FORMAT_R8G8B8A8_UNORM_SRGB", 29,
		"DXGI_FORMAT_R8G8B8A8_UINT", 30,
		"DXGI_FORMAT_R8G8B8A8_SNORM", 31,
		"DXGI_FORMAT_R8G8B8A8_SINT", 32,
		"DXGI_FORMAT_R16G16_TYPELESS", 33,
		"DXGI_FORMAT_R16G16_FLOAT", 34,
		"DXGI_FORMAT_R16G16_UNORM", 35,
		"DXGI_FORMAT_R16G16_UINT", 36,
		"DXGI_FORMAT_R16G16_SNORM", 37,
		"DXGI_FORMAT_R16G16_SINT", 38,
		"DXGI_FORMAT_R32_TYPELESS", 39,
		"DXGI_FORMAT_D32_FLOAT", 40,
		"DXGI_FORMAT_R32_FLOAT", 41,
		"DXGI_FORMAT_R32_UINT", 42,
		"DXGI_FORMAT_R32_SINT", 43,
		"DXGI_FORMAT_R24G8_TYPELESS", 44,
		"DXGI_FORMAT_D24_UNORM_S8_UINT", 45,
		"DXGI_FORMAT_R24_UNORM_X8_TYPELESS", 46,
		"DXGI_FORMAT_X24_TYPELESS_G8_UINT", 47,
		"DXGI_FORMAT_R8G8_TYPELESS", 48,
		"DXGI_FORMAT_R8G8_UNORM", 49,
		"DXGI_FORMAT_R8G8_UINT", 50,
		"DXGI_FORMAT_R8G8_SNORM", 51,
		"DXGI_FORMAT_R8G8_SINT", 52,
		"DXGI_FORMAT_R16_TYPELESS", 53,
		"DXGI_FORMAT_R16_FLOAT", 54,
		"DXGI_FORMAT_D16_UNORM", 55,
		"DXGI_FORMAT_R16_UNORM", 56,
		"DXGI_FORMAT_R16_UINT", 57,
		"DXGI_FORMAT_R16_SNORM", 58,
		"DXGI_FORMAT_R16_SINT", 59,
		"DXGI_FORMAT_R8_TYPELESS", 60,
		"DXGI_FORMAT_R8_UNORM", 61,
		"DXGI_FORMAT_R8_UINT", 62,
		"DXGI_FORMAT_R8_SNORM", 63,
		"DXGI_FORMAT_R8_SINT", 64,
		"DXGI_FORMAT_A8_UNORM", 65,
		"DXGI_FORMAT_R1_UNORM", 66,
		"DXGI_FORMAT_R9G9B9E5_SHAREDEXP", 67,
		"DXGI_FORMAT_R8G8_B8G8_UNORM", 68,
		"DXGI_FORMAT_G8R8_G8B8_UNORM", 69,
		"DXGI_FORMAT_BC1_TYPELESS", 70,
		"DXGI_FORMAT_BC1_UNORM", 71,
		"DXGI_FORMAT_BC1_UNORM_SRGB", 72,
		"DXGI_FORMAT_BC2_TYPELESS", 73,
		"DXGI_FORMAT_BC2_UNORM", 74,
		"DXGI_FORMAT_BC2_UNORM_SRGB", 75,
		"DXGI_FORMAT_BC3_TYPELESS", 76,
		"DXGI_FORMAT_BC3_UNORM", 77,
		"DXGI_FORMAT_BC3_UNORM_SRGB", 78,
		"DXGI_FORMAT_BC4_TYPELESS", 79,
		"DXGI_FORMAT_BC4_UNORM", 80,
		"DXGI_FORMAT_BC4_SNORM", 81,
		"DXGI_FORMAT_BC5_TYPELESS", 82,
		"DXGI_FORMAT_BC5_UNORM", 83,
		"DXGI_FORMAT_BC5_SNORM", 84,
		"DXGI_FORMAT_B5G6R5_UNORM", 85,
		"DXGI_FORMAT_B5G5R5A1_UNORM", 86,
		"DXGI_FORMAT_B8G8R8A8_UNORM", 87,
		"DXGI_FORMAT_B8G8R8X8_UNORM", 88,
		"DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM", 89,
		"DXGI_FORMAT_B8G8R8A8_TYPELESS", 90,
		"DXGI_FORMAT_B8G8R8A8_UNORM_SRGB", 91,
		"DXGI_FORMAT_B8G8R8X8_TYPELESS", 92,
		"DXGI_FORMAT_B8G8R8X8_UNORM_SRGB", 93,
		"DXGI_FORMAT_BC6H_TYPELESS", 94,
		"DXGI_FORMAT_BC6H_UF16", 95,
		"DXGI_FORMAT_BC6H_SF16", 96,
		"DXGI_FORMAT_BC7_TYPELESS", 97,
		"DXGI_FORMAT_BC7_UNORM", 98,
		"DXGI_FORMAT_BC7_UNORM_SRGB", 99,
		"DXGI_FORMAT_AYUV", 100,
		"DXGI_FORMAT_Y410", 101,
		"DXGI_FORMAT_Y416", 102,
		"DXGI_FORMAT_NV12", 103,
		"DXGI_FORMAT_P010", 104,
		"DXGI_FORMAT_P016", 105,
		"DXGI_FORMAT_420_OPAQUE", 106,
		"DXGI_FORMAT_YUY2", 107,
		"DXGI_FORMAT_Y210", 108,
		"DXGI_FORMAT_Y216", 109,
		"DXGI_FORMAT_NV11", 110,
		"DXGI_FORMAT_AI44", 111,
		"DXGI_FORMAT_IA44", 112,
		"DXGI_FORMAT_P8", 113,
		"DXGI_FORMAT_A8P8", 114,
		"DXGI_FORMAT_B4G4R4A4_UNORM", 115,
		"DXGI_FORMAT_P208", 130,
		"DXGI_FORMAT_V208", 131,
		"DXGI_FORMAT_V408", 132,
		"DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE", 189,
		"DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE", 190
	};

	l_DXGINames = DXGINamesArray;

	int d = 0;

	for (d = 0; d < countArray(DXGINamesArray, DXGINames); d++)
	{
		if (l_DXGINames[d].number == (int)f)
		{
			return l_DXGINames[d].name;

			break;
		}
	}

	if (d == countArray(DXGINamesArray, DXGINames))
	{
		m_errorLog->WriteError(true, "Format:%i Unkown\n", f);
	}

	return nullptr;
}