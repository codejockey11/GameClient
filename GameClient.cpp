#include "framework.h"

#include "../GameCommon/CCommandLine.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CWebBrowser.h"
#include "../GameCommon/CWindow.h"
#include "../GameCommon/CXML.h"

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
CSoundDevice* m_soundDevice;
CVideoDevice* m_videoDevice;
CWebBrowser* m_webBrowser;
CWindow* m_window;
CXML* m_xml;

HRESULT m_hr;

LRESULT CALLBACK WndProc(HWND, uint32_t, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, uint32_t, WPARAM, LPARAM);

/*
*/
int32_t APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int32_t nCmdShow)
{
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
	m_errorLog->WriteError(true, "m_installPath:%s\n", m_local->m_installPath->m_text);

	m_errorLog->WriteError(true, m_commandLine->m_arguments->m_text);
	m_errorLog->WriteError(false, "\n");

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

	m_window = new CWindow(hInstance, WndProc, "GameClass", IDC_GAMECLIENT, IDI_ICON1, IDI_ICON2, "Game Window", 1440, 900, 0, 0);

	m_graphicsAdapter = new CGraphicsAdapter(m_errorLog, m_window->m_hWnd, m_window->m_width, m_window->m_height, true, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT);

	m_keyboardDevice = new CKeyboardDevice();

	m_mouseDevice = new CMouseDevice(m_graphicsAdapter, 0.25f);

	m_soundDevice = new CSoundDevice(m_errorLog);

	m_webBrowser = new CWebBrowser(m_window->m_hWnd, m_local);

	m_client = new CClient(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_keyboardDevice, m_errorLog, m_local);


	if (strlen(m_commandLine->m_modelName) > 0)
	{
		m_client->SetModelName(m_commandLine->m_modelName);

		SendMessage(m_window->m_hWnd, WM_COMMAND, IDM_CONNECT, 0);

		Sleep(100);
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

			for (int32_t i = 0; i < CKeyboardDevice::E_MAX_KEYS; i++)
			{
				if ((m_keyboardDevice->m_keyMap[i].m_count == 1) && (m_keyboardDevice->m_keyMap[i].m_repeats == CKeyboardDevice::E_KR_ONCE))
				{
					if (m_keyboardDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_LOCAL)
					{
						m_client->SendLocal(m_keyboardDevice->m_keyMap[i].m_network);
					}
				}
				else if (m_client->m_console->m_isVisible)
				{
					if ((i == VK_PRIOR) && (m_keyboardDevice->m_keyMap[i].m_count > 0))
					{
						m_client->m_console->Paging(CConsole::E_PAGE_BACKWARD);

						Sleep(50);
					}

					if ((i == VK_NEXT) && (m_keyboardDevice->m_keyMap[i].m_count > 0))
					{
						m_client->m_console->Paging(CConsole::E_PAGE_FORWARD);

						Sleep(50);
					}
				}
			}

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
			case CClient::ClientState::E_LOADING:
			{
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

	// Wait for internal threads to end (ie. XAudio2.9)
	Sleep(1000);

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

	SAFE_DELETE(m_window);

	return (int32_t)msg.wParam;
}

/*
*/
LRESULT CALLBACK WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
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
		if (m_client->m_console->m_isVisible)
		{
			if (wParam == VK_RETURN)
			{
				if (strlen(m_client->m_console->m_message->m_text) > 0)
				{
					m_client->SetData(m_client->m_console->m_message->m_text);

					switch (m_client->m_state)
					{
					case(CClient::ClientState::E_LOBBY):
					{
						m_client->SendLobbyMessage();

						break;
					}
					}
				}

				m_client->m_console->Reset();

				return 0;
			}

			if (wParam == VK_BACK)
			{
				m_client->m_console->Backspace();

				return 0;
			}

			if ((wParam >= 32) && (wParam <= 126))
			{
				m_client->m_console->AddChar((char)wParam);

				return 0;
			}
		}

		if (m_client->m_state == CClient::ClientState::E_LOGIN)
		{
			m_client->m_login->UserInput((char)wParam);
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

		m_keyboardDevice->KeyDown((int32_t)wParam);

		return 0;

		break;
	}
	case WM_KEYUP:
	{
		m_keyboardDevice->KeyUp((int32_t)wParam);

		return 0;

		break;
	}
	case WM_COMMAND:
	{
		int32_t wmId = LOWORD(wParam);

		switch (wmId)
		{
		case IDM_ABOUT:
		{
			DialogBox(m_window->m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

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

			CString* url = new CString(NULL, "http://127.0.0.1:49151/game/video.html?source=../captures/%s&width=%i&height=%i", "121B.mp4", m_webBrowser->m_sx, m_webBrowser->m_sy);

			m_webBrowser->SetURL(url->m_text);

			m_webBrowser->Create();

			SAFE_DELETE(url);

			return 0;

			break;
		}
		case IDM_BROWSER_EXIT:
		{
			m_webBrowser->Close();

			return 0;

			break;
		}
		case IDM_ACCOUNT_CONNECT:
		{
			m_client->SetLogin(m_commandLine->m_name);

			m_client->ConnectLobby(m_commandLine->m_ip, m_commandLine->m_port2);

			return 0;

			break;
		}
		case IDM_ACCOUNT_DISCONNECT:
		{
			m_client->DisconnectLobby();

			return 0;

			break;
		}
		case IDM_ACCOUNT_INFO:
		{
			CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::LobbyEvent::E_LE_ACCOUNT_INFO,
				(void*)m_client->m_lobbyLocalClient, sizeof(CLobbyServerInfo),
				nullptr, 0);

			m_client->m_lobbySocket->Send((char*)n, sizeof(CNetwork));

			SAFE_DELETE(n);

			return 0;

			break;
		}
		case IDM_CONSOLE:
		{
			m_client->m_console->ToggleVisibility();

			return 0;

			break;
		}
		case IDM_CONSOLE_MESSAGE:
		{
			m_client->m_console->AddEventMessage((char*)lParam);

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
			if (m_client)
			{
				m_client->Disconnect();
				m_client->DisconnectLobby();
				m_client->DestroyEnvironment();
				m_client->DestroyClientList();
			}

			PostQuitMessage(0);

			return 0;

			break;
		}
		case IDM_FULLSCREEN:
		{
			m_graphicsAdapter->ToggleFullScreenWindow();

			return 0;

			break;
		}
		case CWebBrowser::E_IDM_BROWSER_FROM_DOCUMENT:
		{
			if (lParam == 0)
			{
				return 0;
			}

			if (strncmp((const char*)lParam, "ERROR", 5) == 0)
			{
				m_errorLog->WriteError(true, "CWebBrowser::E_IDM_BROWSER_FROM_DOCUMENT::ERROR:Website not available\n");

				m_webBrowser->Close();

				SAFE_DELETE(m_webBrowser);

				m_webBrowser = new CWebBrowser(m_window->m_hWnd, m_local);

				SetFocus(m_window->m_hWnd);

				return 0;
			}

			m_xml = new CXML();

			m_xml->InitBuffer((const char*)lParam);

			while (!m_xml->CheckEndOfBuffer())
			{
				if (strncmp(m_xml->m_buffer, "<video>", 7) == 0)
				{
					m_xml->MoveToTag("<exit>");

					m_xml->Move(6);

					char* v = m_xml->GetValue();

					if (strncmp(v, "Y", 1) == 0)
					{
						m_webBrowser->Close();

						break;
					}
				}
				else if (strncmp(m_xml->m_buffer, "<game>", 6) == 0)
				{
					m_xml->MoveToTag("<exit>");

					m_xml->Move(6);

					char* v = m_xml->GetValue();

					if (strncmp(v, "Y", 1) == 0)
					{
						m_webBrowser->Close();

						SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_EXIT, 0);

						break;
					}
				}

				m_xml->Move(1);
			}

			SAFE_DELETE(m_xml);

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
	case WM_CLOSE:
	case WM_DESTROY:
	{
		SendMessage(m_window->m_hWnd, WM_COMMAND, IDM_EXIT, 0);

		return 0;

		break;
	}
	case WM_SETCURSOR:
	{
		while (ShowCursor(FALSE) >= 0);

		return 0;

		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
*/
INT_PTR CALLBACK About(HWND hDlg, uint32_t message, WPARAM wParam, LPARAM lParam)
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