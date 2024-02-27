#include "CClient.h"

// Network thread for client receive events
unsigned __stdcall CClient_ReceiveThread(void* obj);

// Event functions
void CClient_Accepted(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_AccountInfo(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_Attack(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_CameraMove(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_Chat(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_ChatBox(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_Enter(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_Exit(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_ExitGame(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_InfoRequest(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_LoadEnvironment(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_SendActivity(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_ServerFull(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_Update(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_UpdateCollectable(CClient* client, CNetwork* network, CServerInfo* serverInfo);
void CClient_WindowMode(CClient* client, CNetwork* network, CServerInfo* serverInfo);

/*
*/
CClient::CClient()
{
	memset(this, 0x00, sizeof(CClient));
}

/*
*/
CClient::CClient(CVideoDevice* videoDevice, CMouseDevice* mouseDevice, CKeyboardDevice* keyboardDevice, CErrorLog* errorLog, CTextureManager* textureManager, CShaderManager* shaderManager, CModelManager* modelManager, CWavefrontManager* wavefrontManager, CSoundManager* soundManager)
{
	memset(this, 0x00, sizeof(CClient));

	m_videoDevice = videoDevice;

	m_mouseDevice = mouseDevice;

	m_keyboardDevice = keyboardDevice;

	m_errorLog = errorLog;

	m_textureManager = textureManager;

	m_shaderManager = shaderManager;

	m_modelManager = modelManager;

	m_wavefrontManager = wavefrontManager;

	m_soundManager = soundManager;

	m_network = new CNetwork();

	m_serverInfo = new CHeapArray(sizeof(CServerInfo), true, true, 1, CServerInfo::E_MAX_CLIENTS);

#ifdef _DEBUG
	constexpr auto serverInfoSize = sizeof(CServerInfo);

	m_errorLog->WriteError(true, "serverInfo Size:%u\n", serverInfoSize);
#endif

	pFunc[CNetwork::ClientEvent::E_CE_ACCEPTED] = &CClient_Accepted;
	pFunc[CNetwork::ClientEvent::E_CE_ACCOUNT_INFO] = &CClient_AccountInfo;
	pFunc[CNetwork::ClientEvent::E_CE_ATTACK] = &CClient_Attack;
	pFunc[CNetwork::ClientEvent::E_CE_CAMERA_MOVE] = &CClient_CameraMove;
	pFunc[CNetwork::ClientEvent::E_CE_CHAT] = &CClient_Chat;
	pFunc[CNetwork::ClientEvent::E_CE_CHATBOX] = &CClient_ChatBox;
	pFunc[CNetwork::ClientEvent::E_CE_ENTER] = &CClient_Enter;
	pFunc[CNetwork::ClientEvent::E_CE_EXIT] = &CClient_Exit;
	pFunc[CNetwork::ClientEvent::E_CE_EXIT_GAME] = &CClient_ExitGame;
	pFunc[CNetwork::ClientEvent::E_CE_INFO] = &CClient_InfoRequest;
	pFunc[CNetwork::ClientEvent::E_CE_LOAD_ENVIRONMENT] = &CClient_LoadEnvironment;
	pFunc[CNetwork::ClientEvent::E_CE_SEND_ACTIVITY] = &CClient_SendActivity;
	pFunc[CNetwork::ClientEvent::E_CE_SERVER_FULL] = &CClient_ServerFull;
	pFunc[CNetwork::ClientEvent::E_CE_UPDATE] = &CClient_Update;
	pFunc[CNetwork::ClientEvent::E_CE_UPDATE_COLLECTABLE] = &CClient_UpdateCollectable;
	pFunc[CNetwork::ClientEvent::E_CE_WINDOW_MODE] = &CClient_WindowMode;
}

/*
*/
CClient::~CClient()
{
	CClient::Disconnect();

	CClient::DestroyEnvironment();

	delete m_serverInfo;
	delete m_network;
}

/*
*/
void CClient::Connect(const char* address)
{
	sscanf_s(address, "%s %s", m_ip, 15, m_port, 6);

	CClient::InitializeWinsock();
	CClient::CreateSocket(m_ip, m_port);

	m_hThread = (HANDLE)_beginthreadex(0, sizeof(CClient),
		&CClient_ReceiveThread,
		(void*)this,
		0,
		&m_receiveThreadId);
}

/*
*/
void CClient::Connect(const char* address, const char* port)
{
	CClient::InitializeWinsock();
	CClient::CreateSocket(address, port);

	m_hThread = (HANDLE)_beginthreadex(0, sizeof(CClient),
		&CClient_ReceiveThread,
		(void*)this,
		0,
		&m_receiveThreadId);
}

/*
*/
bool CClient::ConnectSocket(addrinfo* ptr)
{
	size_t err = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);

	if (err == SOCKET_ERROR)
	{
		m_errorLog->WriteWinsockErrorMessage(true, "CClient::ConnectSocket::connect:");

		return false;
	}

	return true;
}

/*
*/
void CClient::CreateSocket(const char* address, const char* port)
{
	if (address != nullptr)
	{
		memcpy_s((void*)m_ip, 15, (void*)address, strlen(address));
		memcpy_s((void*)m_port, 6, (void*)port, strlen(port));
	}
	else
	{
		memcpy_s((void*)m_ip, 15, (void*)"127.0.0.1", 9);
		memcpy_s((void*)m_port, 6, (void*)"26105", 5);
	}

	struct addrinfo* result = {};
	struct addrinfo* ptr = {};
	struct addrinfo  hints = {};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO::IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	size_t err = getaddrinfo(m_ip, m_port, &hints, &result);

	if (err != 0)
	{
		m_errorLog->WriteError(true, "CClient::CreateSocket::getaddrinfo:%i\n", err);

		return;
	}

	ptr = result;

	while (ptr != NULL)
	{
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (m_socket == INVALID_SOCKET)
		{
			m_errorLog->WriteWinsockErrorMessage(true, "CClient::CreateSocket::socket:");

			return;
		}

		if (CClient::ConnectSocket(ptr))
		{
			m_connectionState = CClient::ClientState::E_CONNECTING;

			break;
		}

		ptr = ptr->ai_next;
	}

	freeaddrinfo(result);

	gethostname(m_hostname, CClient::E_HOSTNAME_LENGTH);

	m_errorLog->WriteError(true, "CClient::CreateSocket::hostname:%s\n", m_hostname);
}

/*
*/
void CClient::Disconnect()
{
	if (m_connectionState != CClient::ClientState::E_CONNECTED)
	{
		return;
	}

	m_connectionState = CClient::ClientState::E_NOTCONNECTED;

	m_isRunning = false;

	int err = shutdown(m_socket, SD_BOTH);

	if (err == SOCKET_ERROR)
	{
		m_errorLog->WriteWinsockErrorMessage(true, "CClient::Disconnect::shutdown:");
	}

	closesocket(m_socket);

	m_socket = 0;

	WaitForSingleObject(m_hThread, 500);

	CloseHandle(m_hThread);

	m_hThread = 0;

	WSACleanup();
}

/*
*/
void CClient::DestroyEnvironment()
{
	m_videoDevice->WaitForGPU();

	if (m_terrain)
	{
		delete m_terrain;

		m_terrain = nullptr;
	}

	if (m_overhead)
	{
		delete m_overhead;

		m_overhead = nullptr;
	}

	if (m_visibility)
	{
		delete m_visibility;

		m_visibility = nullptr;
	}

	for (int i = 0; i < CServerInfo::E_MAX_CLIENTS; i++)
	{
		CServerInfo* serverInfo = (CServerInfo*)m_serverInfo->GetElement(1, i);

		serverInfo->m_isRunning = false;

		delete m_model[i];
		
		m_model[i] = nullptr;

		delete m_camera[i];
		
		m_camera[i] = nullptr;
	}
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
void CClient::ProcessEvent(CNetwork* network)
{
	CServerInfo* serverInfo = (CServerInfo*)network->m_serverInfo;

	CServerInfo* si = (CServerInfo*)m_serverInfo->GetElement(1, serverInfo->m_clientNumber);


	si->Constructor(m_errorLog);

	si->Initialize(serverInfo);


	if ((m_localClient != nullptr) && (serverInfo->m_clientNumber == m_localClient->m_clientNumber))
	{
		m_localClient = si;
	}

	if (network->m_type == 255)
	{
		return;
	}


	pFunc[network->m_type](this, network, si);
}

/*
*/
void CClient::Receive()
{
	DWORD millis = 500;
	int iResult = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&millis, (int)sizeof(DWORD));

	int totalBytes = recv(m_socket, (char*)m_network, sizeof(CNetwork), 0);

	if (totalBytes == 0)
	{
		m_errorLog->WriteError(true, "CClient::Receive::Server Closing Connection\n");

		m_connectionState = CClient::ClientState::E_NOTCONNECTED;

		m_isRunning = false;

		return;
	}
	else if (WSAGetLastError() == WSAETIMEDOUT)
	{
		m_network->m_type = 0xFF;
	}
	else if (totalBytes == SOCKET_ERROR)
	{
		m_errorLog->WriteWinsockErrorMessage(true, "CClient::Receive::recv:SOCKET_ERROR:");

		m_connectionState = CClient::ClientState::E_NOTCONNECTED;

		m_isRunning = false;
	}
}

/*
*/
void CClient::Send(CNetwork* network)
{
	if (network->m_audience == CNetwork::ClientEvent::E_CE_TO_LOCAL)
	{
		pFunc[network->m_type](this, network, nullptr);

		return;
	}

	if (m_connectionState != CClient::ClientState::E_CONNECTED)
	{
		return;
	}

	int totalBytes = send(m_socket, (char*)network, sizeof(CNetwork), 0);

	if (totalBytes == 0)
	{
		m_errorLog->WriteError(true, "CClient::Send::send:Server Closing Connection\n");

		m_connectionState = CClient::ClientState::E_NOTCONNECTED;
	}
	else if (totalBytes == SOCKET_ERROR)
	{
		m_errorLog->WriteWinsockErrorMessage(true, "CClient::Send::send:SOCKET_ERROR:");

		m_connectionState = CClient::ClientState::E_NOTCONNECTED;
	}
}

/*
*/
void CClient::SetLogin(const char* name)
{
	memset(m_name, 0x00, CServerInfo::E_NAME_SIZE);

	memcpy((void*)m_name, (void*)name, strlen(name));
}

/*
*/
void CClient_Accepted(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	client->m_localClient = serverInfo;

	client->m_connectionState = CClient::ClientState::E_CONNECTED;


	CString* message = new CString("Connection accepted");

	// Chat message
	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CHAT,
		(void*)message->GetText(), message->GetLength(),
		(void*)serverInfo);

	client->Send(n);

	delete n;

	delete message;


	serverInfo->SetName(client->m_name);

	// This send will be for the recv in CServer::CreateClient where the name is updated
	n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, NULL,
		client->m_name, (int)strlen(client->m_name),
		(void*)serverInfo);

	client->Send(n);

	delete n;
}

/*
*/
void CClient_AccountInfo(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	CString* message = new CString((char*)network->m_data);

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CHAT,
		(void*)message->GetText(), message->GetLength(),
		(void*)serverInfo);

	client->ProcessEvent(n);

	delete n;

	delete message;
}

/*
*/
void CClient_Attack(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	client->m_errorLog->WriteError(true, "CClient_Attack:%s\n", serverInfo->m_name);

	CSound* s = client->m_soundManager->Get("audio\\rifle.wav");

	if (s)
	{
		s->StartSound();
	}
}

/*
*/
void CClient_CameraMove(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_CAMERA, 0);
}

/*
*/
void CClient_Chat(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_CHATMESSAGE, (LPARAM)network->m_data);
}

/*
*/
void CClient_ChatBox(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_CHATBOX, 0);
}

/*
*/
void CClient_Enter(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	while (client->m_isActiveRender)
	{

	}

	client->m_isActiveUpdate = true;

	client->m_errorLog->WriteError(true, "CClient_Enter:%s\n", serverInfo->m_name);


	client->m_camera[serverInfo->m_clientNumber] = new CCamera((float)client->m_videoDevice->m_width,
		(float)client->m_videoDevice->m_height,
		&serverInfo->m_position,
		45.0f,
		1.0f, 50000.0f);

	client->m_camera[serverInfo->m_clientNumber]->UpdateRotation(
		serverInfo->m_direction.p.x,
		serverInfo->m_direction.p.y,
		serverInfo->m_direction.p.z);

	client->m_camera[serverInfo->m_clientNumber]->UpdateView();

	serverInfo->SetDirection(&client->m_camera[serverInfo->m_clientNumber]->m_look);


	client->m_model[serverInfo->m_clientNumber] = new CObject(client->m_videoDevice, client->m_errorLog, client->m_wavefrontManager, client->m_textureManager, client->m_shaderManager,
		"model\\cube.wft", "model\\cube.mtl",
		"vertexClient.hlsl", "VSMain", "vs_5_1",
		"pixelClient.hlsl", "PSMain", "ps_5_1",
		false, true, true);


	switch (serverInfo->m_team)
	{
	case CServerInfo::Team::E_TEAM_RED:
	{
		client->m_model[serverInfo->m_clientNumber]->m_floats[0]->m_values[0] = 0.0f;
		client->m_model[serverInfo->m_clientNumber]->m_ints[0]->m_values[0] = 0;
		client->m_model[serverInfo->m_clientNumber]->m_overheadFloats[0]->m_values[0] = 0.0f;
		client->m_model[serverInfo->m_clientNumber]->m_overheadInts[0]->m_values[0] = 0;

		break;
	}
	case CServerInfo::Team::E_TEAM_BLUE:
	{
		client->m_model[serverInfo->m_clientNumber]->m_floats[0]->m_values[0] = 1.0f;
		client->m_model[serverInfo->m_clientNumber]->m_ints[0]->m_values[0] = 1;
		client->m_model[serverInfo->m_clientNumber]->m_overheadFloats[0]->m_values[0] = 1.0f;
		client->m_model[serverInfo->m_clientNumber]->m_overheadInts[0]->m_values[0] = 1;

		break;
	}
	}


	// Must wait on the GPU for any texture uploads to complete
	client->m_videoDevice->WaitForGPU();



	// At this point the client can become active on the server game loop
	if (serverInfo->m_clientNumber == client->m_localClient->m_clientNumber)
	{
		// Request other clients info currently on the server
		CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_INFO,
			nullptr, 0,
			(void*)serverInfo);

		client->Send(n);


		serverInfo->m_state = CServerInfo::E_GAME;

		CString* message = new CString(client->m_localClient->m_name);

		message->Concat(" entered");

		serverInfo->SetChat(message->GetText());

		delete message;

		// Idle event to update this clients info on the server
		n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_IDLE,
			nullptr, 0,
			(void*)serverInfo);

		client->Send(n);

		delete n;
	}

	client->m_model[serverInfo->m_clientNumber]->m_isRecordable = true;

	client->m_isActiveUpdate = false;
}

/*
*/
void CClient_Exit(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{

}

/*
*/
void CClient_ExitGame(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_EXIT, 0);
}

/*
*/
void CClient_InfoRequest(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	while (client->m_isActiveRender)
	{
	}

	client->m_isActiveUpdate = true;

	client->m_camera[serverInfo->m_clientNumber] = new CCamera((float)client->m_videoDevice->m_width,
		(float)client->m_videoDevice->m_height,
		&serverInfo->m_position,
		45.0f,
		1.0f, 50000.0f);

	
	client->m_model[serverInfo->m_clientNumber] = new CObject(client->m_videoDevice, client->m_errorLog, client->m_wavefrontManager, client->m_textureManager, client->m_shaderManager,
		"model\\cube.wft", "model\\cube.mtl",
		"vertexClient.hlsl", "VSMain", "vs_5_1",
		"pixelClient.hlsl", "PSMain", "ps_5_1",
		false, true, true);


	switch (serverInfo->m_team)
	{
	case CServerInfo::Team::E_TEAM_RED:
	{
		client->m_model[serverInfo->m_clientNumber]->m_floats[0]->m_values[0] = 0.0f;
		client->m_model[serverInfo->m_clientNumber]->m_ints[0]->m_values[0] = 0;
		client->m_model[serverInfo->m_clientNumber]->m_overheadFloats[0]->m_values[0] = 0.0f;
		client->m_model[serverInfo->m_clientNumber]->m_overheadInts[0]->m_values[0] = 0;

		break;
	}
	case CServerInfo::Team::E_TEAM_BLUE:
	{
		client->m_model[serverInfo->m_clientNumber]->m_floats[0]->m_values[0] = 1.0f;
		client->m_model[serverInfo->m_clientNumber]->m_ints[0]->m_values[0] = 1;
		client->m_model[serverInfo->m_clientNumber]->m_overheadFloats[0]->m_values[0] = 1.0f;
		client->m_model[serverInfo->m_clientNumber]->m_overheadInts[0]->m_values[0] = 1;

		break;
	}
	}


	// Must wait on the GPU for any texture uploads to complete
	client->m_videoDevice->WaitForGPU();


	client->m_model[serverInfo->m_clientNumber]->m_isRecordable = true;

	client->m_isActiveUpdate = false;
}

/*
*/
void CClient_LoadEnvironment(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	client->m_errorLog->WriteError(true, "CClient_LoadEnvironment:%s\n", (char*)network->m_data);

	client->m_overhead = new COverhead(client->m_videoDevice, client->m_errorLog, client->m_shaderManager, CVertex2(256.0f, 256.0f), CVertex2(10.0f, 10.0f));

	client->m_terrain = new CTerrain(client->m_videoDevice, client->m_errorLog, client->m_textureManager, client->m_shaderManager, (char*)network->m_data);

	// Environment cubes that contain some kind of visible item
	client->m_visibility = new CVisibility(client->m_terrain->m_width, client->m_terrain->m_height, 8);

	CObject* collectable = new CObject(client->m_videoDevice, client->m_errorLog, client->m_wavefrontManager, client->m_textureManager, client->m_shaderManager,
		"model\\cube.wft", "model\\cube.mtl",
		"vertexClient.hlsl", "VSMain", "vs_5_1",
		"pixelClient.hlsl", "PSMain", "ps_5_1",
		false, true, true);

	collectable->SetScale(0.250f, 0.250f, 0.250f);
	collectable->SetPosition(5.0f, 0.50f, 5.0f);
	collectable->Update();

	collectable->m_floats[0]->m_values[0] = 2.0f;
	collectable->m_overheadFloats[0]->m_values[0] = 2.0f;

	collectable->m_isRecordable = true;

	// Determine which cube list to add the object.
	// Would need to test for all the vertices of the models making them contained in each block they belong with.

	// Grab the collectable list for the cube where the client is located
	int px = (int)(collectable->m_position.x + (client->m_visibility->m_width / 2.0f)) / client->m_visibility->m_gridUnits;
	int pz = (int)(collectable->m_position.z + (client->m_visibility->m_height / 2.0f)) / client->m_visibility->m_gridUnits;

	CLinkList<CObject>* cube = (CLinkList<CObject>*)client->m_visibility->m_collectables->GetElement(2, px, pz);

	if (cube != nullptr)
	{
		// Need to perform the constructor when this object is the first one to be added.
		if (cube->m_list == nullptr)
		{
			cube->Constructor();
		}

		cube->Add(collectable, "item01");
	}

	// Must wait on the GPU for texture uploads to complete
	client->m_videoDevice->WaitForGPU();


	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_READY,
		nullptr, 0,
		(void*)serverInfo);

	client->Send(n);

	delete n;
}

/*
*/
void CClient_SendActivity(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	while (client->m_isActiveChat)
	{

	}

	client->m_isActiveActivity = true;

	BYTE types[10] = {};
	int typeCount = 0;

	for (int i = 0; i < CMouseDevice::E_BUTTON_COUNT; i++)
	{
		if ((client->m_mouseDevice->m_keyMap[i].m_count > 0) && (client->m_mouseDevice->m_keyMap[i].m_network != nullptr))
		{
			if (client->m_mouseDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				types[typeCount] = client->m_mouseDevice->m_keyMap[i].m_network->m_type;

				typeCount++;
			}
		}
	}

	for (int i = 0; i < CKeyboardDevice::E_MAX_KEYS; i++)
	{
		if ((client->m_keyboardDevice->m_keyMap[i].m_count > 0) && (client->m_keyboardDevice->m_keyMap[i].m_network != nullptr))
		{
			if (client->m_keyboardDevice->m_keyMap[i].m_network->m_audience == CNetwork::ClientEvent::E_CE_TO_SERVER)
			{
				types[typeCount] = client->m_keyboardDevice->m_keyMap[i].m_network->m_type;

				typeCount++;
			}
		}
	}

	if (client->m_camera[client->m_localClient->m_clientNumber] != nullptr)
	{
		client->m_localClient->SetDirection(&client->m_camera[client->m_localClient->m_clientNumber]->m_look);
	}

	CNetwork* n = new CNetwork(CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ServerEvent::E_SE_ACTIVITY,
		(void*)types, 10,
		(void*)client->m_localClient);

	client->Send(n);

	delete n;

	client->m_isActiveActivity = false;
}

/*
*/
void CClient_ServerFull(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	client->m_errorLog->WriteError(true, "CClient_ServerFull\n");
}

/*
*/
void CClient_Update(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	while (client->m_isActiveRender)
	{
	}

	client->m_isActiveUpdate = true;

	
	// A client disconnected
	if (serverInfo->m_isAvailable)
	{
		delete client->m_camera[serverInfo->m_clientNumber];
		client->m_camera[serverInfo->m_clientNumber] = nullptr;

		client->m_videoDevice->WaitForGPU();

		delete client->m_model[serverInfo->m_clientNumber];
		client->m_model[serverInfo->m_clientNumber] = nullptr;

		serverInfo->Reset();

		client->m_isActiveUpdate = false;

		return;
	}


	if (client->m_model[serverInfo->m_clientNumber] != nullptr)
	{
		client->m_camera[serverInfo->m_clientNumber]->SetPosition(&serverInfo->m_position);
		client->m_camera[serverInfo->m_clientNumber]->UpdateView();


		if (serverInfo->m_clientNumber == client->m_localClient->m_clientNumber)
		{
			client->m_localClient->SetDirection(&client->m_camera[client->m_localClient->m_clientNumber]->m_look);

			client->m_overhead->SetCameraPosition(client->m_camera[client->m_localClient->m_clientNumber]->m_position);
			client->m_overhead->m_camera->UpdateView();
		}

		client->m_model[serverInfo->m_clientNumber]->SetPosition(&serverInfo->m_position);

		client->m_model[serverInfo->m_clientNumber]->m_rotation.y = serverInfo->m_direction.PointToDegree().p.y;

		client->m_model[serverInfo->m_clientNumber]->Update();
	}

	client->m_isActiveUpdate = false;
}

/*
*/
void CClient_UpdateCollectable(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	if (client->m_visibility == nullptr)
	{
		return;
	}

	int px = 0;// (int)(serverInfo->m_position.p.x + (client->m_visibility->m_width / 2.0f)) / client->m_visibility->m_gridUnits;
	int pz = 0;// (int)(serverInfo->m_position.p.z + (client->m_visibility->m_height / 2.0f)) / client->m_visibility->m_gridUnits;

	char itemName[128] = {};
	sscanf_s((char*)network->m_data, "%s %i %i", itemName, 128, &px, &pz);

	CLinkList<CObject>* collectables = (CLinkList<CObject>*)client->m_visibility->m_collectables->GetElement(2, px, pz);

	if (collectables != nullptr)
	{
		if (collectables->m_count > 0)
		{
			CLinkListNode<CObject>* collectable = collectables->Search(itemName);

			if (collectable != nullptr)
			{
				collectable->m_object->m_limboTimer->Start();
			}
		}
	}
}

/*
*/
void CClient_WindowMode(CClient* client, CNetwork* network, CServerInfo* serverInfo)
{
	SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_FULLSCREEN, 0);
}

/*
*/
unsigned __stdcall CClient_ReceiveThread(void* obj)
{
	CClient* client = (CClient*)obj;

#ifdef _DEBUG
	client->m_errorLog->WriteError(true, "CClient_ReceiveThread:Starting\n");
#endif

	client->m_isRunning = true;

	while (client->m_isRunning)
	{
		client->Receive();

		if (client->m_connectionState == CClient::E_NOTCONNECTED)
		{
			SendMessage(client->m_videoDevice->m_hWnd, WM_COMMAND, IDM_DISCONNECT, 0);

			client->m_isRunning = false;
		}
		else
		{
			client->ProcessEvent(client->m_network);
		}
	}

#ifdef _DEBUG
	client->m_errorLog->WriteError(true, "CClient_ReceiveThread Ending\n");
#endif

	_endthreadex(0);

	return 0;
}