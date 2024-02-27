// GameClient.cpp : Defines the entry point for the application.
//

#include "framework.h"

#include "CButtonManager.h"
#include "CCamera.h"
#include "CChatBox.h"
#include "CClient.h"
#include "CColor.h"
#include "CCommandListBundle.h"
#include "CErrorLog.h"
#include "CFont.h"
#include "CFontManager.h"
#include "CFrametime.h"
#include "CKeyboardDevice.h"
#include "CModelManager.h"
#include "CMouseDevice.h"
#include "CShaderManager.h"
#include "CSoundDevice.h"
#include "CSoundManager.h"
#include "CString.h"
#include "CTextureManager.h"
#include "CVertex.h"
#include "CVideoDevice.h"
#include "CWavefrontManager.h"


constexpr auto MAX_LOADSTRING = 100;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

CButtonManager* m_buttonManager;
CCamera* m_currentCamera;
CChatBox* m_chatBox;
CClient* m_client;
CColor* m_color;
CCommandListBundle* m_commandListBundle[CCommandListBundle::E_MAX_BUNDLES];
CFont* m_consolas12;
CFontManager* m_fontManager;
CErrorLog* m_errorLog;
CFrametime* m_frametime;
CKeyboardDevice* m_keyboardDevice;
CModelManager* m_modelManager;
CMouseDevice* m_mouseDevice;
CShaderManager* m_shaderManager;
CSoundDevice* m_soundDevice;
CSoundManager* m_soundManager;
CTextureManager* m_textureManager;
CVideoDevice* m_videoDevice;
CWavefrontManager* m_wavefrontManager;

class CCommandLine
{
public:

	WCHAR name[32];
	WCHAR ip[17];
	WCHAR port[6];

	CCommandLine()
	{
		wcscpy_s(name, 32, L"Steve");
		wcscpy_s(ip, 17, L"127.0.0.1");
		wcscpy_s(port, 6, L"26105");
	}
};

CCommandLine commandLine;


// NuGet project items
#include <stdlib.h>
#include <string>

// Pointer to WebViewController
static wil::com_ptr<ICoreWebView2Controller> webviewController;

// Pointer to WebView window
static wil::com_ptr<ICoreWebView2> webview;

bool m_browserOpen;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


void Draw3D()
{
	if ((m_client->m_terrain != nullptr) && (m_client->m_camera[m_client->m_localClient->m_clientNumber] != nullptr))
	{
		m_client->m_overhead->ClearTargets();

		
		m_currentCamera = m_client->m_camera[m_client->m_localClient->m_clientNumber];


		m_client->m_terrain->SetCurrentCamera(m_currentCamera);
		m_client->m_terrain->Record();

		m_client->m_terrain->SetOverheadCamera(m_client->m_overhead->m_camera);
		m_client->m_terrain->Record(m_client->m_overhead);

		
		m_commandListBundle[1]->Add(m_client->m_terrain->m_commandList);


		for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
		{
			CServerInfo* serverInfo = (CServerInfo*)m_client->m_serverInfo->GetElement(1, i);

			if (serverInfo->m_isRunning)
			{
				if ((m_client->m_model[serverInfo->m_clientNumber] != nullptr) && (m_client->m_model[serverInfo->m_clientNumber]->m_isRecordable))
				{
					m_client->m_model[serverInfo->m_clientNumber]->SetCurrentCamera(m_currentCamera);
					m_client->m_model[serverInfo->m_clientNumber]->Record();

					m_client->m_model[serverInfo->m_clientNumber]->SetOverheadCamera(m_client->m_overhead->m_camera);
					m_client->m_model[serverInfo->m_clientNumber]->Record(m_client->m_overhead);

					m_commandListBundle[2]->Add(m_client->m_model[serverInfo->m_clientNumber]->m_commandList);
				}
			}
		}


		if (m_client->m_visibility != nullptr)
		{
			for (UINT pz = 0; pz < m_client->m_visibility->m_gridHeight; pz++)
			{
				for (UINT px = 0; px < m_client->m_visibility->m_gridWidth; px++)
				{
					CLinkList<CObject>* cube = (CLinkList<CObject>*)m_client->m_visibility->m_collectables->GetElement(2, px, pz);

					if (cube != nullptr)
					{
						if (cube->m_count > 0)
						{
							CLinkListNode<CObject>* collectables = cube->m_list;

							while (collectables->m_object)
							{
								if (!collectables->m_object->m_limboTimer->m_isReloading)
								{
									if (collectables->m_object->m_isRecordable)
									{
										collectables->m_object->SetCurrentCamera(m_currentCamera);
										collectables->m_object->Animation(m_frametime);
										collectables->m_object->Update();
										collectables->m_object->Record();

										collectables->m_object->SetOverheadCamera(m_client->m_overhead->m_camera);
										collectables->m_object->Record(m_client->m_overhead);

										m_commandListBundle[2]->Add(collectables->m_object->m_commandList);
									}
								}
								else
								{
									collectables->m_object->m_limboTimer->Frame();
								}

								collectables = collectables->m_next;
							}
						}
					}
				}
			}
		}

		m_client->m_overhead->Record();

		m_commandListBundle[2]->Add(m_client->m_overhead->m_commandList);

	}

}

void DrawHud()
{
	CLinkListNode<CButton>* buttons = m_buttonManager->m_buttons->m_list;

	while (buttons->m_object)
	{
		buttons->m_object->Record();

		m_commandListBundle[3]->Add(buttons->m_object->m_image->m_commandList);

		buttons = buttons->m_next;
	}


	if (m_chatBox->m_isVisible)
	{
		m_chatBox->DisplayConsole();
		m_commandListBundle[3]->Add(m_chatBox->m_background->m_commandList);
	}
}

void ExecuteCommandLists()
{
	// Perform the command lists
	// Video Device is always first
	m_commandListBundle[0]->Close();
	m_commandListBundle[0]->Perform();

	m_commandListBundle[1]->Close();
	m_commandListBundle[1]->Perform();

	m_commandListBundle[2]->Close();
	m_commandListBundle[2]->Perform();

	m_commandListBundle[3]->Close();
	m_commandListBundle[3]->Perform();
}

void DrawText11On12()
{
	// get ready to render text
	m_videoDevice->Init2DDraw();

	wchar_t text[50] = {};

	CVertex2 position(10.0f, 10.0f);
	CVertex2 size(256.0f, 32.0f);

	swprintf_s(text, 50, L"FPS: %i", m_frametime->m_rframecount);

	m_consolas12->Draw(text, position, size, m_color->SunYellow);

	position.p.y += m_consolas12->m_height;


	if (m_client->m_connectionState == CClient::E_CONNECTED)
	{
		swprintf_s(text, 50, L"Connected %5.2f %5.2f %i", m_client->m_localClient->m_idleTime, m_client->m_localClient->m_velocity, m_client->m_localClient->m_countdown);

		m_consolas12->Draw(text, position, size, m_color->SunYellow);

		position.p.y += m_consolas12->m_height;
	}
	else
	{
		swprintf_s(text, 50, L"Disconnected");

		m_consolas12->Draw(text, position, size, m_color->SunYellow);

		position.p.y += m_consolas12->m_height;
	}



	CLinkListNode<CButton>* buttons = m_buttonManager->m_buttons->m_list;

	while (buttons->m_object)
	{
		buttons->m_object->DisplayText();

		buttons = buttons->m_next;
	}



	if (m_chatBox->m_isVisible)
	{
		m_chatBox->DisplayText();
	}

	// end drawing text
	m_videoDevice->End2DDraw();
}

void DrawScene()
{
	// Ready command list bundles
	m_commandListBundle[0]->Clear();
	m_commandListBundle[1]->Clear();
	m_commandListBundle[2]->Clear();
	m_commandListBundle[3]->Clear();


	// Recording video devices command list
	m_videoDevice->ResetAndClearTargets();

	m_commandListBundle[0]->Add(m_videoDevice->m_commandList);

	Draw3D();
	
	DrawHud();

	ExecuteCommandLists();

	DrawText11On12();

	m_videoDevice->MoveToNextFrame();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	HRESULT hr = OleInitialize(0);

	UNREFERENCED_PARAMETER(hPrevInstance);

	// Parse the command line sent from the Game Manager to obtain the clients account information
	swscanf_s(lpCmdLine, L"%s %s %s", commandLine.name, 32, commandLine.ip, 32, commandLine.port, 6);

	// TODO: Place code here.
	m_errorLog = new CErrorLog("clientLog.txt");

	CString* cmdLine = new CString(lpCmdLine);
	cmdLine->Concat("\n");


	m_errorLog->WriteError(true, cmdLine->GetText());


	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GAMECLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMECLIENT));

	MSG msg = {};

	// Main message loop:
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			DispatchMessage(&msg);
		}
		else
		{
			m_frametime->Frame();


			m_mouseDevice->GetState();

			for (int i = 0; i < CMouseDevice::E_BUTTON_COUNT; i++)
			{
				if ((m_mouseDevice->m_keyMap[i].m_network != nullptr) &&
					// Comment this line out for fps movement
					(m_mouseDevice->m_keyMap[i].m_count > 0) &&
					(m_mouseDevice->m_keyMap[i].m_network->m_type == CNetwork::ClientEvent::E_CE_CAMERA_MOVE))
				{
					m_client->Send(m_mouseDevice->m_keyMap[i].m_network);
				}
			}

			m_keyboardDevice->GetState();


			while (m_client->m_isActiveUpdate)
			{
			}

			m_client->m_isActiveRender = true;

			DrawScene();

			m_client->m_isActiveRender = false;
		}
	}

	m_videoDevice->WaitForGPU();

	delete m_chatBox;
	delete m_client;
	delete m_buttonManager;
	delete m_soundManager;
	delete m_soundDevice;
	delete m_mouseDevice;
	delete m_keyboardDevice;
	delete m_fontManager;
	delete m_shaderManager;
	delete m_textureManager;
	delete m_wavefrontManager;
	delete m_modelManager;


	for (int i = 0; i < CCommandListBundle::E_MAX_BUNDLES; i++)
	{
		delete m_commandListBundle[i];
	}

	delete m_videoDevice;
	delete m_errorLog;
	delete m_color;
	delete m_frametime;


	OleUninitialize();

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMECLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAMECLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	int x = (GetSystemMetrics(SM_CXSCREEN) - 1240) / 2;
	int y = ((GetSystemMetrics(SM_CYSCREEN) - 768) / 2);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
		x, y,
		1240, 768,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	m_frametime = new CFrametime();

	m_color = new CColor();


	m_videoDevice = new CVideoDevice(m_errorLog, hWnd, 1240, 768, true, m_color);

	m_commandListBundle[0] = new CCommandListBundle(m_errorLog, m_videoDevice);
	m_commandListBundle[1] = new CCommandListBundle(m_errorLog, m_videoDevice);
	m_commandListBundle[2] = new CCommandListBundle(m_errorLog, m_videoDevice);
	m_commandListBundle[3] = new CCommandListBundle(m_errorLog, m_videoDevice);


	m_modelManager = new CModelManager(m_errorLog);

	m_wavefrontManager = new CWavefrontManager(m_errorLog);

	m_textureManager = new CTextureManager(m_videoDevice, m_errorLog);

	m_shaderManager = new CShaderManager(m_videoDevice, m_errorLog);

	m_fontManager = new CFontManager(m_videoDevice, m_errorLog, m_color);

	m_consolas12 = m_fontManager->Create("Consolas 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);


	m_keyboardDevice = new CKeyboardDevice();

	m_mouseDevice = new CMouseDevice(m_videoDevice, m_errorLog, 0.0125f);

	m_soundDevice = new CSoundDevice(m_errorLog);

	m_soundManager = new CSoundManager(m_soundDevice, m_errorLog);

	m_buttonManager = new CButtonManager(m_videoDevice, m_mouseDevice, m_errorLog, m_fontManager, m_shaderManager, m_soundManager, m_color);


	CTexture* button = m_textureManager->Create("image\\button.tga");

	m_buttonManager->Create("Fullscreen", button, "Calibri 12",
		CVertex2(64.0f, 32.0f), CVertex2(10.0f, 60.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_WINDOW_MODE);


	m_soundManager->Create("audio\\rifle.wav", false);

	m_client = new CClient(m_videoDevice, m_mouseDevice, m_keyboardDevice, m_errorLog, m_textureManager, m_shaderManager, m_modelManager, m_wavefrontManager, m_soundManager);

	m_chatBox = new CChatBox(m_videoDevice, m_errorLog, button, m_consolas12, m_color, m_shaderManager, 1024, 400);


	m_browserOpen = false;


	m_videoDevice->WaitForGPU();

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		if (wParam & MK_CONTROL)
		{
			return 0;
		}

		if (wParam & MK_SHIFT)
		{
			return 0;
		}

		return 0;

		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		m_mouseDevice->SaveLastPosition();

		m_mouseDevice->FirstPersonReset();

		m_mouseDevice->MouseClick(CMouseDevice::E_RMB, 1);

		return 0;

		break;
	}
	case WM_RBUTTONUP:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		m_mouseDevice->SetLastPosition();

		m_mouseDevice->MouseClick(CMouseDevice::E_RMB, 0);

		return 0;

		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		CLinkListNode<CButton>* buttons = m_buttonManager->m_buttons->m_list;

		while (buttons->m_object)
		{
			if (buttons->m_object->m_isMouseOver)
			{
				m_client->Send(buttons->m_object->m_network);

				return 0;
			}

			buttons = buttons->m_next;
		}

		m_mouseDevice->MouseClick(CMouseDevice::E_LMB, 1);

		return 0;

		break;
	}
	case WM_LBUTTONUP:
	{
		m_mouseDevice->MouseClick(CMouseDevice::E_LMB, 0);

		return 0;

		break;
	}
	case WM_MBUTTONDOWN:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		m_mouseDevice->MouseClick(CMouseDevice::E_MMB, 1);

		return 0;

		break;
	}
	case WM_MBUTTONUP:
	{
		m_mouseDevice->MouseClick(CMouseDevice::E_MMB, 0);

		return 0;

		break;
	}
	case WM_MOUSEWHEEL:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		if (m_currentCamera == nullptr)
		{
			return 0;
		}

		long wheelDelta = (long)GET_WHEEL_DELTA_WPARAM(wParam);

		m_mouseDevice->MouseWheel(wheelDelta);

		m_currentCamera->UpdateRotation((m_mouseDevice->m_lY * m_mouseDevice->m_sensitivity),
			(m_mouseDevice->m_lX * m_mouseDevice->m_sensitivity),
			(m_mouseDevice->m_lZ * m_mouseDevice->m_sensitivity));

		return 0;

		break;
	}
	case WM_CHAR:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		if (m_chatBox->m_isVisible)
		{
			if (wParam == VK_RETURN)
			{
				// m_message was defined with a fixed length so need to use StrLen() instead of GetLength()
				if (m_chatBox->m_message->StrLen() > 0)
				{
					CString* message = new CString(m_client->m_localClient->m_name);

					message->Concat(":");
					message->Concat(m_chatBox->m_message->GetText());

					if ((m_client->m_localClient != nullptr))// && (m_client->m_localClient->m_state == CServerInfo::E_LOBBY))
					{
						CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_CHAT,
							(void*)message->GetText(), message->GetLength(),
							(void*)m_client->m_localClient);

						m_client->Send(n);

						delete n;
					}
					else
					{
						while (m_client->m_isActiveActivity)
						{

						}

						m_client->m_isActiveChat = true;

						if (m_client->m_localClient != nullptr)
						{
							m_client->m_localClient->SetChat(message->GetText());
						}

						m_client->m_isActiveChat = false;
					}

					delete message;
				}

				m_chatBox->Reset();

				return 0;
			}

			if (wParam == VK_BACK)
			{
				m_chatBox->Backspace();

				return 0;
			}

			// ASCII values
			if ((wParam >= 32) && (wParam <= 126))
			{
				m_chatBox->AddChar((char)wParam);

				return 0;
			}
		}

		return 0;

		break;
	}
	case WM_KEYDOWN:
	{
		if (m_browserOpen)
		{
			return 0;
		}

		if (wParam == VK_SHIFT)
		{
			return 0;
		}

		if (wParam == VK_CONTROL)
		{
			return 0;
		}

		CNetwork* n = m_client->m_keyboardDevice->m_keyMap[(int)wParam].m_network;

		if ((n != nullptr) && (n->m_audience == CNetwork::E_CE_TO_LOCAL))
		{
			if (m_client->m_localClient != nullptr)
			{
				n->SetServerInfo(m_client->m_localClient);
			}

			m_client->Send(n);

			return 0;
		}

		if (m_chatBox->m_isVisible)
		{
			if (wParam == VK_PRIOR)
			{
				m_chatBox->Paging(CChatBox::E_PAGE_BACKWARD);
			}

			if (wParam == VK_NEXT)
			{
				m_chatBox->Paging(CChatBox::E_PAGE_FORWARD);
			}

			return 0;
		}

		m_keyboardDevice->KeyDown((int)wParam);

		return 0;

		break;
	}
	case WM_KEYUP:
	{
		m_keyboardDevice->KeyUp((int)wParam);

		return 0;

		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			return 0;

			break;
		}
		case IDM_BROWSER:
		{
			m_browserOpen = true;

			std::wstring m_userDataFolder = L"C:\\Users\\junk_\\source\\repos\\udf";

			CreateCoreWebView2EnvironmentWithOptions(nullptr, m_userDataFolder.c_str(), nullptr,
				Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
					[hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

						// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
						env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
								if (controller != nullptr) {
									webviewController = controller;
									webviewController->get_CoreWebView2(&webview);
								}

			// Add a few settings for the webview
			// The demo step is redundant since the values are the default settings
			wil::com_ptr<ICoreWebView2Settings> settings;
			webview->get_Settings(&settings);
			settings->put_IsScriptEnabled(TRUE);
			settings->put_AreDefaultScriptDialogsEnabled(TRUE);
			settings->put_IsWebMessageEnabled(TRUE);

			// Resize WebView to fit the bounds of the parent window
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			bounds.left += 100;
			bounds.top += 100;
			bounds.right -= 50;
			bounds.bottom -= 50;

			webviewController->put_Bounds(bounds);

			// Schedule an async task to navigate to Bing
			webview->Navigate(L"https://www.google.com/");
			//webview->Navigate(L"https://www.youtube.com/?themeRefresh=1");
			//webview->Navigate(L"file://C:/Users/junk_/Videos/Captures/_video.html");

			// <NavigationEvents>
			// Step 4 - Navigation events
			// register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
			EventRegistrationToken token;
			webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
				[](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
					wil::unique_cotaskmem_string uri;
			args->get_Uri(&uri);
			std::wstring source(uri.get());
			if (source.substr(0, 5) != L"https") {
				//args->put_Cancel(true);
			}
			return S_OK;
				}).Get(), &token);
			// </NavigationEvents>

			// <Scripting>
			// Step 5 - Scripting
			// Schedule an async task to add initialization script that freezes the Object object
			webview->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
			// Schedule an async task to get the document URL
			webview->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
					LPCWSTR URL = resultObjectAsJson;
			//doSomethingWithURL(URL);
			return S_OK;
				}).Get());
			// </Scripting>

			// <CommunicationHostWeb>
			// Step 6 - Communication between host and web content
			// Set an event handler for the host to return received message back to the web content
			webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
					wil::unique_cotaskmem_string message;
			args->TryGetWebMessageAsString(&message);
			// processMessage(&message);
			webview->PostWebMessageAsString(message.get());
			return S_OK;
				}).Get(), &token);

			// <WindowCloseRequested>
			// Register a handler for the WindowCloseRequested event.
			// This handler will close the app window if it is not the main window.
			webview->add_WindowCloseRequested(
				Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
					[hWnd](ICoreWebView2* sender, IUnknown* args)
					{
						m_browserOpen = false;

						webviewController->Close();

			return S_OK;
					})
				.Get(),
						nullptr);
			// <WindowCloseRequested>

			// Schedule an async task to add initialization script that
			// 1) Add an listener to print message from the host
			// 2) Post document URL to the host
			/*
			webview->AddScriptToExecuteOnDocumentCreated(
				L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
				L"window.chrome.webview.postMessage(window.document.URL);",
				nullptr);
				*/
				// </CommunicationHostWeb>

			return S_OK;
							}).Get());
			return S_OK;
					}).Get());


			return 0;
		}
		case IDM_BROWSEREXIT:
		{
			m_browserOpen = false;

			webviewController->Close();

			return 0;
		}
		case IDM_ACCOUNT:
		{
			CString* name = new CString(commandLine.name);

			m_client->m_localClient->SetName(name->GetText());

			CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_ACCOUNT_INFO,
				name->GetText(), name->GetLength(),
				(void*)m_client->m_localClient);

			m_client->Send(n);

			delete n;

			delete name;

			return 0;

			break;
		}
		case IDM_CAMERA:
		{
			m_mouseDevice->MouseMove();

			if ((m_client->m_localClient != nullptr) && (m_client->m_camera[m_client->m_localClient->m_clientNumber] != nullptr))
			{
				m_client->m_camera[m_client->m_localClient->m_clientNumber]->UpdateRotation((m_mouseDevice->m_lY * 0.1250f),
					(m_mouseDevice->m_lX * 0.1250f),
					(m_mouseDevice->m_lZ * 0.1250f));

				m_mouseDevice->FirstPersonReset();
			}

			return 0;

			break;
		}
		case IDM_CHATBOX:
		{
			m_chatBox->ToggleVisibility();

			return 0;

			break;
		}
		case IDM_CHATMESSAGE:
		{
			if (lParam == 0)
			{
				return 0;
			}

			m_chatBox->AddEventMessage((char*)lParam);

			return 0;

			break;
		}
		case IDM_CONNECT:
		{
			CString* name = new CString(commandLine.name);

			m_client->SetLogin(name->GetText());

			CString* ip = new CString(commandLine.ip);
			CString* port = new CString(commandLine.port);

			ip->Concat(" ");
			ip->Concat(port->GetText());

			m_client->Connect(ip->GetText());

			delete port;
			delete ip;
			delete name;

			return 0;

			break;
		}
		case IDM_DISCONNECT:
		{
			while (m_client->m_isActiveRender)
			{

			}

			m_client->m_isActiveUpdate = true;

			m_client->Disconnect();

			m_client->DestroyEnvironment();

			m_client->m_isActiveUpdate = false;

			return 0;

			break;
		}
		case IDM_EXIT:
		{
			DestroyWindow(hWnd);

			return 0;

			break;
		}
		case IDM_FULLSCREEN:
		{
			m_videoDevice->ToggleFullScreenWindow();

			return 0;

			break;
		}
		case IDM_JOIN:
		{
			CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_JOIN,
				nullptr, 0,
				(void*)m_client->m_localClient);

			m_client->Send(n);

			delete n;

			return 0;

			break;
		}
		case IDM_LOAD:
		{
			CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_LOAD_ENVIRONMENT,
				(void*)"terrain\\vertices.txt", 20,
				(void*)m_client->m_localClient);

			m_client->Send(n);

			delete n;

			return 0;

			break;
		}
		}

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);

		return 0;

		break;
	}
	case WM_SIZE:
	{
		if (webviewController != nullptr)
		{
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			bounds.left += 250;
			bounds.top += 250;
			bounds.right -= 50;
			bounds.bottom -= 50;

			webviewController->put_Bounds(bounds);
		}

		return 0;

		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);

		return 0;

		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}