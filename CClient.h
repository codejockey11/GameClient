#pragma once

#include "framework.h"

#include "resource.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CDirectoryList.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CLobbyServerInfo.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CMesh.h"
#include "../GameCommon/CNetwork.h"
#include "../GameCommon/CServerInfo.h"
#include "../GameCommon/CSocket.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CXML.h"

#include "CButtonManager.h"
#include "CChatBox.h"
#include "CGlyphFixed.h"
#include "CGlyphText.h"
#include "CGlyphVariable.h"
#include "CGraphicsAdapter.h"
#include "CKeyboardDevice.h"
#include "CMap.h"
#include "CMouseDevice.h"
#include "CObject.h"
#include "COverhead.h"
#include "CPanel.h"
#include "CShaderBinaryManager.h"
#include "CSound.h"
#include "CSound3D.h"
#include "CSoundManager.h"
#include "CTextureManager.h"
#include "CVideoDevice.h"
#include "CVisibility.h"
#include "CWavefrontManager.h"

class CClient
{
public:

	enum NetworkState
	{
		E_NOTCONNECTED = 0,
		E_CONNECTED,
		E_CONNECTING
	};

	enum ClientState
	{
		E_LOGIN = 0,
		E_LOBBY,
		E_GAME,
		E_LOADING,

		E_MAX_CLIENT_STATE
	};

	enum
	{
		E_IP_LENGTH = 15,
		E_PORT_LENGTH = 6,
		E_HOSTNAME_LENGTH = 64,
		E_MAX_LOAD_STAGE = 3
	};

	CButton* m_buttonLobby;
	CButtonManager* m_buttonManager;
	CCamera* m_camera;
	CCamera* m_currentCamera;
	CChatBox* m_chatBox;
	CErrorLog* m_errorLog;
	CFont* m_consolas12;
	CFont* m_fontArial;
	CFontManager* m_fontManager;
	CFrametime* m_frametime;
	CGlyphFixed* m_glyphFixed;
	CGlyphText* m_text1;
	CGlyphText* m_text2;
	CGlyphText* m_text3;
	CGlyphVariable* m_glyphVariable;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeapArray* m_serverInfos;
	CImage* m_cursor;
	CKeyboardDevice* m_keyboardDevice;
	CLobbyServerInfo* m_lobbyServerInfo;
	CLocal* m_local;
	CMap* m_map;
	CMouseDevice* m_mouseDevice;
	CNetwork* m_lobbyNetwork;
	CNetwork* m_network;
	CObject* m_model;
	COverhead* m_overhead;
	CPanel* m_login;
	CServerInfo* m_localClient;
	CServerInfo* m_serverInfo;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShadowMap* m_shadowMap;
	CSocket* m_gameSocket;
	CSocket* m_lobbySocket;
	CSound* m_frameSounds[32];
	CSound3D* m_frame3DSounds[32];
	CSoundDevice* m_soundDevice;
	CSoundManager* m_soundManager;
	CString* m_accountInfoXML;
	CTextureManager* m_textureManager;
	CToken m_mapScript;
	CVec3f m_lastDirection;
	CVideoDevice* m_videoDevice;
	CVisibility* m_visibility;
	CWavefrontManager* m_wavefrontManager;

	bool m_isAccountRunning;
	bool m_isDrawing;
	bool m_isRunning;

	char m_chat[CServerInfo::E_CHAT_SIZE];
	char m_data[CServerInfo::E_CHAT_SIZE];
	char m_hostname[CClient::E_HOSTNAME_LENGTH];
	char m_ip[15];
	char m_modelName[CServerInfo::E_MODEL_NAME_SIZE];
	char m_playerName[CServerInfo::E_PLAYER_NAME_SIZE];
	char m_port[6];

	HANDLE m_hAccountThread;
	HANDLE m_hLoadThread;
	HANDLE m_hThread;

	int m_clientNumber;
	int m_frame3DSoundCount;
	int m_frameSoundCount;
	int m_state;

	UINT m_loadEnvironmentThreadId;
	UINT m_receiveAccountThreadId;
	UINT m_receiveThreadId;

	WSADATA	m_wsaData;

	typedef void (CClient::* TMethod)();

	TMethod m_frame[CClient::E_MAX_CLIENT_STATE];
	TMethod m_lobby[CNetwork::ClientEvent::E_CE_MAX_EVENTS];
	TMethod m_method[CNetwork::ClientEvent::E_CE_MAX_EVENTS];

	CClient();
	CClient(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CKeyboardDevice* keyboardDevice, CErrorLog* errorLog, CLocal* local);
	~CClient();

	// Base
	void Connect(const char* address, const char* port);
	void ConnectAccount(const char* address, const char* port);
	void DestroyClientList();
	void DestroyEnvironment();
	void Disconnect();
	void DisconnectAccount();
	void DisconnectMessage();
	void DrawHud();
	void DrawLoading();
	void DrawLoadingScreen(CImage* loadingScreen, char* mapName, char* name, char* item);
	void DrawScene();
	void DrawText11On12();
	void InitializeWinsock();
	void ProcessAccountEvent();
	void ProcessEvent();
	void QueSound(CSound* sound);
	void QueSound(CSound3D* sound);
	void Send(CNetwork* network);
	void SetLogin(const char* name);
	void SetModelName(const char* modelName);
	void SkipDoubleSection();
	void SkipSingleSection();

	// Network
	void Accepted();
	void AcceptedAccount();
	void AccountInfo();
	void AccountInfoEnd();
	void AccountServerFull();
	void Chat();
	void ChatBox();
	void CloseBrowser();
	void Connect();
	void ConnectAccount();
	void DrawFrame();
	void Enter();
	void Exit();
	void ExitGame();
	void InfoRequest();
	void LoadEnvironment();
	void QueSound();
	void ReadyCheck();
	void SendActivity();
	void SendNullActivity();
	void ServerFull();
	void OpenBrowser();
	void Update();
	void UpdateCollectable();
	void WindowMode();

	// Client State
	void Frame();
	void DrawLogin();
	void DrawLobby();
	void DrawGame();

	static unsigned __stdcall AccountReceiveThread(void* obj);
	static unsigned __stdcall LoadEnvironmentThread(void* obj);
	static unsigned __stdcall ReceiveThread(void* obj);
};