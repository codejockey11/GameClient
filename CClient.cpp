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

	CClient::InitializeManagers();
	CClient::InitializeNetworking();
	CClient::InitializeShaders();
	CClient::InitializeGraphics();
	CClient::InitializeSounds();
	CClient::InitializeLoginPanel();

	m_event[CNetwork::ClientEvent::E_CE_ACCEPTED] = &CClient::Accepted;
	m_event[CNetwork::ClientEvent::E_CE_ACCEPTED_ACCOUNT] = &CClient::AcceptedAccount;
	m_event[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO] = &CClient::AccountInfo;
	m_event[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO_END] = &CClient::AccountInfoEnd;
	m_event[CNetwork::ClientEvent::E_CE_ACCOUNT_SERVER_FULL] = &CClient::AccountServerFull;
	m_event[CNetwork::ClientEvent::E_CE_BROWSER] = &CClient::OpenBrowser;
	m_event[CNetwork::ClientEvent::E_CE_CLOSE_BROWSER] = &CClient::CloseBrowser;
	m_event[CNetwork::ClientEvent::E_CE_CONNECT] = &CClient::Connect;
	m_event[CNetwork::ClientEvent::E_CE_CONNECT_ACCOUNT] = &CClient::ConnectAccount;
	m_event[CNetwork::ClientEvent::E_CE_CONSOLE] = &CClient::Console;
	m_event[CNetwork::ClientEvent::E_CE_CONSOLE_MESSAGE] = &CClient::ConsoleMessage;
	m_event[CNetwork::ClientEvent::E_CE_DISCONNECT] = &CClient::DisconnectMessage;
	m_event[CNetwork::ClientEvent::E_CE_DRAW_FRAME] = &CClient::DrawFrame;
	m_event[CNetwork::ClientEvent::E_CE_ENTER] = &CClient::Enter;
	m_event[CNetwork::ClientEvent::E_CE_EXIT] = &CClient::Exit;
	m_event[CNetwork::ClientEvent::E_CE_EXIT_GAME] = &CClient::ExitGame;
	m_event[CNetwork::ClientEvent::E_CE_INFO] = &CClient::InfoRequest;
	m_event[CNetwork::ClientEvent::E_CE_LOAD_ENVIRONMENT] = &CClient::LoadEnvironment;
	m_event[CNetwork::ClientEvent::E_CE_QUE_SOUND] = &CClient::QueSound;
	m_event[CNetwork::ClientEvent::E_CE_READY_CHECK] = &CClient::ReadyCheck;
	m_event[CNetwork::ClientEvent::E_CE_SEND_ACTIVITY] = &CClient::SendActivity;
	m_event[CNetwork::ClientEvent::E_CE_SEND_NULL_ACTIVITY] = &CClient::SendNullActivity;
	m_event[CNetwork::ClientEvent::E_CE_SERVER_FULL] = &CClient::ServerFull;
	m_event[CNetwork::ClientEvent::E_CE_UPDATE] = &CClient::Update;
	m_event[CNetwork::ClientEvent::E_CE_UPDATE_COLLECTABLE] = &CClient::UpdateCollectable;
	m_event[CNetwork::ClientEvent::E_CE_WINDOW_MODE] = &CClient::WindowMode;

	m_frame[CClient::ClientState::E_GAME] = &CClient::DrawGame;
	m_frame[CClient::ClientState::E_LOBBY] = &CClient::DrawLobby;
	m_frame[CClient::ClientState::E_LOGIN] = &CClient::DrawLogin;

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
CClient::~CClient()
{
	SAFE_DELETE(m_overheadCameraConstantBuffer);
	SAFE_DELETE(m_cameraConstantBuffer);
	SAFE_DELETE(m_terrainPipelineState);
	SAFE_DELETE(m_clientPipelineState);
	SAFE_DELETE(m_collectablePipelineState);
	SAFE_DELETE(m_imagePipelineState);
	SAFE_DELETE(m_mapPipelineState);

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		SAFE_DELETE(m_commandListGroup[i]);
	}

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		SAFE_DELETE(m_scoreboardText[i]);
	}

	SAFE_DELETE(m_fpsText);
	SAFE_DELETE(m_glyphFixed);
	SAFE_DELETE(m_glyphVariable);
	SAFE_DELETE(m_cursor);
	SAFE_DELETE(m_login);
	SAFE_DELETE(m_shadowMap);
	SAFE_DELETE(m_lobbySocket);
	SAFE_DELETE(m_gameSocket);
	SAFE_DELETE(m_console);
	SAFE_DELETE(m_buttonManager);
	SAFE_DELETE(m_soundManager);
	SAFE_DELETE(m_fontManager);
	SAFE_DELETE(m_shaderBinaryManager);
	SAFE_DELETE(m_textureManager);
	SAFE_DELETE(m_wavefrontManager);
	SAFE_DELETE(m_lobbyServerInfo);
	SAFE_DELETE(m_serverInfos);
	SAFE_DELETE(m_accountInfoXML);
	SAFE_DELETE(m_lobbyNetwork);
	SAFE_DELETE(m_gameNetwork);
	SAFE_DELETE(m_frametime);

	WSACleanup();
}

/*
*/
void CClient::Accepted()
{
	m_errorLog->WriteError(true, "CClient::Accepted\n");

	m_localClient = m_serverInfo;

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)"CClient::Accepted\n");

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

	SAFE_DELETE(n);
}

/*
*/
void CClient::AcceptedAccount()
{
	m_errorLog->WriteError(true, "CClient::AcceptedAccount\n");

	m_state = CClient::ClientState::E_LOBBY;

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)"Account Connection accepted\n");

	m_lobbyServerInfo->SetName(m_playerName);

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, NULL,
		(void*)m_lobbyServerInfo, sizeof(CLobbyServerInfo),
		nullptr, 0);

	m_lobbySocket->Send((char*)n, sizeof(CNetwork));

	SAFE_DELETE(n);
}

/*
*/
void CClient::AccountInfo()
{
	if (m_accountInfoXML == nullptr)
	{
		m_errorLog->WriteError(true, "CClient::AccountInfo\n");

		m_accountInfoXML = new CString((const char*)m_lobbyNetwork->m_data);
	}
	else
	{
		m_accountInfoXML->Append((const char*)m_lobbyNetwork->m_data);
	}
}

/*
*/
void CClient::AccountInfoEnd()
{
	m_errorLog->WriteError(true, "CClient::AccountInfoEnd\n");

	m_errorLog->WriteBytes(m_accountInfoXML->m_text);

	CXML* xml = new CXML();

	xml->InitBuffer(m_accountInfoXML->m_text);

	while (true)
	{
		if (xml->CheckEndOfBuffer())
		{
			break;
		}

		if (strncmp(xml->m_buffer, "<raw_text>", 10) == 0)
		{
			xml->Move(10);

			char* v = xml->GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)v);

			xml->MoveToEnd();
		}
		else if (strncmp(xml->m_buffer, "<temp_c>", 8) == 0)
		{
			xml->Move(8);

			char* v = xml->GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)v);

			xml->MoveToEnd();
		}
		else if (strncmp(xml->m_buffer, "<wind_dir_degrees>", 18) == 0)
		{
			xml->Move(18);

			char* v = xml->GetValue();

			m_errorLog->WriteError(true, "%s\n", v);

			SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)v);

			xml->MoveToEnd();
		}
		else
		{
			xml->Move(1);
		}
	}

	SAFE_DELETE(xml);
	SAFE_DELETE(m_accountInfoXML);
}

/*
*/
void CClient::AccountServerFull()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)"Server Full\n");
}

/*
*/
void CClient::AllocateClientList()
{
	m_model = new CObject[CServerInfo::E_MAX_CLIENTS]();

	m_camera = new CCamera[CServerInfo::E_MAX_CLIENTS]();
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
void CClient::CloseBrowser()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_BROWSER_EXIT, 0);
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
void CClient::Console()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE, 0);
}

/*
*/
void CClient::ConsoleMessage()
{
	switch (m_state)
	{
	case (CClient::ClientState::E_GAME):
	{
		SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)m_gameNetwork->m_data);

		break;
	}
	case (CClient::ClientState::E_LOBBY):
	{
		SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)m_lobbyNetwork->m_data);

		break;
	}
	}
}

/*
*/
void CClient::DestroyClientList()
{
	m_errorLog->WriteError(true, "CClient::DestroyClientList\n");

	m_graphicsAdapter->WaitForGPU();

	SAFE_DELETE_ARRAY(m_model);
	SAFE_DELETE_ARRAY(m_camera);

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::DestroyEnvironment()
{
	m_errorLog->WriteError(true, "CClient::DestroyEnvironment\n");

	m_graphicsAdapter->WaitForGPU();

	SAFE_DELETE(m_map);
	SAFE_DELETE(m_shadowMap);
	SAFE_DELETE(m_overhead);

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

	Sleep(500);

	SAFE_DELETE(n);

	m_isRunning = false;

	m_errorLog->WriteError(true, "CClient::ReceiveThread:Ending\n");

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

	Sleep(500);

	SAFE_DELETE(n);

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

	m_isDrawing = true;

	m_frametime->Frame();

	for (int32_t i = 0; i < m_frameSoundCount; i++)
	{
		m_frameSounds[i]->StartSound();
	}

	for (int32_t i = 0; i < m_frame3DSoundCount; i++)
	{
		m_frame3DSounds[i]->StartSound();
	}

	CClient::DrawScene();

	memset(m_frameSounds, 0x00, sizeof(CSound*) * 32);
	memset(m_frame3DSounds, 0x00, sizeof(CSound3D*) * 32);

	m_frameSoundCount = 0;
	m_frame3DSoundCount = 0;

	m_isDrawing = false;
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

	CListNode* buttonNode = m_buttonManager->m_buttons->m_list;

	while ((buttonNode) && (buttonNode->m_object))
	{
		CButton* button = (CButton*)buttonNode->m_object;

		button->CheckMouseOver();

		if (button->m_isMouseOver)
		{
			if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
			{
				button->m_network->SetServerInfo((void*)m_localClient, sizeof(CServerInfo));

				CClient::Send(button->m_network);

				break;
			}
		}

		buttonNode = buttonNode->m_next;
	}
}

/*
*/
void CClient::DrawHud()
{
	CListNode* buttonNode = m_buttonManager->m_buttons->m_list;

	while ((buttonNode) && (buttonNode->m_object))
	{
		CButton* button = (CButton*)buttonNode->m_object;

		button->m_image->RecordConstantBuffer();

		m_commandListGroup[0]->Add(button->m_image->m_constantBuffer->m_commandList);

		button->Record();

		m_commandListGroup[button->m_image->m_listGroup]->Add(button->m_image->m_commandList);

		buttonNode = buttonNode->m_next;
	}

	if (m_console->m_isVisible)
	{
		m_console->m_background->RecordConstantBuffer();

		m_commandListGroup[0]->Add(m_console->m_background->m_constantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}
}

/*
*/
void CClient::DrawLoading()
{
	CString* message = new CString(NULL, "Loading:%s\n", m_mapName->m_text);

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)message->m_text);

	SAFE_DELETE(message);

	m_shadowMap = new CShadowMap(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager);

	m_overhead = new COverhead(m_graphicsAdapter, m_errorLog, m_local, m_imagePipelineState, 3, 0, CVec2f(10.0f, 10.0f), CVec2f(256.0f, 256.0f));

	CString* pathMapImage = new CString(m_local->m_installPath->m_text);

	pathMapImage->Append("main/maps/");
	pathMapImage->Append(m_mapName->m_text);
	pathMapImage->Append(".tga");

	CTexture* texture = new CTexture(m_graphicsAdapter, m_errorLog, m_textureManager->m_computeShader, m_mapName->m_text);

	texture->CreateFromFilename(pathMapImage->m_text);

	SAFE_DELETE(pathMapImage);

	CImage* loadingScreen = new CImage(m_graphicsAdapter, m_errorLog, m_imagePipelineState, 1, texture, CVec2f(10.0f, 10.0f), CVec2f(600.0f, 600.0f));

	CClient::DrawLoadingScreen(loadingScreen, m_mapName->m_text, (char*)"Environment", (char*)"Loading ...");

	CString* pathMapFile = new CString(m_local->m_installPath->m_text);

	pathMapFile->Append("main/maps/");
	pathMapFile->Append(m_mapName->m_text);
	pathMapFile->Append(".dat");

	m_map = new CMap(m_graphicsAdapter, m_errorLog, m_local, m_soundDevice, m_wavefrontManager, m_textureManager, m_mapPipelineState, m_collectablePipelineState,
		m_terrainPipelineState, m_soundManager, 1, m_shadowMap, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_mapName->m_text, pathMapFile->m_text);

	SAFE_DELETE(pathMapFile);

	message = new CString("Load Environment Completed\n");

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)message->m_text);

	SAFE_DELETE(message);

	m_errorLog->WriteError(true, "CClient::LoadEnvironment Completed\n");

	CClient::DrawLoadingScreen(loadingScreen, m_mapName->m_text, (char*)"Load Environment Completed", (char*)"Waiting on the server ...");

	m_graphicsAdapter->WaitForGPU();

	SAFE_DELETE(texture);
	SAFE_DELETE(loadingScreen);

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::DrawLoadingScreen(CImage* loadingScreen, char* mapName, char* name, char* item)
{
	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->Clear();
	}

	m_graphicsAdapter->Record();

	m_commandListGroup[0]->Add(m_graphicsAdapter->m_commandList);

	loadingScreen->RecordConstantBuffer();

	m_commandListGroup[0]->Add(loadingScreen->m_constantBuffer->m_commandList);

	loadingScreen->Record();

	m_commandListGroup[loadingScreen->m_listGroup]->Add(loadingScreen->m_commandList);

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->ExecuteCommandLists();
	}

#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	char text[132] = {};

	CVec2f position(10.0f, 620.0f);
	CVec2f size(1024.0f, 800.0f);

	sprintf_s(text, 132, "%s %s %s", mapName, name, item);

	m_fontArial->Draw(text, position, size, m_graphicsAdapter->m_rgbColor->SunYellow);

	m_graphicsAdapter->End2DDraw();
#endif

	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawLobby()
{
	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->Clear();
	}

	m_graphicsAdapter->Record();

	m_commandListGroup[0]->Add(m_graphicsAdapter->m_commandList);

	if (m_console->m_isVisible)
	{
		m_console->m_background->RecordConstantBuffer();

		m_commandListGroup[0]->Add(m_console->m_background->m_constantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}

	m_cursor->SetPosition(&m_mouseDevice->m_hud.m_position);

	m_cursor->RecordConstantBuffer();

	m_commandListGroup[0]->Add(m_cursor->m_constantBuffer->m_commandList);

	m_cursor->Record();

	m_commandListGroup[m_cursor->m_listGroup]->Add(m_cursor->m_commandList);

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->ExecuteCommandLists();
	}

#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	char text[50] = {};

	CVec2f position(10.0f, 10.0f);
	CVec2f size(256.0f, 32.0f);

	sprintf_s(text, 50, "Lobby");

	m_fontArial->Draw(text, position, size, m_graphicsAdapter->m_rgbColor->SunYellow);

	if (m_console->m_isVisible)
	{
		m_console->DisplayText();
	}

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

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->Clear();
	}

	m_graphicsAdapter->Record();

	m_commandListGroup[0]->Add(m_graphicsAdapter->m_commandList);


	m_login->m_background->RecordConstantBuffer();

	m_commandListGroup[0]->Add(m_login->m_background->m_constantBuffer->m_commandList);

	m_login->Record();

	m_commandListGroup[m_login->m_background->m_listGroup]->Add(m_login->m_background->m_commandList);


	CListNode* node = m_login->m_buttons->m_list;

	while ((node) && (node->m_object))
	{
		CButton* button = (CButton*)node->m_object;

		button->m_image->RecordConstantBuffer();

		m_commandListGroup[0]->Add(button->m_image->m_constantBuffer->m_commandList);

		m_commandListGroup[button->m_image->m_listGroup]->Add(button->m_image->m_commandList);

		node = node->m_next;
	}


	node = m_login->m_textInputs->m_list;

	while ((node) && (node->m_object))
	{
		CTextInput* textInput = (CTextInput*)node->m_object;

		textInput->m_image->RecordConstantBuffer();

		m_commandListGroup[0]->Add(textInput->m_image->m_constantBuffer->m_commandList);

		m_commandListGroup[textInput->m_image->m_listGroup]->Add(textInput->m_image->m_commandList);

		node = node->m_next;
	}


	if (m_console->m_isVisible)
	{
		m_console->m_background->RecordConstantBuffer();

		m_commandListGroup[0]->Add(m_console->m_background->m_constantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}


	m_cursor->SetPosition(&m_mouseDevice->m_hud.m_position);

	m_cursor->RecordConstantBuffer();

	m_commandListGroup[0]->Add(m_cursor->m_constantBuffer->m_commandList);

	m_cursor->Record();

	m_commandListGroup[m_cursor->m_listGroup]->Add(m_cursor->m_commandList);


	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->ExecuteCommandLists();
	}

#ifdef D3D11on12
	m_graphicsAdapter->Init2DDraw();

	m_login->DisplayText();

	if (m_console->m_isVisible)
	{
		m_console->DisplayText();
	}

	m_graphicsAdapter->End2DDraw();
#endif

	m_graphicsAdapter->MoveToNextFrame();
}

/*
*/
void CClient::DrawScene()
{
	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->Clear();
	}

	m_graphicsAdapter->Record();

	m_commandListGroup[0]->Add(m_graphicsAdapter->m_commandList);

	m_overhead->RecordClear();

	m_commandListGroup[m_overhead->m_listGroupClear]->Add(m_overhead->m_commandListClear);

	m_currentCamera = &m_camera[m_localClient->m_clientNumber];

	/*
	CVec3f position =
		CVec3f(
		m_currentCamera->m_position.x + (m_currentCamera->m_look.x * -16.0f),
		m_currentCamera->m_position.y + (m_currentCamera->m_look.y * -16.0f),
		m_currentCamera->m_position.z + (m_currentCamera->m_look.z * -16.0f));

	CVec3f target = CVec3f(m_currentCamera->m_look.x, m_currentCamera->m_look.y, m_currentCamera->m_look.z);
	
	m_shadowMap->m_light->Constructor(&position, &target, 1.0f);

	m_shadowMap->m_light->m_vUp = XMVectorSet(m_currentCamera->m_up.x, m_currentCamera->m_up.y, m_currentCamera->m_up.z, 1.0f);

	m_shadowMap->m_light->InitForShadow();

	m_shadowMap->m_constantBuffer->Reset();

	m_shadowMap->m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_world);
	m_shadowMap->m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
	m_shadowMap->m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
	
	m_shadowMap->m_constantBuffer->Record();

	m_shadowMap->m_constantBuffer->m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_shadowMap->m_constantBuffer->m_commandList.GetAddressOf());
	*/

	m_shadowMap->PreRecord();

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if ((serverInfo->m_isRunning) || (serverInfo->m_isBot))
		{
			m_model[serverInfo->m_clientNumber].DrawGeometry(m_shadowMap->m_commandList);
		}
	}

	if (m_map->m_visibility)
	{
		for (int32_t y = 0; y < m_map->m_visibility->m_sector->m_gridHeight; y++)
		{
			for (int32_t z = 0; z < m_map->m_visibility->m_sector->m_gridDepth; z++)
			{
				for (int32_t x = 0; x < m_map->m_visibility->m_sector->m_gridWidth; x++)
				{
					if (m_map->m_visibility->m_collectables)
					{
						CList* collectables = (CList*)m_map->m_visibility->m_collectables->GetElement(3, x, z, y);

						if ((collectables) && (collectables->m_list))
						{
							CListNode* node = collectables->m_list;

							while ((node) && (node->m_object))
							{
								CObject* object = (CObject*)node->m_object;

								if ((object->m_isInitialized) && (object->m_isVisible))
								{
									object->DrawGeometry(m_shadowMap->m_commandList);
								}

								node = node->m_next;
							}
						}
					}

					if (m_map->m_visibility->m_statics)
					{
						CList* statics = (CList*)m_map->m_visibility->m_statics->GetElement(3, x, z, y);

						if ((statics) && (statics->m_list))
						{
							CListNode* node = statics->m_list;

							while ((node) && (node->m_object))
							{
								CObject* object = (CObject*)node->m_object;

								if ((object->m_isInitialized) && (object->m_isVisible))
								{
									object->DrawGeometry(m_shadowMap->m_commandList);
								}

								node = node->m_next;
							}
						}
					}
				}
			}
		}
	}

	m_shadowMap->PostRecord();

	m_cameraConstantBuffer->Reset();

	m_cameraConstantBuffer->m_values->Append(m_currentCamera->m_xmworld);
	m_cameraConstantBuffer->m_values->Append(m_currentCamera->m_xmview);
	m_cameraConstantBuffer->m_values->Append(m_currentCamera->m_xmproj);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
	m_cameraConstantBuffer->m_values->Append(m_currentCamera->m_position);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(m_currentCamera->m_look);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
	m_cameraConstantBuffer->m_values->Append(1.0f);

	m_cameraConstantBuffer->Record();

	m_commandListGroup[0]->Add(m_cameraConstantBuffer->m_commandList);

	m_overheadCameraConstantBuffer->Reset();

	m_overheadCameraConstantBuffer->m_values->Append(m_overhead->m_camera->m_xmworld);
	m_overheadCameraConstantBuffer->m_values->Append(m_overhead->m_camera->m_xmview);
	m_overheadCameraConstantBuffer->m_values->Append(m_overhead->m_camera->m_xmproj);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
	m_overheadCameraConstantBuffer->m_values->Append(m_overhead->m_camera->m_position);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(m_overhead->m_camera->m_look);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);

	m_overheadCameraConstantBuffer->Record();

	m_commandListGroup[0]->Add(m_overheadCameraConstantBuffer->m_commandList);


	m_map->RecordConstantBuffers();

	m_commandListGroup[m_map->m_listGroup]->Add(m_map->m_lightConstantBuffer->m_commandList);

	for (int32_t i = 0; i < m_map->m_numberOfMaterials; i++)
	{
		m_commandListGroup[m_map->m_listGroup]->Add(m_map->m_materialConstantBuffers[i].m_commandList);
	}

	m_map->Record();
	m_map->Record(m_overhead);

	m_commandListGroup[m_map->m_listGroup]->Add(m_map->m_commandList);

	if (m_map->m_terrain)
	{
		if (m_map->m_terrain->m_isInitialized)
		{
			m_map->m_terrain->RecordConstantBuffers();

			m_commandListGroup[m_map->m_terrain->m_listGroup]->Add(m_map->m_terrain->m_blendConstantBuffer->m_commandList);

			m_commandListGroup[m_map->m_terrain->m_listGroup]->Add(m_map->m_terrain->m_materialConstantBuffer->m_commandList);

			m_map->m_terrain->Record();
			m_map->m_terrain->Record(m_overhead);

			m_commandListGroup[m_map->m_terrain->m_listGroup]->Add(m_map->m_terrain->m_commandList);
		}
	}

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if ((serverInfo->m_isRunning) || (serverInfo->m_isBot))
		{
			m_model[serverInfo->m_clientNumber].RecordConstantBuffers();

			for (int32_t ii = 0; ii < m_model[serverInfo->m_clientNumber].m_materialCount; ii++)
			{
				m_commandListGroup[m_model[serverInfo->m_clientNumber].m_listGroup]->Add(m_model[serverInfo->m_clientNumber].m_materialConstantBuffers[ii].m_commandList);
			}

			m_model[serverInfo->m_clientNumber].Record();
			m_model[serverInfo->m_clientNumber].Record(m_overhead);

			m_commandListGroup[m_model[serverInfo->m_clientNumber].m_listGroup]->Add(m_model[serverInfo->m_clientNumber].m_commandList);
		}
	}

	if (m_map->m_visibility)
	{
		for (int32_t y = 0; y < m_map->m_visibility->m_sector->m_gridHeight; y++)
		{
			for (int32_t z = 0; z < m_map->m_visibility->m_sector->m_gridDepth; z++)
			{
				for (int32_t x = 0; x < m_map->m_visibility->m_sector->m_gridWidth; x++)
				{
					CList* collectables = (CList*)m_map->m_visibility->m_collectables->GetElement(3, x, z, y);

					if ((collectables) && (collectables->m_list))
					{
						CListNode* node = collectables->m_list;

						while ((node) && (node->m_object))
						{
							CObject* object = (CObject*)node->m_object;

							if ((object->m_isInitialized) && (object->m_isVisible))
							{
								object->RecordConstantBuffers();

								for (int32_t ii = 0; ii < object->m_materialCount; ii++)
								{
									m_commandListGroup[object->m_listGroup]->Add(object->m_materialConstantBuffers[ii].m_commandList);
								}

								object->Animation(m_frametime);
								object->Update();
								object->Record();

								object->Record(m_overhead);

								m_commandListGroup[object->m_listGroup]->Add(object->m_commandList);
							}

							node = node->m_next;
						}
					}

					CList* statics = (CList*)m_map->m_visibility->m_statics->GetElement(3, x, z, y);

					if ((statics) && (statics->m_list))
					{
						CListNode* node = statics->m_list;

						while ((node) && (node->m_object))
						{
							CObject* object = (CObject*)node->m_object;

							if ((object->m_isInitialized) && (object->m_isVisible))
							{
								object->RecordConstantBuffers();

								for (int32_t ii = 0; ii < object->m_materialCount; ii++)
								{
									m_commandListGroup[object->m_listGroup]->Add(object->m_materialConstantBuffers[ii].m_commandList);
								}

								object->Animation(m_frametime);
								object->Update();
								object->Record();

								object->Record(m_overhead);

								m_commandListGroup[object->m_listGroup]->Add(object->m_commandList);
							}

							node = node->m_next;
						}
					}
				}
			}
		}
	}

	m_overhead->RecordConstantBuffer();

	m_commandListGroup[m_overhead->m_listGroup]->Add(m_overhead->m_constantBuffer->m_commandList);

	m_overhead->Record();

	m_commandListGroup[m_overhead->m_listGroup]->Add(m_overhead->m_commandList);

	CClient::DrawHud();

	char text[132] = {};

	sprintf_s(text, 132, "FPS: %3i", m_frametime->m_rframecount);

	m_fpsText->SetText(text);

	m_fpsText->RecordConstantBuffer();

	m_commandListGroup[m_fpsText->m_listGroup]->Add(m_fpsText->m_constantBuffer->m_commandList);

	m_fpsText->Record();

	m_commandListGroup[m_fpsText->m_listGroup]->Add(m_fpsText->m_commandList);

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		CString* name = new CString(serverInfo->m_playerName);
		int32_t nl = name->m_length;
		int32_t nll = 10 - nl;
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

		m_scoreboardText[i]->SetText(text);

		m_scoreboardText[i]->RecordConstantBuffer();

		m_commandListGroup[m_scoreboardText[i]->m_listGroup]->Add(m_scoreboardText[i]->m_constantBuffer->m_commandList);

		m_scoreboardText[i]->Record();

		m_commandListGroup[m_scoreboardText[i]->m_listGroup]->Add(m_scoreboardText[i]->m_commandList);

		SAFE_DELETE_ARRAY(np);
		SAFE_DELETE(name);
	}

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->ExecuteCommandLists();
	}

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

	CListNode* node = m_buttonManager->m_buttons->m_list;

	while ((node) && (node->m_object))
	{
		CButton* button = (CButton*)node->m_object;

		button->DisplayText();

		node = node->m_next;
	}

	if (m_console->m_isVisible)
	{
		m_console->DisplayText();
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
		m_wavefront = m_wavefrontManager->Create(m_serverInfo->m_modelName);

		m_model[m_serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_serverInfo->m_modelName, m_wavefront,
			m_textureManager, m_clientPipelineState, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_map->m_lightConstantBuffer, true, 2, m_shadowMap);

		if (m_serverInfo->m_isBot)
		{
			m_model[m_serverInfo->m_clientNumber].m_scale.x = 8.0f;
			m_model[m_serverInfo->m_clientNumber].m_scale.y = 8.0f;
			m_model[m_serverInfo->m_clientNumber].m_scale.z = 8.0f;
		}

		m_node = m_model[m_serverInfo->m_clientNumber].m_meshs->m_list;

		while ((m_node) && (m_node->m_object))
		{
			m_mesh = (CMesh*)m_node->m_object;

			switch (m_serverInfo->m_team)
			{
			case CServerInfo::E_TEAM_RED:
			{
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_emissive = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);

				break;
			}
			case CServerInfo::E_TEAM_BLUE:
			{
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
				m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_emissive = XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f);

				break;
			}
			}

			m_node = m_node->m_next;
		}

		m_model[m_serverInfo->m_clientNumber].InitMaterialBuffer();

		m_sound = m_soundManager->Create("audio/mono/rifle.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(m_sound, false);

		m_sound = m_soundManager->Create("audio/mono/shotgun.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(m_sound, false);

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
		m_wavefront = m_wavefrontManager->Create(m_serverInfo->m_modelName);

		m_model[m_serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_serverInfo->m_modelName, m_wavefront,
			m_textureManager, m_collectablePipelineState, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_map->m_lightConstantBuffer, true, 2, m_shadowMap);

		for (int32_t i = 0; i < m_model[m_serverInfo->m_clientNumber].m_materialCount; i++)
		{
			m_node = m_model[m_serverInfo->m_clientNumber].m_meshs->m_list;

			while ((m_node) && (m_node->m_object))
			{
				m_mesh = (CMesh*)m_node->m_object;

				switch (m_serverInfo->m_team)
				{
				case CServerInfo::E_TEAM_RED:
				{
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_emissive = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);

					break;
				}
				case CServerInfo::E_TEAM_BLUE:
				{
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
					m_model[m_serverInfo->m_clientNumber].m_shaderMaterials[m_mesh->m_material.m_number].m_emissive = XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f);

					break;
				}
				}

				m_node = m_node->m_next;
			}
		}

		m_model[m_serverInfo->m_clientNumber].InitMaterialBuffer();

		m_sound = m_soundManager->Create("audio/mono/rifle.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(m_sound, false);

		m_sound = m_soundManager->Create("audio/mono/shotgun.wav");

		m_model[m_serverInfo->m_clientNumber].AddSound(m_sound, false);

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
	m_versionRequested = MAKEWORD(2, 2);

	m_err = WSAStartup(m_versionRequested, &m_wsaData);

	if (m_err != 0)
	{
		m_errorLog->WriteError(true, "CClient::InitializeWinsock::WSAStartup:%i\n", m_err);

		return;
	}

	m_errorLog->WriteError(true, "CClient::InitializeWinsock::WSAStartup:%s\n", m_wsaData.szDescription);
}

/*
*/
void CClient::InitializeGraphics()
{
	m_frametime = new CFrametime();

	m_calibri12 = m_fontManager->Create("Calibri 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);

	m_consolas12 = m_fontManager->Create("Consolas 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);

	m_fontArial = m_fontManager->Create("Arial 24", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD);

	CShaderBinary* m_vs = m_shaderBinaryManager->Get("image.vs");
	CShaderBinary* m_ps = m_shaderBinaryManager->Get("image.ps");

	m_imagePipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 1, 1, CPipelineState::VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_imagePipelineState->SetPixelShader(m_ps);
	m_imagePipelineState->SetVertexShader(m_vs);

	m_imagePipelineState->Create(false, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_vs = m_shaderBinaryManager->Get("map.vs");
	m_ps = m_shaderBinaryManager->Get("map.ps");

	m_mapPipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 4, 11, CPipelineState::VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_mapPipelineState->SetPixelShader(m_ps);
	m_mapPipelineState->SetVertexShader(m_vs);

	m_mapPipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_vs = m_shaderBinaryManager->Get("collectable.vs");
	m_ps = m_shaderBinaryManager->Get("collectable.ps");

	m_collectablePipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 4, 11, CPipelineState::VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_collectablePipelineState->SetPixelShader(m_ps);
	m_collectablePipelineState->SetVertexShader(m_vs);

	m_collectablePipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_vs = m_shaderBinaryManager->Get("client.vs");
	m_ps = m_shaderBinaryManager->Get("client.ps");

	m_clientPipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 4, 11, CPipelineState::VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_clientPipelineState->SetPixelShader(m_ps);
	m_clientPipelineState->SetVertexShader(m_vs);

	m_clientPipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_vs = m_shaderBinaryManager->Get("terrain.vs");
	m_ps = m_shaderBinaryManager->Get("terrain.ps");

	m_terrainPipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 4, 11, CPipelineState::VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_terrainPipelineState->SetPixelShader(m_ps);
	m_terrainPipelineState->SetVertexShader(m_vs);

	m_terrainPipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_cameraConstantBuffer = new CConstantBuffer();

	m_cameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 384);

	m_overheadCameraConstantBuffer = new CConstantBuffer();

	m_overheadCameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 384);

	CTexture* consoleImage = m_textureManager->Create("textures/misc/console.jpg");

	m_console = new CConsole(m_graphicsAdapter, m_errorLog, m_imagePipelineState, 5, consoleImage, m_consolas12, 1024, 400);

	CTexture* glyphs = m_textureManager->Create("textures/misc/consolas.tga");

	m_glyphFixed = new CGlyphFixed(glyphs, 18, 18);

	glyphs = m_textureManager->Create("textures/misc/calibri.tga");

	m_glyphVariable = new CGlyphVariable(glyphs, 16);

	CTexture* cursorImage = m_textureManager->Create("textures/misc/cursor.tga");

	m_cursor = new CImage(m_graphicsAdapter, m_errorLog, m_imagePipelineState, 6, cursorImage,
		CVec2f((float)m_graphicsAdapter->m_middle.x, (float)m_graphicsAdapter->m_middle.y), CVec2f(8.0f, 32.0f));

	CVec2f position(10.0f, 10.0f);

	m_fpsText = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imagePipelineState, m_glyphFixed, 4, 8, position, 14);

	position.m_p.y += m_consolas12->m_height;

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		m_scoreboardText[i] = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imagePipelineState, m_glyphFixed, 4, 132, position, 14);

		position.m_p.y += m_consolas12->m_height;

		m_scoreboardText[i]->m_position.m_p.y = position.m_p.y;
	}
}

/*
*/
void CClient::InitializeLoginPanel()
{
	CWavLoader* sound = m_soundManager->Create("audio/stereo/logon.wav");

	CTexture* buttonImage = m_textureManager->Create("textures/misc/button.tga");

	CButton* buttonLogon = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_imagePipelineState, 1, "Login", m_calibri12, sound, sound, sound, buttonImage,
		CVec2f(160.0f, 160.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CONNECT_ACCOUNT);

	CTexture* panelImage = m_textureManager->Create("textures/brick/roughbrick01.tga");

	CTexture* textInputImage = m_textureManager->Create("textures/misc/button.tga");

	CTextInput* m_userid = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imagePipelineState, 1, m_calibri12, textInputImage, CVec2f(100, 110), CVec2f(256, 14));
	CTextInput* m_password = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imagePipelineState, 1, m_calibri12, textInputImage, CVec2f(100, 126), CVec2f(256, 14));

	m_login = new CPanel(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imagePipelineState, 1, m_cursor, panelImage, m_fontArial, "Login", CVec2f(10, 10), CVec2f(400, 400));

	m_login->AddTextInput(m_userid);
	m_login->AddTextInput(m_password);
	m_login->AddButton(buttonLogon);
}

/*
*/
void CClient::InitializeManagers()
{
	m_fontManager = new CFontManager(m_graphicsAdapter, m_errorLog);

	m_shaderBinaryManager = new CShaderBinaryManager(m_graphicsAdapter, m_errorLog, m_local);

	m_soundManager = new CSoundManager(m_errorLog, m_local, "audio/mono/ding.wav");

	m_textureManager = new CTextureManager(m_graphicsAdapter, m_errorLog, m_local, "textures/misc/black.tga");

	m_wavefrontManager = new CWavefrontManager(m_errorLog, m_local, "model/sphere.obj");

	m_buttonManager = new CButtonManager(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_fontManager, m_imagePipelineState);

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i] = new CCommandListGroup(m_graphicsAdapter, m_errorLog);
	}
}

/*
*/
void CClient::InitializeNetworking()
{
	m_gameNetwork = new CNetwork();

	m_lobbyNetwork = new CNetwork();

	m_serverInfos = new CHeapArray(true, sizeof(CServerInfo), 1, CServerInfo::E_MAX_CLIENTS);

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		serverInfo->Constructor();
	}

	m_lobbyServerInfo = new CLobbyServerInfo();

	m_gameSocket = new CSocket(m_errorLog);

	m_lobbySocket = new CSocket(m_errorLog);
}

/*
*/
void CClient::InitializeShaders()
{
	m_directoryList = new CDirectoryList();

	CString* shaderDirectory = new CString(m_local->m_installPath->m_text);

	shaderDirectory->Append("shaders/");

	m_directoryList->LoadFromDirectory(shaderDirectory->m_text, "vs");

	m_node = m_directoryList->m_filenames->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CString* name = (CString*)m_node->m_object;

		m_shaderBinaryManager->Create(m_local->m_installPath->m_text, name->m_text, CShaderBinaryManager::BinaryType::VERTEX);

		m_node = m_node->m_next;
	}

	m_directoryList->Reset();

	m_directoryList->LoadFromDirectory(shaderDirectory->m_text, "ps");

	m_node = m_directoryList->m_filenames->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CString* name = (CString*)m_node->m_object;

		m_shaderBinaryManager->Create(m_local->m_installPath->m_text, name->m_text, CShaderBinaryManager::BinaryType::PIXEL);

		m_node = m_node->m_next;
	}

	SAFE_DELETE(shaderDirectory);
	SAFE_DELETE(m_directoryList);
}

/*
*/
void CClient::InitializeSounds()
{
	m_soundManager->Create("audio/stereo/logon.wav");
}

/*
*/
void CClient::LoadEnvironment()
{
	m_errorLog->WriteError(true, "CClient::LoadEnvironment:%s\n", (char*)m_gameNetwork->m_data);

	CClient::DestroyEnvironment();

	m_state = CClient::ClientState::E_LOADING;

	CServerInfo* serverInfo = (CServerInfo*)m_gameNetwork->m_serverInfo;

	m_localClient = (CServerInfo*)m_serverInfos->GetElement(1, serverInfo->m_clientNumber);

	m_localClient->SetClient(serverInfo);

	m_clientNumber = m_localClient->m_clientNumber;

	m_mapName = new CString((const char*)m_gameNetwork->m_data);

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

	(this->*m_event[m_lobbyNetwork->m_type])();

	m_lobbyNetwork->SetData(nullptr, 0);
}

/*
*/
void CClient::ProcessEvent()
{
	CServerInfo* serverInfo = (CServerInfo*)m_gameNetwork->m_serverInfo;

	m_serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, serverInfo->m_clientNumber);

	m_serverInfo->SetClient(serverInfo);

	(this->*m_event[m_gameNetwork->m_type])();

	m_gameNetwork->SetData(nullptr, 0);
}

/*
*/
void CClient::QueSound()
{
	m_errorLog->WriteError(true, "CClient::QueSound:%s\n", m_gameNetwork->m_data);

	m_index = atoi((char*)m_gameNetwork->m_data);

	m_sound3D = m_model[m_serverInfo->m_clientNumber].m_sounds[m_index];

	CClient::QueSound(m_sound3D);
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

	m_errorLog->WriteError(true, "CClient::QueSound:%s\n", sound->m_wavLoader->m_name->m_text);

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

	m_errorLog->WriteError(true, "CClient::QueSound(3D):%s\n", sound->m_wavLoader->m_name->m_text);

	m_position = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_position.x, m_camera[m_localClient->m_clientNumber].m_position.y, m_camera[m_localClient->m_clientNumber].m_position.z);
	m_look = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_look.x, m_camera[m_localClient->m_clientNumber].m_look.y, m_camera[m_localClient->m_clientNumber].m_look.z);
	m_up = X3DAUDIO_VECTOR(m_camera[m_localClient->m_clientNumber].m_up.x, m_camera[m_localClient->m_clientNumber].m_up.y, m_camera[m_localClient->m_clientNumber].m_up.z);

	sound->SetListener(&m_position, &m_look, &m_up);

	m_frame3DSounds[m_frame3DSoundCount] = sound;
	m_frame3DSoundCount++;
}

/*
*/
void CClient::ReadyCheck()
{
	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_READY_CHECK,
		(void*)m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	CClient::Send(n);

	SAFE_DELETE(n);
}

/*
*/
void CClient::Send(CNetwork* network)
{
	m_gameSocket->Send((char*)network, sizeof(CNetwork));
}

/*
*/
void CClient::SendActivity()
{
	char events[10] = {};

	int32_t eventCount = 0;

	for (int32_t i = 0; i < CMouseDevice::E_BUTTON_COUNT; i++)
	{
		if ((m_mouseDevice->m_keyMap[i].m_count > 0) && (m_mouseDevice->m_keyMap[i].m_network))
		{
			if (m_mouseDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				events[eventCount] = m_mouseDevice->m_keyMap[i].m_network->m_type;

				eventCount++;
			}
		}
	}

	for (int32_t i = 0; i < CKeyboardDevice::E_MAX_KEYS; i++)
	{
		if ((m_keyboardDevice->m_keyMap[i].m_count > 0) && (m_keyboardDevice->m_keyMap[i].m_network))
		{
			if (m_keyboardDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				events[eventCount] = m_keyboardDevice->m_keyMap[i].m_network->m_type;

				eventCount++;
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

	m_localClient->SetActivity(events);

	if (strlen(m_data) > 0)
	{
		CString* message = new CString(m_localClient->m_playerName);

		message->Append(":");
		message->Append(m_data);

		memset(m_data, 0x00, CNetwork::E_DATA_SIZE);

		memcpy(m_data, message->m_text, message->m_length);

		SAFE_DELETE(message);
	}

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_ACTIVITY,
		(void*)m_localClient, sizeof(CServerInfo),
		(void*)m_data, (int32_t)strlen(m_data));

	CClient::Send(n);

	memset(m_data, 0x00, CNetwork::E_DATA_SIZE);

	SAFE_DELETE(n);
}

/*
*/
void CClient::SendLobbyMessage()
{
	if (strlen(m_data) > 0)
	{
		CString* message = new CString(m_lobbyServerInfo->m_name);

		message->Append(":");
		message->Append(m_data);

		CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::AccountServerEvent::E_ASE_NULL_ACTIVITY,
			(void*)m_lobbyServerInfo, sizeof(CLobbyServerInfo),
			(void*)message->m_text, message->m_length);

		m_lobbySocket->Send((char*)n, sizeof(CNetwork));

		memset(m_data, 0x00, CNetwork::E_DATA_SIZE);

		SAFE_DELETE(n);
		SAFE_DELETE(message);
	}
}

/*
*/
void CClient::SendLocal(CNetwork* network)
{
	(this->*m_event[network->m_type])();
}

/*
*/
void CClient::SendNullActivity()
{
	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_NULL_ACTIVITY,
		(void*)m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	CClient::Send(n);

	SAFE_DELETE(n);
}

/*
*/
void CClient::ServerFull()
{
	m_errorLog->WriteError(true, "CClient::ServerFull\n");

	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONSOLE_MESSAGE, (LPARAM)"CClient::ServerFull\n");

	m_gameSocket->Shutdown();

	m_isRunning = false;

	CloseHandle(m_hReceiveThread);

	m_hReceiveThread = 0;
}

/*
*/
void CClient::SetData(const char* data)
{
	memcpy(m_data, data, strlen(data));
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
	while (m_isDrawing)
	{

	}

	if (!m_camera[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		CClient::Enter();

		return;
	}

	m_camera[m_serverInfo->m_clientNumber].SetPosition(&m_serverInfo->m_position);
	m_camera[m_serverInfo->m_clientNumber].UpdateView();

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
	m_errorLog->WriteError(true, "CClient::UpdateCollectable:%s\n", m_gameNetwork->m_data);

	int32_t px = 0;
	int32_t py = 0;
	int32_t pz = 0;

	char itemName[CKeyValue::MAX_VALUE] = {};
	char inLimbo = 'f';

	sscanf_s((char*)m_gameNetwork->m_data, "%s %i %i %i %c", itemName, CKeyValue::MAX_VALUE, &px, &py, &pz, &inLimbo, 1);

	m_collectables = (CList*)m_map->m_visibility->m_collectables->GetElement(3, px, pz, py);

	if ((m_collectables) && (m_collectables->m_list))
	{
		m_node = m_collectables->Search(itemName);

		if (m_node)
		{
			m_object = (CObject*)m_node->m_object;

			if (inLimbo == 't')
			{
				m_object->m_isVisible = false;

				CClient::QueSound(m_object->m_sounds[0]);
			}
			else
			{
				m_object->m_isVisible = true;

				CClient::QueSound(m_object->m_sounds[1]);
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
		int32_t bytes = client->m_lobbySocket->Receive((char*)client->m_lobbyNetwork, sizeof(CNetwork));

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
		int32_t bytes = client->m_gameSocket->Receive((char*)client->m_gameNetwork, sizeof(CNetwork));

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

	SAFE_DELETE(n);
	SAFE_DELETE(client->m_mapName);

	_endthreadex(0);

	return 0;
}