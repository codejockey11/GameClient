#include "CClient.h"

/*
*/
CClient::CClient()
{
	memset(this, 0x00, sizeof(CClient));
}

/*
*/
CClient::CClient(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CKeyboardDevice* keyboardDevice, CErrorLog* errorLog, CLocal* local)
{
	memset(this, 0x00, sizeof(CClient));

	m_graphicsAdapter = graphicsAdapter;

	m_soundDevice = soundDevice;

	m_mouseDevice = mouseDevice;

	m_keyboardDevice = keyboardDevice;

	m_errorLog = errorLog;

	m_local = local;

	m_frametime = new CFrametime();

	m_network = new CNetwork();

	m_lobbyNetwork = new CNetwork();

	m_serverInfos = new CHeapArray(true, sizeof(CServerInfo), 1, CServerInfo::E_MAX_CLIENTS);

	for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		serverInfo->Constructor();
	}

	m_lobbyServerInfo = new CLobbyServerInfo();

	m_gameSocket = new CSocket(m_errorLog);

	m_lobbySocket = new CSocket(m_errorLog);

	m_model = new CObject[CServerInfo::E_MAX_CLIENTS]();

	m_camera = new CCamera[CServerInfo::E_MAX_CLIENTS]();

	m_wavefrontManager = new CWavefrontManager(m_errorLog, m_local);

	m_textureManager = new CTextureManager(m_graphicsAdapter, m_errorLog, m_local);

	m_shaderBinaryManager = new CShaderBinaryManager(m_graphicsAdapter, m_errorLog, m_local);

	m_shaderBinaryManager->Create("image.vs", CShaderBinaryManager::BinaryType::VERTEX);
	m_shaderBinaryManager->Create("image.ps", CShaderBinaryManager::BinaryType::PIXEL);

	m_fontManager = new CFontManager(m_graphicsAdapter, m_errorLog);

	CFont* consolas12 = m_fontManager->Create("Consolas 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);

	CFont* calibri12 = m_fontManager->Create("Calibri 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);

	CFont* fontArial = m_fontManager->Create("Arial 24", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD);

	m_fontArial = fontArial;

	m_consolas12 = consolas12;

	m_soundManager = new CSoundManager(m_errorLog, m_local);

	CWavLoader* logon = m_soundManager->Create("audio/stereo/logon.wav");

	m_buttonManager = new CButtonManager(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_fontManager, m_shaderBinaryManager);

	CTexture* buttonImage = m_textureManager->Create("textures/misc/97.png");

	m_buttonManager->Create("Disconnect", calibri12, logon, logon, logon, buttonImage, CVec2f(10.0f, 60.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_DISCONNECT);

	m_chatBox = new CChatBox(m_graphicsAdapter, m_errorLog, m_shaderBinaryManager, buttonImage, consolas12, 1024, 400);

	CWavLoader* sound = m_soundManager->Get("audio/stereo/logon.wav");

	m_buttonLobby = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_shaderBinaryManager, "Lobby", calibri12, sound, sound, sound, buttonImage,
		CVec2f(10.0f, 60.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CONNECT);

	m_shadowMap = new CShadowMap(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager);

	CTexture* glyphs = m_textureManager->Create("textures/misc/consolas.tga");

	m_glyphFixed = new CGlyphFixed(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager, glyphs, 18, 16);

	glyphs = m_textureManager->Create("textures/misc/calibri.tga");

	m_glyphVariable = new CGlyphVariable(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager, glyphs, 16);

	m_text1 = new CGlyphText(m_graphicsAdapter, m_errorLog, m_glyphFixed->m_commandList, m_glyphFixed->m_glyphVertices, "This is some moving text", CVec2f(50.0f, (float)m_graphicsAdapter->m_middle.y), m_glyphFixed->m_size, m_glyphFixed->m_glyphHeight);

	m_text2 = new CGlyphText(m_graphicsAdapter, m_errorLog, m_glyphFixed->m_commandList, m_glyphFixed->m_glyphVertices, "More text for reading", CVec2f(450.0f, (float)m_graphicsAdapter->m_middle.y - 100.0f), m_glyphFixed->m_size, m_glyphFixed->m_glyphHeight);

	m_text3 = new CGlyphText(m_graphicsAdapter, m_errorLog, m_glyphVariable->m_commandList, m_glyphVariable->m_glyphVertices, "This is a variable sized font", CVec2f(650.0f, (float)m_graphicsAdapter->m_middle.y + 200.0f), m_glyphVariable->m_size, m_glyphVariable->m_glyphHeight);

	CTexture* cursorImage = m_textureManager->Create("textures/misc/cursor.tga");

	m_cursor = new CImage(m_graphicsAdapter, m_errorLog, m_shaderBinaryManager, cursorImage, CVec2f((float)m_graphicsAdapter->m_middle.x, (float)m_graphicsAdapter->m_middle.y), CVec2f(8.0f, 32.0f));

	CButton* buttonLogon = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_shaderBinaryManager, "Login", calibri12, sound, sound, sound, buttonImage,
		CVec2f(10.0f, 60.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CONNECT_ACCOUNT);

	CTextInput* m_userid = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_shaderBinaryManager, calibri12, buttonImage, CVec2f(100, 10), CVec2f(256, 14));
	CTextInput* m_password = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_shaderBinaryManager, calibri12, buttonImage, CVec2f(100, 26), CVec2f(256, 14));

	m_login = new CPanel(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_shaderBinaryManager, m_cursor, buttonImage, m_fontArial, "Login", CVec2f(10, 10), CVec2f(400, 400));

	m_login->AddTextInput(m_userid);
	m_login->AddTextInput(m_password);
	m_login->AddButton(buttonLogon);

	m_method[CNetwork::ClientEvent::E_CE_ACCEPTED] = &CClient::Accepted;
	m_method[CNetwork::ClientEvent::E_CE_ACCEPTED_ACCOUNT] = &CClient::AcceptedAccount;
	m_method[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO] = &CClient::AccountInfo;
	m_method[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO_END] = &CClient::AccountInfoEnd;
	m_method[CNetwork::ClientEvent::E_CE_ACCOUNT_SERVER_FULL] = &CClient::AccountServerFull;
	m_method[CNetwork::ClientEvent::E_CE_BROWSER] = &CClient::OpenBrowser;
	m_method[CNetwork::ClientEvent::E_CE_CHAT] = &CClient::Chat;
	m_method[CNetwork::ClientEvent::E_CE_CHATBOX] = &CClient::ChatBox;
	m_method[CNetwork::ClientEvent::E_CE_CLOSE_BROWSER] = &CClient::CloseBrowser;
	m_method[CNetwork::ClientEvent::E_CE_CONNECT] = &CClient::Connect;
	m_method[CNetwork::ClientEvent::E_CE_CONNECT_ACCOUNT] = &CClient::ConnectAccount;
	m_method[CNetwork::ClientEvent::E_CE_DISCONNECT] = &CClient::DisconnectMessage;
	m_method[CNetwork::ClientEvent::E_CE_DRAW_FRAME] = &CClient::DrawFrame;
	m_method[CNetwork::ClientEvent::E_CE_ENTER] = &CClient::Enter;
	m_method[CNetwork::ClientEvent::E_CE_EXIT] = &CClient::Exit;
	m_method[CNetwork::ClientEvent::E_CE_EXIT_GAME] = &CClient::ExitGame;
	m_method[CNetwork::ClientEvent::E_CE_INFO] = &CClient::InfoRequest;
	m_method[CNetwork::ClientEvent::E_CE_LOAD_ENVIRONMENT] = &CClient::LoadEnvironment;
	m_method[CNetwork::ClientEvent::E_CE_QUE_SOUND] = &CClient::QueSound;
	m_method[CNetwork::ClientEvent::E_CE_READY_CHECK] = &CClient::ReadyCheck;
	m_method[CNetwork::ClientEvent::E_CE_SEND_ACTIVITY] = &CClient::SendActivity;
	m_method[CNetwork::ClientEvent::E_CE_SEND_NULL_ACTIVITY] = &CClient::SendNullActivity;
	m_method[CNetwork::ClientEvent::E_CE_SERVER_FULL] = &CClient::ServerFull;
	m_method[CNetwork::ClientEvent::E_CE_UPDATE] = &CClient::Update;
	m_method[CNetwork::ClientEvent::E_CE_UPDATE_COLLECTABLE] = &CClient::UpdateCollectable;
	m_method[CNetwork::ClientEvent::E_CE_WINDOW_MODE] = &CClient::WindowMode;

	m_frame[CClient::ClientState::E_GAME] = &CClient::DrawGame;
	m_frame[CClient::ClientState::E_LOBBY] = &CClient::DrawLobby;
	m_frame[CClient::ClientState::E_LOGIN] = &CClient::DrawLogin;

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
CClient::~CClient()
{
	delete m_text1;
	delete m_text2;
	delete m_text3;
	delete m_glyphFixed;
	delete m_glyphVariable;
	delete m_cursor;
	delete m_login;
	delete m_shadowMap;
	delete m_buttonLobby;
	delete m_lobbySocket;
	delete m_gameSocket;
	delete m_chatBox;
	delete m_buttonManager;
	delete m_soundManager;
	delete m_fontManager;
	delete m_shaderBinaryManager;
	delete m_textureManager;
	delete m_wavefrontManager;
	delete m_lobbyServerInfo;
	delete m_serverInfos;
	delete m_accountInfoXML;
	delete m_lobbyNetwork;
	delete m_network;
	delete m_frametime;

	WSACleanup();
}

/*
*/
void CClient::Accepted()
{
	m_errorLog->WriteError(true, "CClient::Accepted\n");

	m_localClient = m_serverInfo;

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)"CClient::Accepted\n");

	m_localClient->SetPlayerName(m_playerName);

	if (strlen(m_modelName) > 0)
	{
		m_localClient->SetModelName(m_modelName);
	}
	else
	{
		m_localClient->SetModelName("model/player.obj");
	}

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, NULL,
		(void*)m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	CClient::Send(n);

	delete n;
}

/*
*/
void CClient::AcceptedAccount()
{
	m_errorLog->WriteError(true, "CClient::AcceptedAccount\n");

	m_state = CClient::ClientState::E_LOBBY;

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)"Account Connection accepted\n");

	m_lobbyServerInfo->SetName(m_playerName);

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, NULL,
		(void*)m_lobbyServerInfo, sizeof(CLobbyServerInfo),
		nullptr, 0);

	m_lobbySocket->Send((char*)n, sizeof(CNetwork));

	delete n;
}

/*
*/
void CClient::AccountInfo()
{
	if (m_accountInfoXML == nullptr)
	{
		m_errorLog->WriteError(true, "CClient::AccountInfo\n");

		m_accountInfoXML = new CString((char*)m_lobbyNetwork->m_data);
	}
	else
	{
		m_accountInfoXML->Append((char*)m_lobbyNetwork->m_data);
	}
}

/*
*/
void CClient::AccountInfoEnd()
{
	//m_errorLog->WriteBytes(m_accountInfoXML->m_text);
	m_errorLog->WriteError(true, "CClient::AccountInfoEnd\n");

	CXML xml;

	xml.InitBuffer(m_accountInfoXML->m_text);

	while (!xml.CheckEndOfBuffer())
	{
		if (strncmp(xml.m_buffer, "<raw_text>", 10) == 0)
		{
			xml.Move(10);

			char* v = xml.GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			xml.MoveToEnd();
		}
		else if (strncmp(xml.m_buffer, "<temp_c>", 8) == 0)
		{
			xml.Move(8);

			char* v = xml.GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			xml.MoveToEnd();
		}
		else if (strncmp(xml.m_buffer, "<wind_dir_degrees>", 18) == 0)
		{
			xml.Move(18);

			char* v = xml.GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			xml.MoveToEnd();
		}
		else
		{
			xml.Move(1);
		}
	}

	delete m_accountInfoXML;
}

/*
*/
void CClient::AccountServerFull()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)"Server Full\n");
}

/*
*/
void CClient::Connect(const char* address, const char* port)
{
	m_errorLog->WriteError(true, "CClient::Connect:%s %s\n", address, port);

	CClient::InitializeWinsock();

	if (m_gameSocket->CreateConnectSocket(address, port))
	{
		m_errorLog->WriteError(true, "CClient::ReceiveThread:Starting\n");

		m_hReceiveThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient),
			&CClient::ClientThread,
			(void*)this,
			0,
			&m_receiveThreadId);
	}
}

/*
*/
void CClient::ConnectAccount(const char* address, const char* port)
{
	m_errorLog->WriteError(true, "CClient::ConnectAccount:%s %s\n", address, port);

	CClient::InitializeWinsock();

	if (m_lobbySocket->CreateConnectSocket(address, port))
	{
		m_errorLog->WriteError(true, "CClient::AccountReceiveThread:Starting\n");

		m_hAccountThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient),
			&CClient::AccountThread,
			(void*)this,
			0,
			&m_receiveAccountThreadId);
	}
}

/*
*/
void CClient::Chat()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)m_network->m_data);
}

/*
*/
void CClient::ChatBox()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATBOX, 0);
}

/*
*/
void CClient::CloseBrowser()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_BROWSEREXIT, 0);
}

/*
*/
void CClient::Connect()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONNECT, 0);
}

/*
*/
void CClient::ConnectAccount()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_ACCOUNT_CONNECT, 0);
}

/*
*/
void CClient::DestroyClientList()
{
	m_errorLog->WriteError(true, "CClient::DestroyClientList\n");

	m_graphicsAdapter->WaitForGPU();

	delete[] m_model;
	delete[] m_camera;

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::DestroyEnvironment()
{
	m_errorLog->WriteError(true, "CClient::DestroyEnvironment\n");

	int count = 0;
	while (m_isDrawing)
	{
		m_errorLog->WriteError(true, "CClient::m_isDrawing:%i\n", count);

		count++;

		if (count == 5000)
		{
			break;
		}
	}

	m_graphicsAdapter->WaitForGPU();

	delete m_map;
	delete m_overhead;
	delete m_visibility;

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::Disconnect()
{
	m_errorLog->WriteError(true, "CClient::Disconnect\n");

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_DISCONNECT,
		(void*)m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	CClient::Send(n);

	delete n;

	m_isRunning = false;

	m_errorLog->WriteError(true, "CClient::ReceiveThread Ending\n");

	WaitForSingleObject(m_hReceiveThread, INFINITE);

	CloseHandle(m_hReceiveThread);

	m_hReceiveThread = 0;

	m_gameSocket->Shutdown();

	m_state = CClient::ClientState::E_LOBBY;
}

/*
*/
void CClient::DisconnectAccount()
{
	m_errorLog->WriteError(true, "CClient::DisconnectAccount\n");

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::AccountServerEvent::E_ASE_DISCONNECT,
		(void*)m_lobbyServerInfo, sizeof(CLobbyServerInfo),
		nullptr, 0);

	m_lobbySocket->Send((char*)n, sizeof(CNetwork));

	delete n;

	m_isAccountRunning = false;

	m_errorLog->WriteError(true, "CClient::AccountThread Ending\n");

	WaitForSingleObject(m_hAccountThread, INFINITE);

	CloseHandle(m_hAccountThread);

	m_hAccountThread = 0;

	m_lobbySocket->Shutdown();

	m_state = CClient::ClientState::E_LOGIN;
}

/*
*/
void CClient::DisconnectMessage()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_DISCONNECT, 0);
}

/*
*/
void CClient::DrawFrame()
{
	if (m_state != CClient::ClientState::E_GAME)
	{
		return;
	}

	m_frametime->Frame();

	for (int i = 0; i < m_frameSoundCount; i++)
	{
		m_frameSounds[i]->StartSound();
	}

	for (int i = 0; i < m_frame3DSoundCount; i++)
	{
		m_frame3DSounds[i]->StartSound();
	}

	m_isDrawing = true;

	CClient::DrawScene();

	m_isDrawing = false;

	memset(m_frameSounds, 0x00, sizeof(CSound*) * 32);
	memset(m_frame3DSounds, 0x00, sizeof(CSound3D*) * 32);

	m_frameSoundCount = 0;
	m_frame3DSoundCount = 0;
}

/*
*/
void CClient::DrawGame()
{
	if (m_mouseDevice->m_keyMap[CMouseDevice::E_RMB].m_count > 0)
	{
		m_mouseDevice->MouseMove();

		if (m_localClient)
		{
			m_camera[m_localClient->m_clientNumber].UpdateRotation(
				((float)m_mouseDevice->m_depth.m_lY * m_mouseDevice->m_sensitivity),
				((float)m_mouseDevice->m_depth.m_lX * m_mouseDevice->m_sensitivity),
				((float)m_mouseDevice->m_depth.m_lZ * m_mouseDevice->m_sensitivity));

			CVec3f l = CVec3f(m_camera[m_localClient->m_clientNumber].m_look);

			m_model[m_localClient->m_clientNumber].m_rotation.y = l.PointToDegree().m_p.y;

			m_mouseDevice->ResetFirstPerson();
		}
	}

	CLinkListNode<CButton>* buttons = m_buttonManager->m_buttons->m_list;

	while (buttons->m_object)
	{
		buttons->m_object->CheckMouseOver();

		if (buttons->m_object->m_isMouseOver)
		{
			if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
			{
				buttons->m_object->m_network->SetServerInfo((void*)m_localClient, sizeof(CServerInfo));

				CClient::Send(buttons->m_object->m_network);

				break;
			}
		}

		buttons = buttons->m_next;
	}
}

/*
*/
void CClient::DrawHud()
{
	CLinkListNode<CButton>* buttons = m_buttonManager->m_buttons->m_list;

	while (buttons->m_object)
	{
		buttons->m_object->Record();

		m_graphicsAdapter->BundleCommandList(3, buttons->m_object->m_image->m_commandList);

		buttons = buttons->m_next;
	}

	if (m_chatBox->m_isVisible)
	{
		m_chatBox->DisplayConsole();

		m_graphicsAdapter->BundleCommandList(3, m_chatBox->m_background->m_commandList);
	}
}

/*
*/
void CClient::DrawLoading()
{
	if (m_map)
	{
		CClient::DestroyEnvironment();
	}

	CString* message = new CString(NULL, "CClient::LoadEnvironment:%s\n", (char*)m_data);

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)message->m_text);

	delete message;

	m_overhead = new COverhead(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager, CVec2f(10.0f, 10.0f), CVec2f(256.0f, 256.0f));

	CString* pathMapImage = new CString("maps/");
	pathMapImage->Append(m_data);
	pathMapImage->Append(".tga");

	CTexture* texture = new CTexture(m_graphicsAdapter, m_errorLog, m_local, m_textureManager->m_computeShader, pathMapImage->m_text);

	texture->CreateFromFilename();

	CImage* loadingScreen = new CImage(m_graphicsAdapter, m_errorLog, m_shaderBinaryManager, texture, CVec2f(10.0f, 10.0f), CVec2f(600.0f, 600.0f));

	CDirectoryList* shaderDirectory = new CDirectoryList();

	shaderDirectory->LoadFromDirectory(m_local->m_shaderInstallPath->m_text, ".ps");

	CLinkListNode<CString>* paths = shaderDirectory->m_files->m_list;

	while (paths->m_object)
	{
		int s = 0;
		for (int i = 0; i < paths->m_object->m_length; i++)
		{
			if (paths->m_object->m_text[i] == '/')
			{
				s = i + 1;
			}
		}

		m_shaderBinaryManager->Create(&paths->m_object->m_text[s], CShaderBinaryManager::BinaryType::PIXEL);

		CClient::DrawLoadingScreen(loadingScreen, m_data, (char*)"PIXEL", (char*)paths->m_object->m_text);

		paths = paths->m_next;
	}

	shaderDirectory->Reset();

	shaderDirectory->LoadFromDirectory(m_local->m_shaderInstallPath->m_text, ".vs");

	paths = shaderDirectory->m_files->m_list;

	while (paths->m_object)
	{
		int s = 0;
		for (int i = 0; i < paths->m_object->m_length; i++)
		{
			if (paths->m_object->m_text[i] == '/')
			{
				s = i + 1;
			}
		}

		m_shaderBinaryManager->Create(&paths->m_object->m_text[s], CShaderBinaryManager::BinaryType::VERTEX);

		CClient::DrawLoadingScreen(loadingScreen, m_data, (char*)"VERTEX", (char*)paths->m_object->m_text);

		paths = paths->m_next;
	}

	delete shaderDirectory;

	CClient::DrawLoadingScreen(loadingScreen, m_data, (char*)"MAP", (char*)m_data);

	m_map = new CMap(m_graphicsAdapter, m_errorLog, m_local, m_soundDevice, m_wavefrontManager, m_textureManager, m_shaderBinaryManager, m_soundManager, m_data, m_shadowMap);

	message = new CString("CClient::LoadEnvironment Completed\n");

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)message->m_text);

	delete message;

	m_errorLog->WriteError(true, "CClient::LoadEnvironment Completed\n");

	CClient::DrawLoadingScreen(loadingScreen, m_data, (char*)"COMPLETED", (char*)"Waiting on the server ...");

	delete texture;
	delete loadingScreen;

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::DrawLoadingScreen(CImage* loadingScreen, char* mapName, char* name, char* item)
{
	m_graphicsAdapter->ResetAndClearTargets();

	loadingScreen->Record();

	m_graphicsAdapter->BundleCommandList(1, loadingScreen->m_commandList);

	m_graphicsAdapter->ExecuteCommandLists();
#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	char text[132] = {};

	CVec2f position(10.0f, 620.0f);
	CVec2f size(1024.0f, 800.0f);

	sprintf_s(text, 132, "%s %s %s", mapName, name, item);

	m_fontArial->Draw(text, position, size, m_graphicsAdapter->m_color->SunYellow);

	m_graphicsAdapter->End2DDraw();
#endif
	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawLobby()
{
	m_buttonLobby->CheckMouseOver();

	if (m_buttonLobby->m_isMouseOver)
	{
		if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
		{
			CClient::Send(m_buttonLobby->m_network);
		}
	}

	m_graphicsAdapter->ResetAndClearTargets();

	m_buttonLobby->Record();

	m_graphicsAdapter->BundleCommandList(1, m_buttonLobby->m_image->m_commandList);

	m_graphicsAdapter->ExecuteCommandLists();
#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	char text[50] = {};

	CVec2f position(10.0f, 10.0f);
	CVec2f size(256.0f, 32.0f);

	sprintf_s(text, 50, "Lobby");

	m_fontArial->Draw(text, position, size, m_graphicsAdapter->m_color->SunYellow);

	m_buttonLobby->DisplayText();

	m_graphicsAdapter->End2DDraw();
#endif
	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawLogin()
{
	m_frametime->Frame();

	CNetwork* network = m_login->CheckMouseOver();

	if (network)
	{
		CClient::Send(network);
	}

	m_graphicsAdapter->ResetAndClearTargets();

	m_login->Record();


	m_glyphFixed->Record();

	m_text1->m_position.m_p.x += (64.0f * m_frametime->m_frametime);

	if (m_text1->m_position.m_p.x > (float)m_graphicsAdapter->m_width)
	{
		m_text1->m_position.m_p.x = (m_text1->m_text->m_length * -18.0f);
	}

	m_text1->Record();

	sprintf_s(m_text2->m_text->m_text, m_text2->m_text->m_length, "%f %i", m_frametime->m_frametime, m_frametime->m_rframecount);

	m_text2->Record();

	m_graphicsAdapter->BundleCommandList(4, m_glyphFixed->m_commandList);

	m_glyphVariable->Record();

	m_text3->Record();

	m_graphicsAdapter->BundleCommandList(4, m_glyphVariable->m_commandList);

	m_cursor->SetPosition(&m_mouseDevice->m_hud.m_position);

	m_cursor->Record();

	m_graphicsAdapter->BundleCommandList(4, m_cursor->m_commandList);

	m_graphicsAdapter->ExecuteCommandLists();
#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	m_login->DisplayText();

	m_graphicsAdapter->End2DDraw();
#endif
	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawScene()
{
	m_graphicsAdapter->ResetAndClearTargets();

	m_overhead->ClearTargets(m_graphicsAdapter->m_commandList);

	m_currentCamera = &m_camera[m_localClient->m_clientNumber];

	/*
	CVec3f position = CVec3f(m_currentCamera->m_position.x, m_currentCamera->m_position.y - 4.0f, m_currentCamera->m_position.z);
	CVec3f target = CVec3f(0.0f, -256.0f, 0.0f);
	m_shadowMap->m_light.Constructor(&position, &target, 0.0f);
	m_shadowMap->m_light.m_direction.m_p = m_currentCamera->m_look;

	m_shadowMap->m_light.InitForShadow();

	m_shadowMap->m_shader->m_constantBuffers[m_shadowMap->m_shader->m_b[0]].Reset();

	m_shadowMap->m_shader->m_constantBuffers[m_shadowMap->m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_world);
	m_shadowMap->m_shader->m_constantBuffers[m_shadowMap->m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
	m_shadowMap->m_shader->m_constantBuffers[m_shadowMap->m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	*/

	m_shadowMap->PreRecord();

	m_map->DrawGeometry(m_shadowMap->m_commandList);

	if (m_map->m_terrain)
	{
		m_map->m_terrain->DrawGeometry(m_shadowMap->m_commandList);
	}

	for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if (serverInfo->m_isRunning)
		{
			if (m_model[serverInfo->m_clientNumber].m_isInitialized)
			{
				m_model[serverInfo->m_clientNumber].DrawGeometry(m_shadowMap->m_commandList);
			}
		}
	}

	if (m_visibility)
	{
		for (UINT pz = 0; pz < m_visibility->m_sector->m_gridHeight; pz++)
		{
			for (UINT px = 0; px < m_visibility->m_sector->m_gridWidth; px++)
			{
				if (m_visibility->m_collectables)
				{
					CLinkList<CObject>* objects = (CLinkList<CObject>*)m_visibility->m_collectables->GetElement(2, px, pz);

					if ((objects) && (objects->m_list))
					{
						CLinkListNode<CObject>* collectables = objects->m_list;

						while (collectables->m_object)
						{
							if (!collectables->m_object->m_isVisible)
							{
								collectables->m_object->DrawGeometry(m_shadowMap->m_commandList);
							}

							collectables = collectables->m_next;
						}
					}
				}

				if (m_visibility->m_statics)
				{
					CLinkList<CObject>* objects = (CLinkList<CObject>*)m_visibility->m_statics->GetElement(2, px, pz);

					if ((objects) && (objects->m_list))
					{
						CLinkListNode<CObject>* statics = objects->m_list;

						while (statics->m_object)
						{
							statics->m_object->DrawGeometry(m_shadowMap->m_commandList);

							statics = statics->m_next;
						}
					}
				}
			}
		}
	}

	m_shadowMap->PostRecord();

	m_map->SetCurrentCamera(m_currentCamera);
	m_map->Record();

	m_map->SetOverheadCamera(m_overhead->m_camera);
	m_map->Record(m_overhead);

	m_graphicsAdapter->BundleCommandList(1, m_map->m_commandList);

	if (m_map->m_terrain)
	{
		if (m_map->m_terrain->m_isInitialized)
		{
			m_map->m_terrain->SetCurrentCamera(m_currentCamera);
			m_map->m_terrain->Record();

			m_map->m_terrain->SetOverheadCamera(m_overhead->m_camera);
			m_map->m_terrain->Record(m_overhead);

			m_graphicsAdapter->BundleCommandList(1, m_map->m_terrain->m_commandList);
		}
	}

	for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if (serverInfo->m_isRunning)
		{
			if (m_model[serverInfo->m_clientNumber].m_isInitialized)
			{
				m_model[serverInfo->m_clientNumber].SetCurrentCamera(m_currentCamera);
				m_model[serverInfo->m_clientNumber].Record();

				m_model[serverInfo->m_clientNumber].SetOverheadCamera(m_overhead->m_camera);
				m_model[serverInfo->m_clientNumber].Record(m_overhead);

				m_graphicsAdapter->BundleCommandList(2, m_model[serverInfo->m_clientNumber].m_commandList);
			}
		}
	}

	if (m_map->m_visibility)
	{
		for (UINT pz = 0; pz < m_map->m_visibility->m_sector->m_gridHeight; pz++)
		{
			for (UINT px = 0; px < m_map->m_visibility->m_sector->m_gridWidth; px++)
			{
				CLinkList<CObject>* objects = (CLinkList<CObject>*)m_map->m_visibility->m_collectables->GetElement(2, px, pz);

				if ((objects) && (objects->m_list))
				{
					CLinkListNode<CObject>* collectables = objects->m_list;

					while (collectables->m_object)
					{
						if (!collectables->m_object->m_isVisible)
						{
							collectables->m_object->SetCurrentCamera(m_currentCamera);
							collectables->m_object->Animation(m_frametime);
							collectables->m_object->Update();
							collectables->m_object->Record();

							collectables->m_object->SetOverheadCamera(m_overhead->m_camera);
							collectables->m_object->Record(m_overhead);

							m_graphicsAdapter->BundleCommandList(2, collectables->m_object->m_commandList);
						}

						collectables = collectables->m_next;
					}
				}

				objects = (CLinkList<CObject>*)m_map->m_visibility->m_statics->GetElement(2, px, pz);

				if ((objects) && (objects->m_list))
				{
					CLinkListNode<CObject>* statics = objects->m_list;

					while (statics->m_object)
					{
						statics->m_object->SetCurrentCamera(m_currentCamera);
						statics->m_object->Animation(m_frametime);
						statics->m_object->Update();
						statics->m_object->Record();

						statics->m_object->SetOverheadCamera(m_overhead->m_camera);
						statics->m_object->Record(m_overhead);

						m_graphicsAdapter->BundleCommandList(2, statics->m_object->m_commandList);

						statics = statics->m_next;
					}
				}
			}
		}
	}

	m_overhead->Record();
	m_graphicsAdapter->BundleCommandList(2, m_overhead->m_commandList);

	CClient::DrawHud();

	m_graphicsAdapter->ExecuteCommandLists();
#ifdef D3D11on12
	CClient::DrawText11On12();
#endif
	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawText11On12()
{
	m_graphicsAdapter->Init2DDraw();

	char text[132] = {};

	CVec2f position(10.0f, 10.0f);
	CVec2f size(1024.0f, 1024.0f);

	sprintf_s(text, 132, "FPS: %3i", m_frametime->m_rframecount);

	m_consolas12->Draw(text, position, size, m_graphicsAdapter->m_color->SunYellow);

	position.m_p.y += m_consolas12->m_height;

	for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		CString* name = new CString(serverInfo->m_playerName);
		UINT nl = name->m_length;
		UINT nll = 10 - nl;
		char* np = new char[nll + 1]();
		memset(np, 0x20, nll);
		name->Append(np);

		char team[5] = {};

		strncpy_s(team, 5, "Blue", 4);

		if (serverInfo->m_team == CServerInfo::Team::E_TEAM_RED)
		{
			strncpy_s(team, 5, "Red ", 4);
		}

		sprintf_s(text, 132, "%02i %s %04i %06i %02i %05i %s %s %s %03i %03i %03i %06.2f %09.2f %09.2f %09.2f %i",
			serverInfo->m_clientNumber,
			name->m_text,
			serverInfo->m_reloadTime,
			serverInfo->m_idleTime,
			serverInfo->m_countdown,
			serverInfo->m_matchTimeSeconds,
			serverInfo->m_isFreefall ? "T" : "F",
			serverInfo->m_isReconnect ? "T" : "F",
			team,
			serverInfo->m_px,
			serverInfo->m_py,
			serverInfo->m_pz,
			serverInfo->m_velocity,
			serverInfo->m_position.m_p.x,
			serverInfo->m_position.m_p.y,
			serverInfo->m_position.m_p.z,
			serverInfo->m_surface
		);

		m_consolas12->Draw(text, position, size, m_graphicsAdapter->m_color->SunYellow);

		position.m_p.y += m_consolas12->m_height;

		delete[] np;
		delete name;
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

	m_graphicsAdapter->End2DDraw();
}

/*
*/
void CClient::Enter()
{
	m_errorLog->WriteError(true, "CClient::Enter:%s\n", m_serverInfo->m_playerName);

	if (!m_camera[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		m_camera[m_serverInfo->m_clientNumber].Constructor((float)m_graphicsAdapter->m_width,
			(float)m_graphicsAdapter->m_height,
			&m_serverInfo->m_position,
			45.0f,
			1.0f, 50000.0f,
			(16.0f / 9.0f));
	}

	m_camera[m_serverInfo->m_clientNumber].UpdateRotation(
		m_serverInfo->m_direction.m_p.x,
		m_serverInfo->m_direction.m_p.y,
		m_serverInfo->m_direction.m_p.z);

	m_camera[m_serverInfo->m_clientNumber].UpdateView();

	m_serverInfo->SetDirection(&m_camera[m_serverInfo->m_clientNumber].m_look);
	m_serverInfo->SetRight(&m_camera[m_serverInfo->m_clientNumber].m_right);

	if (!m_model[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		CWavefront* wavefront = m_wavefrontManager->Create(m_serverInfo->m_modelName);

		m_model[m_serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_serverInfo->m_modelName, wavefront, m_textureManager, m_shaderBinaryManager, "client", true, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

		m_model[m_serverInfo->m_clientNumber].SetLight(m_map->m_lights);

		CLinkListNode<CMesh>* mesh = m_model[m_serverInfo->m_clientNumber].m_meshs->m_list;

		while (mesh->m_object)
		{
			switch (m_serverInfo->m_team)
			{
			case CServerInfo::E_TEAM_RED:
			{
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_emissive = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);

				break;
			}
			case CServerInfo::E_TEAM_BLUE:
			{
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_emissive = XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f);

				break;
			}
			}

			mesh = mesh->m_next;
		}

		m_model[m_serverInfo->m_clientNumber].InitMaterialBuffer();

		CWavLoader* sound = m_soundManager->Create("audio/mono/rifle.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(sound, false);

		sound = m_soundManager->Create("audio/mono/shotgun.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(sound, false);

		m_model[m_serverInfo->m_clientNumber].m_sounds[0]->SetPosition(m_serverInfo->m_position.m_p.x, m_serverInfo->m_position.m_p.y, m_serverInfo->m_position.m_p.z);
		m_model[m_serverInfo->m_clientNumber].m_sounds[1]->SetPosition(m_serverInfo->m_position.m_p.x, m_serverInfo->m_position.m_p.y, m_serverInfo->m_position.m_p.z);

		m_model[m_serverInfo->m_clientNumber].m_sounds[0]->SetRange(256.0f);
		m_model[m_serverInfo->m_clientNumber].m_sounds[1]->SetRange(256.0f);
	}

	if (m_localClient->m_clientNumber == m_serverInfo->m_clientNumber)
	{
		m_state = CClient::ClientState::E_GAME;
	}
}

/*
*/
void CClient::Exit()
{
	m_errorLog->WriteError(true, "CClient::Exit:%s\n", m_serverInfo->m_playerName);

	m_serverInfo->Reset();
}

/*
*/
void CClient::ExitGame()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_EXIT, 0);
}

/*
*/
void CClient::Frame()
{
	(this->*m_frame[m_state])();
}

/*
*/
void CClient::InfoRequest()
{
	m_errorLog->WriteError(true, "CClient::InfoRequest:%s\n", m_serverInfo->m_playerName);

	if (!m_camera[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		m_camera[m_serverInfo->m_clientNumber].Constructor((float)m_graphicsAdapter->m_width,
			(float)m_graphicsAdapter->m_height,
			&m_serverInfo->m_position,
			45.0f,
			1.0f, 50000.0f,
			(16.0f / 9.0f));
	}

	if (!m_model[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		CWavefront* wavefront = m_wavefrontManager->Create(m_serverInfo->m_modelName);

		m_model[m_serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_serverInfo->m_modelName, wavefront, m_textureManager, m_shaderBinaryManager, "client", true, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

		m_model[m_serverInfo->m_clientNumber].SetLight(m_map->m_lights);

		for (int i = 0; i < m_model[m_serverInfo->m_clientNumber].m_materialCount; i++)
		{
			CLinkListNode<CMesh>* mesh = m_model[m_serverInfo->m_clientNumber].m_meshs->m_list;

			while (mesh->m_object)
			{
				switch (m_serverInfo->m_team)
				{
				case CServerInfo::E_TEAM_RED:
				{
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_emissive = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);

					break;
				}
				case CServerInfo::E_TEAM_BLUE:
				{
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[mesh->m_object->m_material.m_number].m_emissive = XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f);

					break;
				}
				}

				mesh = mesh->m_next;
			}

		}

		m_model[m_serverInfo->m_clientNumber].InitMaterialBuffer();

		CWavLoader* sound = m_soundManager->Create("audio/mono/rifle.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(sound, false);

		sound = m_soundManager->Create("audio/mono/shotgun.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(sound, false);

		m_model[m_serverInfo->m_clientNumber].m_sounds[0]->SetPosition(&m_serverInfo->m_position);
		m_model[m_serverInfo->m_clientNumber].m_sounds[1]->SetPosition(&m_serverInfo->m_position);

		m_model[m_serverInfo->m_clientNumber].m_sounds[0]->SetRange(256.0f);
		m_model[m_serverInfo->m_clientNumber].m_sounds[1]->SetRange(256.0f);
	}

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::InitializeWinsock()
{
	WORD wVersionRequested = MAKEWORD(2, 2);

	int err = WSAStartup(wVersionRequested, &m_wsaData);

	if (err != 0)
	{
		m_errorLog->WriteError(true, "CClient::InitializeWinsock::WSAStartup:%i\n", err);

		return;
	}

	m_errorLog->WriteError(true, "CClient::InitializeWinsock::WSAStartup:%s\n", m_wsaData.szDescription);
}

/*
*/
void CClient::LoadEnvironment()
{
	m_errorLog->WriteError(true, "CClient::LoadEnvironment:%s\n", (char*)m_network->m_data);

	m_state = CClient::ClientState::E_LOADING;

	CServerInfo* serverInfo = (CServerInfo*)m_network->m_serverInfo;

	m_localClient = (CServerInfo*)m_serverInfos->GetElement(1, serverInfo->m_clientNumber);

	m_localClient->SetClient(serverInfo);

	m_clientNumber = m_localClient->m_clientNumber;

	memcpy(m_data, m_network->m_data, CServerInfo::E_CHAT_SIZE);

	m_hReceiveThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient),
		&CClient::LoadEnvironmentThread,
		(void*)this,
		0,
		&m_loadEnvironmentThreadId);
}

/*
*/
void CClient::OpenBrowser()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_BROWSER, 0);
}

/*
*/
void CClient::ProcessAccountEvent()
{
	CLobbyServerInfo* serverInfo = (CLobbyServerInfo*)m_lobbyNetwork->m_serverInfo;

	m_lobbyServerInfo->Initialize(serverInfo);

	(this->*m_method[m_lobbyNetwork->m_type])();
}

/*
*/
void CClient::ProcessEvent()
{
	CServerInfo* serverInfo = (CServerInfo*)m_network->m_serverInfo;

	m_serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, serverInfo->m_clientNumber);

	m_serverInfo->SetClient(serverInfo);

	(this->*m_method[m_network->m_type])();
}

/*
*/
void CClient::QueSound()
{
	m_errorLog->WriteError(true, "CClient::QueSound:%s\n", m_network->m_data);

	int i = atoi((char*)m_network->m_data);

	CSound3D* sound = m_model[m_serverInfo->m_clientNumber].m_sounds[i];

	CClient::QueSound(sound);
}

/*
*/
void CClient::QueSound(CSound* sound)
{
	if (sound == nullptr)
	{
		m_errorLog->WriteError(true, "CClient::QueSound:sound == nullptr\n");

		return;
	}

	m_errorLog->WriteError(true, "CClient::QueSound:%s\n", sound->m_wavLoader->m_filename->m_text);

	m_frameSounds[m_frameSoundCount] = sound;
	m_frameSoundCount++;
}

/*
*/
void CClient::QueSound(CSound3D* sound)
{
	if (sound == nullptr)
	{
		m_errorLog->WriteError(true, "CClient::QueSound(3D):sound == nullptr\n");

		return;
	}

	m_errorLog->WriteError(true, "CClient::QueSound(3D):%s\n", sound->m_wavLoader->m_filename->m_text);

	X3DAUDIO_VECTOR p = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_position.x, m_camera[m_localClient->m_clientNumber].m_position.y, m_camera[m_localClient->m_clientNumber].m_position.z);
	X3DAUDIO_VECTOR l = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_look.x, m_camera[m_localClient->m_clientNumber].m_look.y, m_camera[m_localClient->m_clientNumber].m_look.z);
	X3DAUDIO_VECTOR u = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_up.x, m_camera[m_localClient->m_clientNumber].m_up.y, m_camera[m_localClient->m_clientNumber].m_up.z);

	sound->SetListener(&p, &l, &u);

	m_frame3DSounds[m_frame3DSoundCount] = sound;
	m_frame3DSoundCount++;
}

/*
*/
void CClient::ReadyCheck()
{
	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_READY_CHECK,
		(void*)m_localClient, sizeof(CServerInfo),
		NULL, 0);

	CClient::Send(n);

	delete n;
}

/*
*/
void CClient::Send(CNetwork* network)
{
	if (network->m_audience == CNetwork::ClientEvent::E_CE_TO_LOCAL)
	{
		(this->*m_method[network->m_type])();

		return;
	}

	m_gameSocket->Send((char*)network, sizeof(CNetwork));
}

/*
*/
void CClient::SendActivity()
{
	BYTE types[10] = {};
	int typeCount = 0;

	for (int i = 0; i < CMouseDevice::E_BUTTON_COUNT; i++)
	{
		if ((m_mouseDevice->m_keyMap[i].m_count > 0) && (m_mouseDevice->m_keyMap[i].m_network))
		{
			if (m_mouseDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				types[typeCount] = m_mouseDevice->m_keyMap[i].m_network->m_type;

				typeCount++;
			}
		}
	}

	for (int i = 0; i < CKeyboardDevice::E_MAX_KEYS; i++)
	{
		if ((m_keyboardDevice->m_keyMap[i].m_count > 0) && (m_keyboardDevice->m_keyMap[i].m_network))
		{
			if (m_keyboardDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				types[typeCount] = m_keyboardDevice->m_keyMap[i].m_network->m_type;

				typeCount++;
			}
		}
	}

	if (m_camera[m_localClient->m_clientNumber].m_isInitialized)
	{
		m_localClient->SetDirection(&m_camera[m_localClient->m_clientNumber].m_look);
		m_localClient->SetRight(&m_camera[m_localClient->m_clientNumber].m_right);
	}

	m_localClient->m_clientNumber = m_clientNumber;
	m_localClient->SetPlayerName(m_playerName);

	if (strlen(m_chat) > 0)
	{
		CString* message = new CString(m_localClient->m_playerName);

		message->Append(":");
		message->Append(m_chat);

		m_localClient->SetChat(message->m_text);

		delete message;

		memset(m_chat, 0x00, CServerInfo::E_CHAT_SIZE);
	}

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_ACTIVITY,
		(void*)m_localClient, sizeof(CServerInfo),
		(void*)types, typeCount);

	CClient::Send(n);

	delete n;
}

/*
*/
void CClient::SendNullActivity()
{
	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_NULL_ACTIVITY,
		(void*)m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	CClient::Send(n);

	delete n;
}

/*
*/
void CClient::ServerFull()
{
	CString* message = new CString("CClient::ServerFull\n");

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)message->m_text);

	delete message;
}

/*
*/
void CClient::SetLogin(const char* name)
{
	m_errorLog->WriteError(true, "CClient::SetLogin:%s\n", name);

	memset(m_playerName, 0x00, CServerInfo::E_PLAYER_NAME_SIZE);

	memcpy((void*)m_playerName, (void*)name, strlen(name));
}

/*
*/
void CClient::SetModelName(const char* modelName)
{
	m_errorLog->WriteError(true, "CClient::SetModelName:%s\n", modelName);

	char player[] = "model/player.obj";
	char monkey[] = "model/monkey.obj";

	memset(m_modelName, 0x00, CServerInfo::E_MODEL_NAME_SIZE);

	if (strncmp(modelName, "Player", 6) == 0)
	{
		memcpy((void*)m_modelName, (void*)player, strlen(player));
	}

	if (strncmp(modelName, "Monkey", 6) == 0)
	{
		memcpy((void*)m_modelName, (void*)monkey, strlen(monkey));
	}
}

/*
*/
void CClient::Update()
{
	if (!m_camera[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		CClient::Enter();

		return;
	}

	m_camera[m_serverInfo->m_clientNumber].SetPosition(&m_serverInfo->m_position);
	m_camera[m_serverInfo->m_clientNumber].UpdateView();

	//m_overhead->SetCameraPosition(m_camera[m_serverInfo->m_clientNumber]->m_position);

	if (m_overhead)
	{
		if (m_camera[m_localClient->m_clientNumber].m_isInitialized)
		{
			m_overhead->SetCameraPosition(m_camera[m_localClient->m_clientNumber].m_position);
			m_overhead->m_camera->UpdateView();
		}
	}

	m_model[m_serverInfo->m_clientNumber].SetPosition(&m_serverInfo->m_position);

	m_model[m_serverInfo->m_clientNumber].m_rotation.y = m_serverInfo->m_direction.PointToDegree().m_p.y;

	m_model[m_serverInfo->m_clientNumber].Update();

	m_model[m_serverInfo->m_clientNumber].m_sounds[0]->SetPosition(&m_serverInfo->m_position);
	m_model[m_serverInfo->m_clientNumber].m_sounds[1]->SetPosition(&m_serverInfo->m_position);
}

/*
*/
void CClient::UpdateCollectable()
{
	m_errorLog->WriteError(true, "CClient::UpdateCollectable:%s\n", m_network->m_data);

	int px = 0;
	int pz = 0;

	char itemName[CKeyValue::MAX_VALUE] = {};
	char inLimbo = 'f';

	sscanf_s((char*)m_network->m_data, "%s %i %i %c", itemName, CKeyValue::MAX_VALUE, &px, &pz, &inLimbo, 1);

	CLinkList<CObject>* collectables = (CLinkList<CObject>*)m_map->m_visibility->m_collectables->GetElement(2, px, pz);

	if ((collectables) && (collectables->m_list))
	{
		CLinkListNode<CObject>* collectable = collectables->Search(itemName);

		if (collectable)
		{
			collectable->m_object->m_isVisible = true;

			if (inLimbo == 't')
			{
				collectable->m_object->m_isVisible = false;

				CClient::QueSound(collectable->m_object->m_sounds[0]);
			}
			else
			{
				CClient::QueSound(collectable->m_object->m_sounds[1]);
			}
		}
	}
}

/*
*/
void CClient::WindowMode()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
}

/*
*/
unsigned __stdcall CClient::AccountThread(void* obj)
{
	CClient* client = (CClient*)obj;

	client->m_isAccountRunning = true;

	while (client->m_isAccountRunning)
	{
		int bytes = client->m_lobbySocket->Receive((char*)client->m_lobbyNetwork, sizeof(CNetwork));

		if (bytes > 0)
		{
			client->ProcessAccountEvent();
		}
		else
		{
			client->m_isAccountRunning = false;

			client->m_state = CClient::ClientState::E_LOGIN;
		}
	}

	_endthreadex(0);

	return 0;
}

/*
*/
unsigned __stdcall CClient::ClientThread(void* obj)
{
	CClient* client = (CClient*)obj;

	client->m_isRunning = true;

	while (client->m_isRunning)
	{
		int bytes = client->m_gameSocket->Receive((char*)client->m_network, sizeof(CNetwork));

		if (bytes > 0)
		{
			client->ProcessEvent();
		}
		else
		{
			client->m_isRunning = false;

			if (client->m_localClient)
			{
				client->m_localClient->m_isConnected = false;
			}

			client->m_state = CClient::ClientState::E_LOBBY;
		}
	}

	_endthreadex(0);

	return 0;
}

/*
*/
unsigned __stdcall CClient::LoadEnvironmentThread(void* obj)
{
	CClient* client = (CClient*)obj;

	client->DrawLoading();

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_READY,
		(void*)client->m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	client->Send(n);

	delete n;

	_endthreadex(0);

	return 0;
}