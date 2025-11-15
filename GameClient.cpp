#include "framework.h"

#include "../GameCommon/CCommandLine.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CWebBrowser.h"

#include "CClient.h"
#include "CGraphicsAdapter.h"
#include "CKeyboardDevice.h"
#include "CMouseDevice.h"
#include "CSoundDevice.h"

CClient* m_client;
CCommandLine* m_commandLine;
CErrorLog* m_errorLog;
CGraphicsAdapter* m_graphicsAdapter;
CKeyboardDevice* m_keyboardDevice;
CLocal* m_local;
CMouseDevice* m_mouseDevice;

constexpr auto MAX_LOADSTRING = 100;

CSoundDevice* m_soundDevice;
CVideoDevice* m_videoDevice;
CWebBrowser* m_webBrowser;

HINSTANCE m_hInst;

HRESULT m_hr;

HWND m_hWnd;

WCHAR m_class[MAX_LOADSTRING];
WCHAR m_title[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/*
*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	hPrevInstance;

	m_hr = OleInitialize(0);

	m_commandLine = new CCommandLine();

	m_commandLine->ClientConstructor(lpCmdLine);

	m_local = new CLocal();

	CString* logFile = new CString(m_local->m_installPath->m_text);

	logFile->Append("GameClientLog");
	logFile->Append(m_commandLine->m_name);
	logFile->Append(".txt");

	m_errorLog = new CErrorLog(logFile->m_text);

	SAFE_DELETE(logFile);

	m_errorLog->WriteError(true, "m_tempFolder:%s\n", m_local->m_tempFolder->m_text);
	m_errorLog->WriteError(true, "m_edgeDataFolder:%s\n", m_local->m_edgeDataFolder->m_text);
	m_errorLog->WriteError(true, "m_localAppData:%s\n", m_local->m_localAppData->m_text);
	m_errorLog->WriteError(true, "m_exePath:%s\n", m_local->m_exePath->m_text);
	m_errorLog->WriteError(true, "m_locale:%s\n", m_local->m_locale->m_text);

	m_errorLog->WriteError(true, m_commandLine->m_arguments->m_text);
	m_errorLog->WriteError(false, "\n");

	LoadStringW(hInstance, IDS_APP_TITLE, m_title, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GAMECLIENT, m_class, MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	LoadAccelerators(m_hInst, MAKEINTRESOURCE(IDC_GAMECLIENT));

	if (strlen(m_commandLine->m_modelName) > 0)
	{
		m_client->SetModelName(m_commandLine->m_modelName);

		SendMessage(m_hWnd, WM_COMMAND, IDM_CONNECT, 0);
	}

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			DispatchMessage(&msg);
		}
		else
		{
			m_mouseDevice->GetState();

			m_keyboardDevice->GetState();

			switch (m_client->m_state)
			{
			case CClient::ClientState::E_GAME:
			{
				if (m_webBrowser->m_browserOpen)
				{
					break;
				}

				m_client->DrawGame();

				break;
			}
			case CClient::ClientState::E_LOBBY:
			{
				if (m_webBrowser->m_browserOpen)
				{
					break;
				}

				m_client->DrawLobby();

				break;
			}
			case CClient::ClientState::E_LOGIN:
			{
				if (m_webBrowser->m_browserOpen)
				{
					break;
				}

				m_client->DrawLogin();

				break;
			}
			}
		}
	}

	SAFE_DELETE(m_commandLine);
	SAFE_DELETE(m_local);
	SAFE_DELETE(m_webBrowser);
	SAFE_DELETE(m_client);
	SAFE_DELETE(m_soundDevice);
	SAFE_DELETE(m_mouseDevice);
	SAFE_DELETE(m_keyboardDevice);
	SAFE_DELETE(m_graphicsAdapter);
	SAFE_DELETE(m_errorLog);

	OleUninitialize();

	return (int)msg.wParam;
}

/*
*/
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
	wcex.lpszClassName = m_class;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

/*
*/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hInst = hInstance;

	const int w = 1440;
	const int h = 900;

	//const int w = 1920;
	//const int h = 1080;

	int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

	m_hWnd = CreateWindow(m_class, m_title, WS_POPUP,
		x, y,
		w, h,
		nullptr, nullptr, m_hInst, nullptr);

	if (!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	/*
	Back Buffer
	DXGI_FORMAT_R8G8B8A8_UNORM
	Depth Stencil
	DXGI_FORMAT_D16_UNORM
	DXGI_FORMAT_D24_UNORM_S8_UINT
	DXGI_FORMAT_D32_FLOAT
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT
	DXGI_FORMAT_UNKNOWN
	*/
	m_graphicsAdapter = new CGraphicsAdapter(m_errorLog, m_hWnd, w, h, true, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT);

	m_keyboardDevice = new CKeyboardDevice();

	m_mouseDevice = new CMouseDevice(m_graphicsAdapter, 0.125f);

	m_soundDevice = new CSoundDevice(m_errorLog);

	m_client = new CClient(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_keyboardDevice, m_errorLog, m_local);

	m_webBrowser = new CWebBrowser(m_hWnd, m_local);

	return TRUE;
}

/*
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
		if (wParam & MK_CONTROL)
		{
			return 0;
		}

		if (wParam & MK_SHIFT)
		{
			return 0;
		}

		m_mouseDevice->MouseMove(lParam);

		return 0;

		break;
	}
	case WM_RBUTTONDOWN:
	{
		m_mouseDevice->SaveLastPosition();
		m_mouseDevice->ResetFirstPerson();
		m_mouseDevice->MouseClick(CMouseDevice::E_RMB, 1);

		return 0;

		break;
	}
	case WM_RBUTTONUP:
	{
		m_mouseDevice->SetLastPosition();
		m_mouseDevice->MouseClick(CMouseDevice::E_RMB, 0);

		return 0;

		break;
	}
	case WM_LBUTTONDOWN:
	{
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
		if (m_client->m_currentCamera == nullptr)
		{
			return 0;
		}

		long wheelDelta = (long)GET_WHEEL_DELTA_WPARAM(wParam);

		m_mouseDevice->MouseWheel(wheelDelta);

		m_client->m_currentCamera->UpdateRotation(
			(m_mouseDevice->m_depth.m_lY * m_mouseDevice->m_sensitivity),
			(m_mouseDevice->m_depth.m_lX * m_mouseDevice->m_sensitivity),
			(m_mouseDevice->m_depth.m_lZ * m_mouseDevice->m_sensitivity));

		return 0;

		break;
	}
	case WM_CHAR:
	{
		m_client->m_login->UserInput((char)wParam);

		if (m_client->m_chatBox->m_isVisible)
		{
			if (wParam == VK_RETURN)
			{
				if (strlen(m_client->m_chatBox->m_message->m_text) > 0)
				{
					if (m_client->m_localClient)
					{
						strcpy_s(m_client->m_chat, CServerInfo::E_CHAT_SIZE, m_client->m_chatBox->m_message->m_text);
					}
				}

				m_client->m_chatBox->Reset();

				return 0;
			}

			if (wParam == VK_BACK)
			{
				m_client->m_chatBox->Backspace();

				return 0;
			}

			if ((wParam >= 32) && (wParam <= 126))
			{
				m_client->m_chatBox->AddChar((char)wParam);

				return 0;
			}
		}

		return 0;

		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_SHIFT)
		{
			return 0;
		}

		if (wParam == VK_CONTROL)
		{
			return 0;
		}

		CNetwork* n = m_client->m_keyboardDevice->m_keyMap[(int)wParam].m_network;

		if ((n) && (n->m_audience == CNetwork::E_CE_TO_LOCAL))
		{
			if (m_client->m_localClient)
			{
				n->SetServerInfo(m_client->m_localClient, sizeof(CServerInfo));
			}

			m_client->Send(n);

			return 0;
		}

		if (m_client->m_chatBox->m_isVisible)
		{
			if (wParam == VK_PRIOR)
			{
				m_client->m_chatBox->Paging(CChatBox::E_PAGE_BACKWARD);
			}

			if (wParam == VK_NEXT)
			{
				m_client->m_chatBox->Paging(CChatBox::E_PAGE_FORWARD);
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

		switch (wmId)
		{
		case IDM_ABOUT:
		{
			DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			return 0;

			break;
		}
		case IDM_BROWSER:
		{
			if (m_webBrowser->m_browserOpen)
			{
				return 0;

				break;
			}

			m_webBrowser->Bounds();

			CString* url = new CString(NULL, "file:///C:/Users/junk_/Videos/Captures/_video2.html?source=C:/Users/junk_/Videos/Captures/%s&width=%i&height=%i", "121B.mp4", m_webBrowser->m_sx, m_webBrowser->m_sy);

			m_webBrowser->SetURL(url->m_text);

			m_webBrowser->Create();

			SAFE_DELETE(url);

			return 0;

			break;
		}
		case IDM_BROWSEREXIT:
		{
			m_webBrowser->Close();

			return 0;

			break;
		}
		case IDM_ACCOUNT_CONNECT:
		{
			m_client->SetLogin(m_commandLine->m_name);

			m_client->ConnectAccount(m_commandLine->m_ip, m_commandLine->m_port2);

			return 0;

			break;
		}
		case IDM_ACCOUNT_DISCONNECT:
		{
			m_client->DisconnectAccount();

			return 0;

			break;
		}
		case IDM_ACCOUNT:
		{
			CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::AccountServerEvent::E_ASE_ACCOUNT_INFO,
				(void*)m_client->m_lobbyServerInfo, sizeof(CLobbyServerInfo),
				nullptr, 0);

			m_client->m_lobbySocket->Send((char*)n, sizeof(CNetwork));

			SAFE_DELETE(n);

			return 0;

			break;
		}
		case IDM_CHATBOX:
		{
			m_client->m_chatBox->ToggleVisibility();

			return 0;

			break;
		}
		case IDM_CHATMESSAGE:
		{
			m_client->m_chatBox->AddEventMessage((char*)lParam);

			return 0;

			break;
		}
		case IDM_CONNECT:
		{
			m_client->SetLogin(m_commandLine->m_name);
			m_client->SetModelName(m_commandLine->m_modelName);
			m_client->Connect(m_commandLine->m_ip, m_commandLine->m_port);
			m_client->AllocateClientList();

			return 0;

			break;
		}
		case IDM_DISCONNECT:
		{
			m_client->Disconnect();
			m_client->DestroyEnvironment();
			m_client->DestroyClientList();

			return 0;

			break;
		}
		case IDM_EXIT:
		{
			m_client->Disconnect();
			m_client->DisconnectAccount();
			m_client->DestroyEnvironment();
			m_client->DestroyClientList();

			DestroyWindow(hWnd);

			return 0;

			break;
		}
		case IDM_FULLSCREEN:
		{
			m_graphicsAdapter->ToggleFullScreenWindow();

			return 0;

			break;
		}
		}

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		
		BeginPaint(hWnd, &ps);
		
		EndPaint(hWnd, &ps);

		return 0;

		break;
	}
	case WM_SIZE:
	{
		return 0;

		break;
	}
	case WM_DESTROY:
	{
		if (hWnd != m_hWnd)
		{
			return 0;
		}

		PostQuitMessage(0);

		return 0;

		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
*/
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		return (INT_PTR)TRUE;

		break;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));

			return (INT_PTR)TRUE;
		}

		break;
	}
	}

	return (INT_PTR)FALSE;
}