#pragma once

#include "framework.h"

#include "CCamera.h"
#include "CErrorLog.h"
#include "CHeapArray.h"
#include "CKeyboardDevice.h"
#include "CModelManager.h"
#include "CMouseDevice.h"
#include "CNetwork.h"
#include "COverhead.h"
#include "CServerInfo.h"
#include "CShaderManager.h"
#include "CSoundManager.h"
#include "CTerrain.h"
#include "CTextureManager.h"
#include "CVisibility.h"
#include "CVideoDevice.h"
#include "CWavefrontManager.h"

class CClient
{
public:

	enum ClientState
	{
		E_CONNECTED = 1,
		E_CONNECTING,
		E_NOTCONNECTED
	};

	enum
	{
		E_HOSTNAME_LENGTH = 64
	};
	
	CCamera* m_camera[CServerInfo::E_MAX_CLIENTS];
	CErrorLog* m_errorLog;
	CHeapArray* m_serverInfo;
	CKeyboardDevice* m_keyboardDevice;
	CModelManager* m_modelManager;
	CMouseDevice* m_mouseDevice;
	CNetwork* m_network;
	COverhead* m_overhead;
	CObject* m_model[CServerInfo::E_MAX_CLIENTS];
	CServerInfo* m_localClient;
	CShaderManager* m_shaderManager;
	CTerrain* m_terrain;
	CTextureManager* m_textureManager;
	CVideoDevice* m_videoDevice;
	CVisibility* m_visibility;
	CWavefrontManager* m_wavefrontManager;
	CSoundManager* m_soundManager;

	BYTE m_connectionState;
	
	HANDLE m_hThread;
	
	SOCKET m_socket;
	
	UINT m_receiveThreadId;
	
	bool m_isActiveActivity;
	bool m_isActiveChat;
	bool m_isActiveRender;
	bool m_isActiveUpdate;
	bool m_isDisconnecting;
	bool m_isRunning;
	
	char m_hostname[CClient::E_HOSTNAME_LENGTH];
	char m_name[CServerInfo::E_NAME_SIZE];
	char m_chat[CServerInfo::E_CHAT_SIZE];

	void (*pFunc[CNetwork::ClientEvent::E_CE_MAX_EVENTS]) (CClient* client, CNetwork* network, CServerInfo* serverInfo);

	CClient();
	CClient(CVideoDevice* videoDevice, CMouseDevice* mouseDevice, CKeyboardDevice* keyboardDevice, CErrorLog* errorLog, CTextureManager* textureManager, CShaderManager* shaderManager, CModelManager* modelManager, CWavefrontManager* wavefrontManager, CSoundManager* soundManager);
	~CClient();

	void Connect(const char* address);
	void Connect(const char* address, const char* port);
	bool ConnectSocket(addrinfo* ptr);
	void CreateSocket(const char* address, const char* port);
	void DestroyEnvironment();
	void InitializeWinsock();
	void Disconnect();
	void ProcessEvent(CNetwork* network);
	void Receive();
	void Send(CNetwork* network);
	void SetLogin(const char* name);

private:

	char m_ip[15];
	char m_port[6];

	WSADATA	m_wsaData;
};