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
	m_event[CNetwork::ClientEvent::E_CE_ACCEPTED_LOBBY] = &CClient::AcceptedLobby;
	m_event[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO] = &CClient::AccountInfo;
	m_event[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO_END] = &CClient::AccountInfoEnd;
	m_event[CNetwork::ClientEvent::E_CE_BROWSER] = &CClient::OpenBrowser;
	m_event[CNetwork::ClientEvent::E_CE_CLOSE_BROWSER] = &CClient::CloseBrowser;
	m_event[CNetwork::ClientEvent::E_CE_COMPLETE_LOBBY_CONNECT] = &CClient::CompleteLobbyConnect;
	m_event[CNetwork::ClientEvent::E_CE_CONNECT] = &CClient::Connect;
	m_event[CNetwork::ClientEvent::E_CE_CONNECT_LOBBY] = &CClient::ConnectLobby;
	m_event[CNetwork::ClientEvent::E_CE_CONSOLE] = &CClient::Console;
	m_event[CNetwork::ClientEvent::E_CE_CONSOLE_MESSAGE] = &CClient::ConsoleMessage;
	m_event[CNetwork::ClientEvent::E_CE_DISCONNECT] = &CClient::DisconnectMessage;
	m_event[CNetwork::ClientEvent::E_CE_DRAW_FRAME] = &CClient::DrawFrame;
	m_event[CNetwork::ClientEvent::E_CE_ENTER] = &CClient::Enter;
	m_event[CNetwork::ClientEvent::E_CE_EXIT] = &CClient::Exit;
	m_event[CNetwork::ClientEvent::E_CE_EXIT_GAME] = &CClient::ExitGame;
	m_event[CNetwork::ClientEvent::E_CE_LOADING] = &CClient::Loading;
	m_event[CNetwork::ClientEvent::E_CE_LOAD_ENVIRONMENT] = &CClient::LoadEnvironment;
	m_event[CNetwork::ClientEvent::E_CE_LOAD_PLAYER] = &CClient::LoadPlayer;
	m_event[CNetwork::ClientEvent::E_CE_LOBBY_SERVER_FULL] = &CClient::LobbyServerFull;
	m_event[CNetwork::ClientEvent::E_CE_QUE_SOUND] = &CClient::QueSound;
	m_event[CNetwork::ClientEvent::E_CE_READY_CHECK] = &CClient::ReadyCheck;
	m_event[CNetwork::ClientEvent::E_CE_REQUEST_ACCOUNT_INFO] = &CClient::RequestAccountInfo;
	m_event[CNetwork::ClientEvent::E_CE_SEND_ACTIVITY] = &CClient::SendActivity;
	m_event[CNetwork::ClientEvent::E_CE_SEND_NULL_ACTIVITY] = &CClient::SendNullActivity;
	m_event[CNetwork::ClientEvent::E_CE_SERVER_FULL] = &CClient::ServerFull;
	m_event[CNetwork::ClientEvent::E_CE_SERVER_SHUTDOWN] = &CClient::ServerShutdown;
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
	SAFE_DELETE(m_imageWvp);
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
	SAFE_DELETE(m_pointer);
	SAFE_DELETE(m_login);
	SAFE_DELETE(m_shadowMap);
	SAFE_DELETE(m_lobbySocket);
	SAFE_DELETE(m_gameSocket);
	SAFE_DELETE(m_console);
	SAFE_DELETE(m_buttonManager);
	SAFE_DELETE(m_soundManagerHud);
	SAFE_DELETE(m_soundManager);
	SAFE_DELETE(m_fontManager);
	SAFE_DELETE(m_shaderBinaryManager);
	SAFE_DELETE(m_textureManagerHud);
	SAFE_DELETE(m_textureManager);
	SAFE_DELETE(m_wavefrontManager);
	SAFE_DELETE(m_lobbyLocalClient);
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
void CClient::AcceptedLobby()
{
	m_errorLog->WriteError(true, "CClient::AcceptedLobby\n");

	m_lobbyLocalClient->SetClient(m_lobbyServerInfo);

	m_lobbyLocalClient->SetName(m_playerName);

	m_state = CClient::ClientState::E_LOBBY;

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, NULL,
		(void*)m_lobbyLocalClient, sizeof(CLobbyServerInfo),
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
void CClient::AllocateClientList()
{
	m_player = new CPlayer[CServerInfo::E_MAX_CLIENTS]();
}

/*
*/
void CClient::Connect(const char* address, const char* port)
{
	m_errorLog->WriteError(true, "CClient::Connect:%s %s\n", address, port);

	if (m_gameSocket->CreateConnectSocket(address, port))
	{
		m_gameIsRunning = true;

		m_gameThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient), &CClient::GameThread, (void*)this, 0, &m_gameThreadId);

		CloseHandle(m_gameThread);

		m_gameThread = 0;
	}
}

/*
*/
void CClient::ConnectLobby(const char* address, const char* port)
{
	m_errorLog->WriteError(true, "CClient::ConnectLobby:%s %s\n", address, port);

	if (m_lobbySocket->CreateConnectSocket(address, port))
	{
		m_lobbyIsRunning = true;

		m_lobbyThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient), &CClient::LobbyThread, (void*)this, 0, &m_lobbyThreadId);
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
void CClient::CompleteLobbyConnect()
{
	m_lobbyLocalClient->SetClient(m_lobbyServerInfo);
}

/*
*/
void CClient::Connect()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_CONNECT, 0);
}

/*
*/
void CClient::ConnectLobby()
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

	SAFE_DELETE_ARRAY(m_player);

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		serverInfo->Clear();
	}

	m_graphicsAdapter->WaitForGPU();
}

/*
*/
void CClient::DestroyEnvironment()
{
	if (m_map == nullptr)
	{
		return;
	}

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

	SAFE_DELETE(n);

	Sleep(500);

	m_gameIsRunning = false;

	m_gameSocket->Shutdown();

	m_state = CClient::ClientState::E_LOBBY;
}

/*
*/
void CClient::DisconnectLobby()
{
	m_errorLog->WriteError(true, "CClient::DisconnectLobby\n");

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::LobbyEvent::E_LE_DISCONNECT,
		(void*)m_lobbyLocalClient, sizeof(CLobbyServerInfo),
		nullptr, 0);

	if (m_lobbySocket->m_state == CSocket::State::E_CONNECTED)
	{
		m_lobbySocket->Send((char*)n, sizeof(CNetwork));
	}

	SAFE_DELETE(n);

	// wait so the network event can transmit
	Sleep(500);

	m_lobbyIsRunning = false;

	CloseHandle(m_lobbyThread);

	m_lobbyThread = 0;

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
			m_player[m_localClient->m_clientNumber].m_camera->UpdateRotation(
				((float)m_mouseDevice->m_depth.m_lY * m_mouseDevice->m_sensitivity),
				((float)m_mouseDevice->m_depth.m_lX * m_mouseDevice->m_sensitivity),
				((float)m_mouseDevice->m_depth.m_lZ * m_mouseDevice->m_sensitivity));

			CVec3f l = CVec3f(m_player[m_localClient->m_clientNumber].m_camera->m_look);

			m_player[m_localClient->m_clientNumber].m_model->m_rotation.y = l.PointToDegree().m_p.y;

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

				if (button->m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_LOCAL)
				{
					CClient::SendLocal(button->m_network);
				}

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

		button->m_image->m_materialConstantBuffer->RecordStatic();

		m_commandListGroup[0]->Add(button->m_image->m_materialConstantBuffer->m_commandList);

		button->Record();

		m_commandListGroup[button->m_image->m_listGroup]->Add(button->m_image->m_commandList);

		buttonNode = buttonNode->m_next;
	}

	char text[132] = {};

	CVec2f textPosition(10.0f, 10.0f);

	sprintf_s(text, 132, "FPS: %3i", m_frametime->m_rframecount);

	m_fpsText->SetText(text);

	m_fpsText->m_position.m_p.y = textPosition.m_p.y;

	m_fpsText->m_materialConstantBuffer->RecordStatic();

	m_commandListGroup[m_fpsText->m_listGroup]->Add(m_fpsText->m_materialConstantBuffer->m_commandList);

	m_fpsText->Record();

	m_commandListGroup[m_fpsText->m_listGroup]->Add(m_fpsText->m_commandList);

	textPosition.m_p.y += m_fpsText->m_size;

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if ((serverInfo->m_isRunning) || (serverInfo->m_isBot))
		{
			textPosition.m_p.y += m_scoreboardText[i]->m_size;

			m_scoreboardText[i]->m_position = textPosition;

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

			m_scoreboardText[i]->m_materialConstantBuffer->RecordStatic();

			m_commandListGroup[m_scoreboardText[i]->m_listGroup]->Add(m_scoreboardText[i]->m_materialConstantBuffer->m_commandList);

			m_scoreboardText[i]->Record();

			m_commandListGroup[m_scoreboardText[i]->m_listGroup]->Add(m_scoreboardText[i]->m_commandList);

			SAFE_DELETE_ARRAY(np);
			
			SAFE_DELETE(name);
		}
	}

	if (m_console->m_isVisible)
	{
		m_console->m_background->RecordConstantBuffer();

		m_commandListGroup[0]->Add(m_console->m_background->m_materialConstantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}
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

	loadingScreen->Record();

	m_commandListGroup[loadingScreen->m_listGroup]->Add(loadingScreen->m_commandList);

#ifndef D3D11on12
	char text[132] = {};

	sprintf_s(text, 132, "%s %s %s", mapName, name, item);

	m_loadingText->SetText(text);

	m_loadingText->m_materialConstantBuffer->RecordStatic();

	m_commandListGroup[0]->Add(m_loadingText->m_materialConstantBuffer->m_commandList);

	m_loadingText->Record();

	m_commandListGroup[1]->Add(m_loadingText->m_commandList);
#endif
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

		m_commandListGroup[0]->Add(m_console->m_background->m_materialConstantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}

	m_pointer->m_image->SetPosition(&m_mouseDevice->m_hud.m_position);

	m_pointer->m_image->RecordConstantBuffer();

	m_commandListGroup[0]->Add(m_pointer->m_image->m_materialConstantBuffer->m_commandList);

	m_pointer->m_image->Record();

	m_commandListGroup[m_pointer->m_image->m_listGroup]->Add(m_pointer->m_image->m_commandList);

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
		if (network->m_audience == CNetwork::E_CE_TO_LOCAL)
		{
			CClient::SendLocal(network);

			if (m_lobbySocket->m_state == CSocket::State::E_FAILED)
			{
				CListNode* message = m_login->m_glyphTexts->Search("message");

				if (message)
				{
					CGlyphText* glyphText = (CGlyphText*)message->m_object;

					glyphText->SetText("Failed to connect");
				}
			}
		}
	}

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i]->Clear();
	}

	m_graphicsAdapter->Record();

	m_commandListGroup[0]->Add(m_graphicsAdapter->m_commandList);

	m_login->Record();

	m_commandListGroup[0]->Add(m_login->m_background->m_materialConstantBuffer->m_commandList);

	m_commandListGroup[m_login->m_background->m_listGroup]->Add(m_login->m_background->m_commandList);

	m_node = m_login->m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CButton* button = (CButton*)m_node->m_object;

		m_commandListGroup[0]->Add(button->m_image->m_materialConstantBuffer->m_commandList);

		m_commandListGroup[button->m_image->m_listGroup]->Add(button->m_image->m_commandList);

		m_node = m_node->m_next;
	}

	m_node = m_login->m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CTextInput* textInput = (CTextInput*)m_node->m_object;

		m_commandListGroup[0]->Add(textInput->m_image->m_materialConstantBuffer->m_commandList);

		m_commandListGroup[textInput->m_image->m_listGroup]->Add(textInput->m_image->m_commandList);

		m_node = m_node->m_next;
	}

	m_node = m_login->m_glyphTexts->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CGlyphText* glyphText = (CGlyphText*)m_node->m_object;

		m_commandListGroup[0]->Add(glyphText->m_materialConstantBuffer->m_commandList);

		m_commandListGroup[glyphText->m_listGroup]->Add(glyphText->m_commandList);

		m_node = m_node->m_next;
	}

	if (m_console->m_isVisible)
	{
		m_console->m_background->RecordConstantBuffer();

		m_commandListGroup[0]->Add(m_console->m_background->m_materialConstantBuffer->m_commandList);

		m_console->Record();

		m_commandListGroup[m_console->m_background->m_listGroup]->Add(m_console->m_background->m_commandList);
	}

	m_pointer->m_image->SetPosition(&m_mouseDevice->m_hud.m_position);

	m_pointer->m_image->RecordConstantBuffer();

	m_commandListGroup[0]->Add(m_pointer->m_image->m_materialConstantBuffer->m_commandList);

	m_pointer->m_image->Record();

	m_commandListGroup[m_pointer->m_image->m_listGroup]->Add(m_pointer->m_image->m_commandList);

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

	m_currentCamera = m_player[m_localClient->m_clientNumber].m_camera;

	CVec3f position = CVec3f(
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

	m_shadowMap->m_constantBuffer->RecordStatic();

	m_shadowMap->m_constantBuffer->UploadStaticResources();


	m_shadowMap->PreRecord();

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if ((serverInfo->m_isRunning) || (serverInfo->m_isBot))
		{
			if (m_player[serverInfo->m_clientNumber].m_isInitialized)
			{
				m_player[serverInfo->m_clientNumber].m_model->DrawGeometry(m_shadowMap->m_commandList);
			}
		}
	}

	m_node = m_map->m_collectables->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CObject* object = (CObject*)m_node->m_object;

		if ((object->m_isInitialized) && (object->m_isVisible))
		{
			object->DrawGeometry(m_shadowMap->m_commandList);
		}

		m_node = m_node->m_next;
	}

	m_node = m_map->m_statics->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CObject* object = (CObject*)m_node->m_object;

		if ((object->m_isInitialized) && (object->m_isVisible))
		{
			object->DrawGeometry(m_shadowMap->m_commandList);
		}

		m_node = m_node->m_next;
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

	m_cameraConstantBuffer->LoadDynamicBuffer();

	m_overhead->SetCameraPosition(m_currentCamera->m_position);
	
	m_overhead->m_camera->UpdateView();

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

	m_overheadCameraConstantBuffer->LoadDynamicBuffer();


	m_map->RecordConstantBuffers();

	m_commandListGroup[m_map->m_listGroup]->Add(m_map->m_lightConstantBuffer->m_commandList);

	m_map->Record();
	
	m_map->Record(m_overhead);

	m_commandListGroup[m_map->m_listGroup]->Add(m_map->m_commandList);

	if ((m_map->m_terrain) && (m_map->m_terrain->m_isInitialized))
	{
		m_map->m_terrain->Record();
		
		m_map->m_terrain->Record(m_overhead);

		m_commandListGroup[m_map->m_terrain->m_listGroup]->Add(m_map->m_terrain->m_commandList);
	}

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		if ((serverInfo->m_isRunning) || (serverInfo->m_isBot))
		{
			if (m_player[serverInfo->m_clientNumber].m_isInitialized)
			{
				m_player[serverInfo->m_clientNumber].m_model->Record();
				
				m_player[serverInfo->m_clientNumber].m_model->Record(m_overhead);

				m_commandListGroup[m_player[serverInfo->m_clientNumber].m_model->m_listGroup]->Add(m_player[serverInfo->m_clientNumber].m_model->m_commandList);
			}
		}
	}

	m_node = m_map->m_collectables->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CObject* object = (CObject*)m_node->m_object;

		if ((object->m_isInitialized) && (object->m_isVisible))
		{
			object->Animation(m_frametime);
			
			object->Update();
			
			object->Record();

			object->Record(m_overhead);

			m_commandListGroup[object->m_listGroup]->Add(object->m_commandList);
		}

		m_node = m_node->m_next;
	}

	m_node = m_map->m_statics->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CObject* object = (CObject*)m_node->m_object;

		if ((object->m_isInitialized) && (object->m_isVisible))
		{
			object->Animation(m_frametime);
			
			object->Update();
			
			object->Record();

			object->Record(m_overhead);

			m_commandListGroup[object->m_listGroup]->Add(object->m_commandList);
		}

		m_node = m_node->m_next;
	}

	m_overhead->Record();

	m_commandListGroup[m_overhead->m_listGroup]->Add(m_overhead->m_commandList);

	CClient::DrawHud();

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

	m_node = m_buttonManager->m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		CButton* button = (CButton*)m_node->m_object;

		button->DisplayText();

		m_node = m_node->m_next;
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

	CClient::InitializePlayer(m_serverInfo);

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

	m_vs = m_shaderBinaryManager->Get("image.vs");
	m_ps = m_shaderBinaryManager->Get("image.ps");

	m_imagePipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 2, 1, CPipelineState::VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

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

	m_cameraConstantBuffer->CreateDynamicResource();

	m_overheadCameraConstantBuffer = new CConstantBuffer();

	m_overheadCameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 384);

	m_overheadCameraConstantBuffer->CreateDynamicResource();

	m_imageWvp = new CConstantBuffer();

	m_imageWvp->Constructor(m_graphicsAdapter, m_errorLog, 64);

	m_imageWvp->CreateStaticResource();

	m_imageWvp->Reset();

	m_imageWvp->m_values->Append(m_graphicsAdapter->m_wvpMat);

	m_imageWvp->RecordStatic();

	m_imageWvp->UploadStaticResources();

	m_imageWvp->ReleaseStaticCPUResource();

	CTexture* consoleImage = m_textureManagerHud->Create("textures/misc/console.jpg", 1);

	m_console = new CConsole(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, 5, consoleImage, m_consolas12, 1024, 400);

	CTexture* glyphs = m_textureManagerHud->Create("textures/misc/consolas.tga", 1);

	m_glyphFixed = new CGlyphFixed(glyphs, 18, 18);

	glyphs = m_textureManagerHud->Create("textures/misc/calibri.tga", 1);

	m_glyphVariable = new CGlyphVariable(glyphs, 16);

	CTexture* cursorImage = m_textureManagerHud->Create("textures/misc/cursor.tga", 1);

	m_pointer = new CPointer(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, 6, cursorImage,
		CVec2f((float)m_graphicsAdapter->m_middle.x, (float)m_graphicsAdapter->m_middle.y), CVec2f(16.0f, 32.0f));

	m_fpsText = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, m_glyphFixed, 4, 8, CVec2f(10.0f, 10.0f), 14);

	m_fpsText->SetColor(m_graphicsAdapter->m_rgbColor->SunYellow->GetFloatArray());

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		m_scoreboardText[i] = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, m_glyphFixed, 4, 132, CVec2f(10.0f, 10.0f), 18);

		m_scoreboardText[i]->SetColor(m_graphicsAdapter->m_rgbColor->SunYellow->GetFloatArray());
	}
}

/*
*/
void CClient::InitializeLoginPanel()
{
	CTexture* panelImage = m_textureManagerHud->Create("textures/brick/roughbrick01.tga", 1);

	m_login = new CPanel(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imageWvp, m_imagePipelineState, 1, m_pointer->m_image, panelImage, m_fontArial, "Login",
		CVec2f((float)(m_graphicsAdapter->m_width / 2) - 138, (float)(m_graphicsAdapter->m_height / 2) - 300), CVec2f(276, 400));

	CTexture* textInputImage = m_textureManagerHud->Create("textures/misc/button.tga", 1);

	CTextInput* m_userid = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imageWvp, m_imagePipelineState, 1, m_calibri12, textInputImage,
		CVec2f(10.0f, 56.0f), CVec2f(256.0f, 14.0f));

	m_login->AddTextInput(m_userid, "m_userid");

	CTextInput* m_password = new CTextInput(m_graphicsAdapter, m_mouseDevice, m_errorLog, m_imageWvp, m_imagePipelineState, 1, m_calibri12, textInputImage,
		CVec2f(10.0f, 73.0f), CVec2f(256.0f, 14.0f));

	m_login->AddTextInput(m_password, "m_password");

	CWavLoader* sound = m_soundManagerHud->Create("audio/stereo/logon.wav");

	CTexture* buttonImage = m_textureManagerHud->Create("textures/misc/button.tga", 1);

	CButton* buttonLogon = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_imageWvp, m_imagePipelineState, 1, "Login", m_calibri12,
		sound, sound, sound, buttonImage, CVec2f(106.0f, 112.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CONNECT_LOBBY);

	m_login->AddButton(buttonLogon, "buttonLogon");

	CButton* buttonExit = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_imageWvp, m_imagePipelineState, 1, "Exit", m_calibri12,
		sound, sound, sound, buttonImage, CVec2f(106.0f, 147.0f), CVec2f(64.0f, 32.0f), CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_EXIT_GAME);

	m_login->AddButton(buttonExit, "buttonExit");

	CGlyphText* message = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, m_glyphFixed, 4, 64, CVec2f(66.0f, 202.0f), 16);

	message->SetText("Enter Credentials");
	message->SetColor(m_graphicsAdapter->m_rgbColor->SunYellow->GetFloatArray());

	m_login->AddText(message, "message");
}

/*
*/
void CClient::InitializeManagers()
{
	m_fontManager = new CFontManager(m_graphicsAdapter, m_errorLog, "Arial 12", DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL);

	m_shaderBinaryManager = new CShaderBinaryManager(m_graphicsAdapter, m_errorLog, m_local);

	m_soundManager = new CSoundManager(m_errorLog, m_local, "audio/mono/ding.wav");

	m_soundManagerHud = new CSoundManager(m_errorLog, m_local, "audio/mono/ding.wav");

	m_textureManager = new CTextureManager(m_graphicsAdapter, m_errorLog, m_local, "textures/misc/black.tga");

	m_textureManagerHud = new CTextureManager(m_graphicsAdapter, m_errorLog, m_local, "textures/misc/black.tga");

	m_wavefrontManager = new CWavefrontManager(m_errorLog, m_local, "model/sphere.obj");

	m_buttonManager = new CButtonManager(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_imageWvp, m_fontManager, m_imagePipelineState);

	for (int32_t i = 0; i < CCommandListGroup::E_MAX_LIST_GROUP; i++)
	{
		m_commandListGroup[i] = new CCommandListGroup(m_graphicsAdapter, m_errorLog);
	}
}

/*
*/
void CClient::InitializeNetworking()
{
	CClient::InitializeWinsock();

	m_gameNetwork = new CNetwork();

	m_lobbyNetwork = new CNetwork();

	m_serverInfos = new CHeapArray(true, sizeof(CServerInfo), 1, CServerInfo::E_MAX_CLIENTS);

	for (int32_t i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfos->GetElement(1, i);

		serverInfo->Constructor();
	}

	m_lobbyServerInfo = new CLobbyServerInfo();

	m_lobbyLocalClient = new CLobbyServerInfo();

	m_gameSocket = new CSocket(m_errorLog);

	m_lobbySocket = new CSocket(m_errorLog);
}

/*
*/
void CClient::InitializePlayer(CServerInfo* serverInfo)
{
	m_player[serverInfo->m_clientNumber].Deconstructor();

	m_player[serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_soundManager, m_errorLog, m_local, m_wavefrontManager, m_textureManager,
		m_clientPipelineState, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, true, 2, m_shadowMap);

	m_player[serverInfo->m_clientNumber].SetModel(serverInfo->m_modelName);

	m_player[serverInfo->m_clientNumber].SetPosition(&serverInfo->m_position);

	m_player[serverInfo->m_clientNumber].SetTeam(serverInfo->m_team);

	m_player[serverInfo->m_clientNumber].m_model->RecordConstantBuffers();

	m_player[serverInfo->m_clientNumber].m_model->UploadConstantBuffers();

	m_player[serverInfo->m_clientNumber].m_lightConstantBuffer = m_map->m_lightConstantBuffer;

	if (serverInfo->m_clientNumber == m_localClient->m_clientNumber)
	{
		if (serverInfo->m_isReconnect)
		{
			m_player[serverInfo->m_clientNumber].m_camera->UpdateRotation(0.0f, serverInfo->m_lastDirection.PointToDegree().m_p.y, 0.0f);

			m_player[serverInfo->m_clientNumber].m_camera->UpdateView();
		}
		else
		{
			m_player[serverInfo->m_clientNumber].m_camera->UpdateRotation(serverInfo->m_direction.m_p.x, serverInfo->m_direction.m_p.y, serverInfo->m_direction.m_p.z);

			m_player[serverInfo->m_clientNumber].m_camera->UpdateView();
		}
	}

	m_player[serverInfo->m_clientNumber].m_isInitialized = true;
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
void CClient::Loading()
{
	m_state = CClient::ClientState::E_LOADING;
}

/*
*/
void CClient::LoadEnvironment()
{
	m_errorLog->WriteError(true, "CClient::LoadEnvironment:%s\n", (char*)m_gameNetwork->m_data);

	CClient::DestroyEnvironment();

	CServerInfo* serverInfo = (CServerInfo*)m_gameNetwork->m_serverInfo;

	m_localClient = (CServerInfo*)m_serverInfos->GetElement(1, serverInfo->m_clientNumber);

	m_localClient->SetClient(serverInfo);

	m_clientNumber = m_localClient->m_clientNumber;

	m_mapName = new CString((const char*)m_gameNetwork->m_data);

	m_loadingText = new CGlyphText(m_graphicsAdapter, m_errorLog, m_imageWvp, m_imagePipelineState, m_glyphFixed, 4, 132, CVec2f(10.0f, 620.0f), 28);

	m_loadingText->SetColor(m_graphicsAdapter->m_rgbColor->SunYellow->GetFloatArray());

	m_shadowMap = new CShadowMap(m_graphicsAdapter, m_errorLog, m_local, m_shaderBinaryManager);

	m_overhead = new COverhead(m_graphicsAdapter, m_errorLog, m_local, m_imageWvp, m_imagePipelineState, 3, 0, CVec2f(10.0f, 10.0f), CVec2f(256.0f, 256.0f));

	CString* pathMapFile = new CString(m_local->m_installPath->m_text);

	pathMapFile->Append("main/maps/");
	pathMapFile->Append(m_mapName->m_text);
	pathMapFile->Append(".dat");

	m_map = new CMap(m_graphicsAdapter, m_errorLog, m_local, m_soundDevice, m_wavefrontManager, m_mapPipelineState, m_collectablePipelineState,
		m_terrainPipelineState, m_soundManager, 1, m_shadowMap, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_mapName->m_text, pathMapFile->m_text);

	SAFE_DELETE(pathMapFile);

	m_isLoading = true;

	m_loadScreenThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient), &CClient::LoadingScreenThread, (void*)this, 0, &m_loadScreenThreadId);

	CloseHandle(m_loadScreenThread);

	m_loadScreenThread = 0;

	Sleep(5);

	m_loadThread = (HANDLE)_beginthreadex(NULL, sizeof(CClient), &CClient::LoadEnvironmentThread, (void*)this, 0, &m_loadThreadId);

	CloseHandle(m_loadThread);

	m_loadThread = 0;
}

/*
*/
void CClient::LoadPlayer()
{
	m_errorLog->WriteError(true, "CClient::LoadPlayer:%s:%s\n", m_serverInfo->m_playerName, m_gameNetwork->m_data);

	m_player[m_serverInfo->m_clientNumber].Deconstructor();

	m_player[m_serverInfo->m_clientNumber].Constructor(m_graphicsAdapter, m_soundDevice, m_soundManager, m_errorLog, m_local, m_wavefrontManager, m_textureManager,
		m_clientPipelineState, m_cameraConstantBuffer, m_overheadCameraConstantBuffer, true, 2, m_shadowMap);

	m_player[m_serverInfo->m_clientNumber].SetModel(m_serverInfo->m_modelName);

	m_player[m_serverInfo->m_clientNumber].SetTeam(m_serverInfo->m_team);

	m_player[m_serverInfo->m_clientNumber].SetPosition(&m_serverInfo->m_position);

	m_player[m_serverInfo->m_clientNumber].m_model->RecordConstantBuffers();

	m_player[m_serverInfo->m_clientNumber].m_model->UploadConstantBuffers();

	m_player[m_serverInfo->m_clientNumber].m_camera->UpdateRotation(m_serverInfo->m_direction.m_p.x, m_serverInfo->m_direction.m_p.y, m_serverInfo->m_direction.m_p.z);

	m_player[m_serverInfo->m_clientNumber].m_camera->UpdateView();

	m_player[m_serverInfo->m_clientNumber].m_isInitialized = true;
}

/*
*/
void CClient::LobbyServerFull()
{
	m_errorLog->WriteError(true, "CClient::LobbyServerFull\n");
}

/*
*/
void CClient::OpenBrowser()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_BROWSER, 0);
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
void CClient::ProcessLobbyEvent()
{
	CLobbyServerInfo* serverInfo = (CLobbyServerInfo*)m_lobbyNetwork->m_serverInfo;

	m_lobbyServerInfo->Initialize(serverInfo);

	(this->*m_event[m_lobbyNetwork->m_type])();

	m_lobbyNetwork->SetData(nullptr, 0);
}

/*
*/
void CClient::QueSound()
{
	m_errorLog->WriteError(true, "CClient::QueSound:%s\n", m_gameNetwork->m_data);

	m_index = atoi((char*)m_gameNetwork->m_data);

	if (m_player[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		m_sound3D = m_player[m_serverInfo->m_clientNumber].m_model->m_sounds[m_index];

		CClient::QueSound(m_sound3D);
	}
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

	m_position = X3DAUDIO_VECTOR(
		m_player[m_localClient->m_clientNumber].m_camera->m_position.x,
		m_player[m_localClient->m_clientNumber].m_camera->m_position.y,
		m_player[m_localClient->m_clientNumber].m_camera->m_position.z
	);

	m_look = X3DAUDIO_VECTOR(
		m_player[m_localClient->m_clientNumber].m_camera->m_look.x,
		m_player[m_localClient->m_clientNumber].m_camera->m_look.y,
		m_player[m_localClient->m_clientNumber].m_camera->m_look.z
	);

	m_up = X3DAUDIO_VECTOR(
		m_player[m_localClient->m_clientNumber].m_camera->m_up.x,
		m_player[m_localClient->m_clientNumber].m_camera->m_up.y,
		m_player[m_localClient->m_clientNumber].m_camera->m_up.z
	);

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
void CClient::RequestAccountInfo()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_ACCOUNT_INFO, 0);
}

/*
*/
void CClient::Send(CNetwork* network)
{
	if (m_gameSocket->m_state == CSocket::State::E_CONNECTED)
	{
		m_gameSocket->Send((char*)network, sizeof(CNetwork));
	}
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

	if (m_player[m_localClient->m_clientNumber].m_camera->m_isInitialized)
	{
		m_localClient->SetDirection(&m_player[m_localClient->m_clientNumber].m_camera->m_look);
		
		m_localClient->SetRight(&m_player[m_localClient->m_clientNumber].m_camera->m_right);
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

				if (button->m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
				{
					events[eventCount] = button->m_network->m_type;

					eventCount++;
				}

				break;
			}
		}

		buttonNode = buttonNode->m_next;
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
		CString* message = new CString(m_lobbyLocalClient->m_name);

		message->Append(":");
		message->Append(m_data);

		CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::LobbyEvent::E_LE_MESSAGE,
			(void*)m_lobbyLocalClient, sizeof(CLobbyServerInfo),
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
	if (m_player[m_localClient->m_clientNumber].m_camera->m_isInitialized)
	{
		m_localClient->SetDirection(&m_player[m_localClient->m_clientNumber].m_camera->m_look);
		
		m_localClient->SetRight(&m_player[m_localClient->m_clientNumber].m_camera->m_right);
	}

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

	m_gameSocket->Shutdown();

	m_gameIsRunning = false;

	CloseHandle(m_gameThread);

	m_gameThread = 0;
}

/*
*/
void CClient::ServerShutdown()
{
	m_errorLog->WriteError(true, "CClient::ServerShutdown\n");

	m_gameIsRunning = false;

	CloseHandle(m_gameThread);

	m_gameThread = 0;

	m_gameSocket->Shutdown();

	CClient::DestroyEnvironment();
	
	CClient::DestroyClientList();

	m_state = CClient::ClientState::E_LOBBY;
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
	if (!m_player[m_serverInfo->m_clientNumber].m_isInitialized)
	{
		return;
	}

	m_player[m_serverInfo->m_clientNumber].SetPosition(&m_serverInfo->m_position);

	m_player[m_serverInfo->m_clientNumber].SetDirection(&m_serverInfo->m_direction);
}

/*
*/
void CClient::UpdateCollectable()
{
	m_errorLog->WriteError(true, "CClient::UpdateCollectable:%s\n", m_gameNetwork->m_data);

	char itemName[CKeyValue::MAX_VALUE] = {};

	char isVisible = {};

	sscanf_s((char*)m_gameNetwork->m_data, "%s %c", itemName, CKeyValue::MAX_VALUE, &isVisible, 1);

	m_node = m_map->m_collectables->Search(itemName);

	if (m_node)
	{
		m_object = (CObject*)m_node->m_object;

		if (isVisible == 'f')
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

/*
*/
void CClient::WindowMode()
{
	SendMessage(m_graphicsAdapter->m_hWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
}

/*
*/
unsigned __stdcall CClient::GameThread(void* obj)
{
	CClient* client = (CClient*)obj;

	while (client->m_gameIsRunning)
	{
		int32_t bytes = client->m_gameSocket->Receive((char*)client->m_gameNetwork, sizeof(CNetwork));

		if (bytes > 0)
		{
			client->ProcessEvent();
		}
		else
		{
			client->m_gameIsRunning = false;

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

	client->m_map->Load();

	client->m_isLoading = false;

	_endthreadex(0);

	return 0;
}

/*
*/
unsigned __stdcall CClient::LoadingScreenThread(void* obj)
{
	CClient* client = (CClient*)obj;

	CString* pathMapImage = new CString(client->m_local->m_installPath->m_text);

	pathMapImage->Append("main/maps/");
	pathMapImage->Append(client->m_mapName->m_text);
	pathMapImage->Append(".tga");

	CTexture* texture = new CTexture(client->m_graphicsAdapter, client->m_errorLog, client->m_textureManager->m_computeShader, client->m_mapName->m_text);

	texture->CreateFromFilename(pathMapImage->m_text, 0);

	SAFE_DELETE(pathMapImage);

	CImage* loadingScreen = new CImage(client->m_graphicsAdapter, client->m_errorLog, client->m_imageWvp, client->m_imagePipelineState, 1, texture, CVec2f(10.0f, 10.0f), CVec2f(600.0f, 600.0f));

	loadingScreen->m_shaderMaterial->SetKa(1.0f, 1.0f, 1.0f, 1.0f);

	loadingScreen->RecordConstantBuffer();

	loadingScreen->m_materialConstantBuffer->m_commandList->Close();

	client->m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)loadingScreen->m_materialConstantBuffer->m_commandList.GetAddressOf());

	client->DrawLoadingScreen(loadingScreen, client->m_mapName->m_text, (char*)"Environment", (char*)"Loading ...");

	while (client->m_isLoading)
	{
		client->DrawLoadingScreen(loadingScreen, client->m_mapName->m_text, (char*)"Loading...", (char*)client->m_map->m_loadMessage->m_text);

		Sleep(100);
	}

	client->m_player[client->m_localClient->m_clientNumber].m_lightConstantBuffer = client->m_map->m_lightConstantBuffer;
	
	client->m_player[client->m_localClient->m_clientNumber].m_shadowMap = client->m_shadowMap;

	client->DrawLoadingScreen(loadingScreen, client->m_mapName->m_text, (char*)"Load Environment Complete", (char*)"Waiting on the server ...");

	client->m_graphicsAdapter->WaitForGPU();

	SAFE_DELETE(texture);
	SAFE_DELETE(loadingScreen);
	SAFE_DELETE(client->m_loadingText);
	SAFE_DELETE(client->m_mapName);

	client->m_errorLog->WriteError(true, "CClient::LoadEnvironment Complete\n");

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_READY,
		(void*)client->m_localClient, sizeof(CServerInfo),
		nullptr, 0);

	client->Send(n);

	SAFE_DELETE(n);

	_endthreadex(0);

	return 0;
}

/*
*/
unsigned __stdcall CClient::LobbyThread(void* obj)
{
	CClient* client = (CClient*)obj;

	while (client->m_lobbyIsRunning)
	{
		int32_t bytes = client->m_lobbySocket->Receive((char*)client->m_lobbyNetwork, sizeof(CNetwork));

		if (bytes > 0)
		{
			client->ProcessLobbyEvent();
		}
		else
		{
			client->m_lobbyIsRunning = false;

			client->m_state = CClient::ClientState::E_LOGIN;
		}
	}

	_endthreadex(0);

	return 0;
}