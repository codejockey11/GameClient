#include "CGraphicsAdapter.h"

/*
*/
CGraphicsAdapter::CGraphicsAdapter()
{
	memset(this, 0x00, sizeof(CGraphicsAdapter));
}

/*
*/
CGraphicsAdapter::CGraphicsAdapter(CErrorLog* errorLog, HWND hWnd, int32_t width, int32_t height, bool windowed, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
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

	m_DXGINames = new CDXGINames();

	m_swapChainViewport = new CViewport(0.0f, 0.0f, (float)m_width, (float)m_height);

#ifdef _DEBUG
	m_hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&m_debugController);

	//m_debugController->EnableDebugLayer();
#endif	

	CGraphicsAdapter::DXGIFactory();
	CGraphicsAdapter::CreateDevice();

#ifdef _DEBUG
	m_device->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&m_infoQueue);

	m_severities = new D3D12_MESSAGE_SEVERITY[1]();

	m_severities[0] = D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_INFO;

	m_denyIds = new D3D12_MESSAGE_ID[1]();

	m_denyIds[0] = D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE;

	m_newFilter.DenyList.NumSeverities = 1;
	m_newFilter.DenyList.pSeverityList = m_severities;

	m_newFilter.DenyList.NumIDs = 1;
	m_newFilter.DenyList.pIDList = m_denyIds;

	if (m_infoQueue != nullptr)
	{
		m_infoQueue->PushStorageFilter(&m_newFilter);
	}
#endif

	CGraphicsAdapter::CreateCommandQueue();
	CGraphicsAdapter::CreateSwapChain();

#ifdef D3D11on12
	CGraphicsAdapter::Create11on12Device();
#endif

	CGraphicsAdapter::CreateRenderTargets();

	for (int32_t i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocators[i]);

		m_commandAllocators[i]->SetName(L"CGraphicsAdapter::CommandAllocator");
	}

	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr,
		__uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);

	m_commandList->SetName(L"CGraphicsAdapter::CommandList");

	m_commandList->Close();

#ifndef D3D11on12
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr,
		__uuidof(ID3D12GraphicsCommandList), (void**)&m_eofCommandList);

	m_eofCommandList->SetName(L"CGraphicsAdapter::EOFCommandList");

	m_eofCommandList->Close();
#endif

	CGraphicsAdapter::CreateFence();

	m_descriptorHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_rgbColor = new CRGBColor();

	float xScale = (m_swapChainViewport->m_viewport.Width > 0) ? 2.0f / m_swapChainViewport->m_viewport.Width : 0.0f;

	float yScale = (m_swapChainViewport->m_viewport.Height > 0) ? 2.0f / m_swapChainViewport->m_viewport.Height : 0.0f;

	m_wvpMat = XMFLOAT4X4
	(
		xScale, 0, 0, 0,
		0, -yScale, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);
}

/*
*/
CGraphicsAdapter::~CGraphicsAdapter()
{
	CGraphicsAdapter::WaitForGPU();

	SAFE_DELETE(m_rgbColor);

	CloseHandle(m_fenceEvent);

	m_fenceEvent = 0;

	m_fence.Reset();

	m_commandList.Reset();

	for (int32_t i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();

		SAFE_DELETE(m_swapChainDepthBuffers[i]);
		SAFE_DELETE(m_swapChainRenderTargets[i]);
	}

#ifdef D3D11on12
	SAFE_DELETE(m_d3d11On12);
#else
	m_eofCommandList.Reset();
#endif

	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, nullptr);
	}

	m_swapChain.Reset();

	SAFE_DELETE(m_swapChainViewport);

	m_commandQueue.Reset();

	SAFE_DELETE_ARRAY(m_denyIds);
	SAFE_DELETE_ARRAY(m_severities);

	m_infoQueue.Reset();

	m_device.Reset();

	m_adapterOutput.Reset();

	m_adapter.Reset();

	m_factory.Reset();

#ifdef _DEBUG
	m_debugController.Reset();

	if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug), (void**)&m_debug)))
	{
		m_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS::DXGI_DEBUG_RLO_DETAIL);

		m_debug->Release();
	}
#endif

	SAFE_DELETE(m_DXGINames);
}

/*
*/
void CGraphicsAdapter::Create11on12Device()
{
#ifdef D3D11on12
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

	m_commandQueue->SetName(L"CGraphicsAdapter::CreateCommandQueue");
}

/*
*/
void CGraphicsAdapter::CreateDevice()
{
	/*
	D3D_FEATURE_LEVEL_9_1 = 0x9100,
	D3D_FEATURE_LEVEL_9_2 = 0x9200,
	D3D_FEATURE_LEVEL_9_3 = 0x9300,
	D3D_FEATURE_LEVEL_10_0 = 0xa000,
	D3D_FEATURE_LEVEL_10_1 = 0xa100,
	D3D_FEATURE_LEVEL_11_0 = 0xb000,
	D3D_FEATURE_LEVEL_11_1 = 0xb100,
	D3D_FEATURE_LEVEL_12_0 = 0xc000,
	D3D_FEATURE_LEVEL_12_1 = 0xc100,
	D3D_FEATURE_LEVEL_12_2 = 0xc200
	*/

	m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter4), (void**)&m_adapter);

	for (int32_t i = 0; i < 10; i++)
	{
		switch (i)
		{
		case 0:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1;

			break;
		}
		case 1:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_2;

			break;
		}
		case 2:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3;

			break;
		}
		case 3:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0;

			break;
		}
		case 4:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1;

			break;
		}
		case 5:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

			break;
		}
		case 6:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

			break;
		}
		case 7:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0;

			break;
		}
		case 8:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1;

			break;
		}
		case 9:
		{
			m_featureCheck = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_2;

			break;
		}
		}

		m_hr = D3D12CreateDevice(m_adapter.Get(), m_featureCheck, __uuidof(ID3D12Device), nullptr);

		if (SUCCEEDED(m_hr))
		{
			m_featureLevel = m_featureCheck;

			m_errorLog->WriteError(true, "CGraphicsAdapter::GetHardwareAdapter:Feature supported:d3dLevel:0x%0x\n", m_featureCheck);
		}
		else
		{
			m_errorLog->WriteError(true, "CGraphicsAdapter::GetHardwareAdapter:Feature not supported:d3dLevel:0x%0x:", m_featureCheck);
			
			m_errorLog->WriteDXGIErrorMessage(false, "", m_hr);
		}
	}

	m_hr = D3D12CreateDevice(m_adapter.Get(), m_featureLevel, __uuidof(ID3D12Device), (void**)&m_device);

	CGraphicsAdapter::DeviceInformation();
}

/*
*/
void CGraphicsAdapter::CreateFence()
{
	m_hr = m_device->CreateFence(m_fenceValues[m_backbufferIndex], D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CreateFence:", m_hr);

		return;
	}

	m_fence->SetName(L"CGraphicsAdapter::CreateFence");

	m_fenceValues[m_backbufferIndex]++;

	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
}

/*
*/
void CGraphicsAdapter::CreateRenderTargets()
{
	for (int32_t i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_swapChainRenderTargets[i] = new CRenderTarget(m_errorLog, m_device);

		m_swapChainRenderTargets[i]->SwapChainTarget(m_swapChain, i);

#ifdef D3D11on12
		m_swapChainRenderTargets[i]->WrapResource(m_dpi, m_d3d11On12->m_d3d11On12Device, m_d3d11On12->m_d2dDeviceContext);
#endif

		m_swapChainDepthBuffers[i] = new CDepthBuffer(m_errorLog, m_device, 1, m_width, m_height);

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
	m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
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
	m_swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_hr = m_factory->CreateSwapChain(m_commandQueue.Get(), &m_swapChainDesc, (IDXGISwapChain**)m_swapChain.GetAddressOf());

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::CreateSwapChain::CreateSwapChain:", m_hr);
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

	SAFE_DELETE(m_adapterDescription);

	m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_ARCHITECTURE1, &m_featureDataArchitecture, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE1));

	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::Architecture UMA:%s\n", m_featureDataArchitecture.UMA ? "T" : "F");

	// 1M = 1,048,576 = 1024 * 1024
	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::DedicatedVideoMemory:%dM\n", (int32_t)(m_adapterDesc.DedicatedVideoMemory / 1048576));
	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::DedicatedSystemMemory:%dM\n", (int32_t)(m_adapterDesc.DedicatedSystemMemory / 1048576));
	m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::SharedSystemMemory:%dM\n", (int32_t)(m_adapterDesc.SharedSystemMemory / 1048576));

	m_hr = m_adapter->EnumOutputs(0, &m_adapterOutput);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::EnumOutputs:", m_hr);
	}

	if (m_adapterOutput != nullptr)
	{
		m_hr = m_adapterOutput->GetDisplayModeList(m_backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &m_numModes, nullptr);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DeviceInformation::GetDisplayModeList for numModes:", m_hr);
		}
		else
		{
			if (m_numModes > 0)
			{
				m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::DXGI_FORMAT:%s\n", m_DXGINames->GetDXGIFormatName(m_backBufferFormat));

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
					uint32_t prevWidth = 0;
					uint32_t prevHeight = 0;

					for (uint32_t i = 0; i < m_numModes; i++)
					{
						// Warning C6385
						if ((m_displayModeList[i].Width != prevWidth) && (m_displayModeList[i].Height != prevHeight))
						{
							if ((m_displayModeList[i].RefreshRate.Numerator / m_displayModeList[i].RefreshRate.Denominator) == 60)
							{
								m_errorLog->WriteError(true, "CGraphicsAdapter::DeviceInformation::m:%03i w:%04i h:%04i n:%09i d:%07i hz:%i\n",
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

					SAFE_DELETE_ARRAY(m_displayModeList);
				}
			}
		}
	}
}

/*
*/
void CGraphicsAdapter::DXGIFactory()
{
#ifdef _DEBUG
	//m_factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif	

	m_hr = CreateDXGIFactory2(m_factoryFlags, IID_IDXGIFactory2, (void**)&m_factory);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CGraphicsAdapter::DXGIFactory:", m_hr);
	}
}

/*
*/
void CGraphicsAdapter::End2DDraw()
{
#ifdef D3D11on12
	m_d3d11On12->m_d2dDeviceContext->EndDraw();

	m_d3d11On12->m_d3d11On12Device->ReleaseWrappedResources(m_swapChainRenderTargets[m_backbufferIndex]->m_wrappedBackBuffer.GetAddressOf(), 1);

	m_d3d11On12->m_d3d11DeviceContext->Flush();
#endif
}

/*
*/
D3D12_CPU_DESCRIPTOR_HANDLE* CGraphicsAdapter::GetDepthBuffer()
{
	return &m_swapChainDepthBuffers[m_backbufferIndex]->m_handle;
}

/*
*/
D3D12_CPU_DESCRIPTOR_HANDLE* CGraphicsAdapter::GetRenderTarget()
{
	return &m_swapChainRenderTargets[m_backbufferIndex]->m_handle;
}

/*
*/
D3D12_RECT* CGraphicsAdapter::GetScissorRect()
{
	return &m_swapChainViewport->m_scissorRect;
}

/*
*/
D3D12_VIEWPORT* CGraphicsAdapter::GetViewport()
{
	return &m_swapChainViewport->m_viewport;
}

/*
*/
void CGraphicsAdapter::Init2DDraw()
{
#ifdef D3D11on12
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
#ifndef D3D11on12
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
		m_swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
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
void CGraphicsAdapter::Record()
{
	m_commandAllocators[m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_backbufferIndex].Get(), nullptr);

	m_swapChainRenderTargets[m_backbufferIndex]->SetPreDrawBarrier();
	m_swapChainDepthBuffers[m_backbufferIndex]->SetPreDrawBarrier();

	m_commandList->ResourceBarrier(1, &m_swapChainRenderTargets[m_backbufferIndex]->m_barrier);

	m_commandList->ClearRenderTargetView(m_swapChainRenderTargets[m_backbufferIndex]->m_handle, m_rgbColor->CornflowerBlue->GetFloatArray(), 0, nullptr);

	m_commandList->ClearDepthStencilView(m_swapChainDepthBuffers[m_backbufferIndex]->m_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
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

		SetWindowLong(m_hWnd, GWL_STYLE, WS_TILEDWINDOW);

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
	m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_backbufferIndex]);

	m_fence->SetEventOnCompletion(m_fenceValues[m_backbufferIndex], m_fenceEvent);

	WaitForSingleObject(m_fenceEvent, INFINITE);

	m_fenceValues[m_backbufferIndex]++;
}