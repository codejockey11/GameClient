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
#include "CCommandListGroup.h"
#include "CConsole.h"
#include "CConstantBuffer.h"
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
#include "CPipelineState.h"
#include "CPlayer.h"
#include "CPointer.h"
#include "CShaderBinaryManager.h"
#include "CSound.h"
#include "CSound3D.h"
#include "CSoundManager.h"
#include "CTextureManager.h"
#include "CVideoDevice.h"
#include "CWavefrontManager.h"

class CClient
{
public:

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

	bool m_lobbyIsRunning;
	bool m_gameIsRunning;
	bool m_isLoading;

	CButtonManager* m_buttonManager;
	CCamera* m_currentCamera;
	CCommandListGroup* m_commandListGroup[CCommandListGroup::E_MAX_LIST_GROUP];
	CConsole* m_console;
	CConstantBuffer* m_cameraConstantBuffer;
	CConstantBuffer* m_imageWvp;
	CConstantBuffer* m_overheadCameraConstantBuffer;
	CDirectoryList* m_directoryList;
	CErrorLog* m_errorLog;
	CFont* m_calibri12;
	CFont* m_consolas12;
	CFont* m_fontArial;
	CFontManager* m_fontManager;
	CFrametime* m_frametime;
	CGlyphFixed* m_glyphFixed;
	CGlyphText* m_fpsText;
	CGlyphText* m_loadingText;
	CGlyphText* m_scoreboardText[CServerInfo::E_MAX_CLIENTS];
	CGlyphVariable* m_glyphVariable;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_data[CServerInfo::E_DATA_SIZE];
	char m_modelName[CServerInfo::E_MODEL_NAME_SIZE];
	char m_playerName[CServerInfo::E_PLAYER_NAME_SIZE];

	CHeapArray* m_serverInfos;
	CKeyboardDevice* m_keyboardDevice;
	CList* m_collectables;
	CListNode* m_node;
	CLobbyServerInfo* m_lobbyServerInfo;
	CLobbyServerInfo* m_lobbyLocalClient;
	CLocal* m_local;
	CMap* m_map;
	CMesh* m_mesh;
	CMouseDevice* m_mouseDevice;
	CNetwork* m_gameNetwork;
	CNetwork* m_lobbyNetwork;
	CObject* m_object;
	COverhead* m_overhead;
	CPanel* m_login;
	CPipelineState* m_clientPipelineState;
	CPipelineState* m_collectablePipelineState;
	CPipelineState* m_imagePipelineState;
	CPipelineState* m_mapPipelineState;
	CPipelineState* m_terrainPipelineState;
	CPlayer* m_player;
	CPointer* m_pointer;
	CScript m_mapScript;
	CServerInfo* m_localClient;
	CServerInfo* m_serverInfo;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShadowMap* m_shadowMap;
	CSocket* m_gameSocket;
	CSocket* m_lobbySocket;
	CSound* m_frameSounds[32];
	CSound3D* m_frame3DSounds[32];
	CSound3D* m_sound3D;
	CSoundDevice* m_soundDevice;
	CSoundManager* m_soundManager;
	CSoundManager* m_soundManagerHud;
	CString* m_accountInfoXML;
	CString* m_mapName;
	CTextureManager* m_textureManager;
	CTextureManager* m_textureManagerHud;
	CVec3f m_lastDirection;
	CVideoDevice* m_videoDevice;
	CWavefront* m_wavefront;
	CWavefrontManager* m_wavefrontManager;
	CWavLoader* m_sound;

	HANDLE m_lobbyThread;
	HANDLE m_loadThread;
	HANDLE m_loadScreenThread;
	HANDLE m_gameThread;

	int32_t m_clientNumber;
	int32_t m_err;
	int32_t m_frame3DSoundCount;
	int32_t m_frameSoundCount;
	int32_t m_index;
	int32_t m_state;

	uint32_t m_loadThreadId;
	uint32_t m_loadScreenThreadId;
	uint32_t m_lobbyThreadId;
	uint32_t m_gameThreadId;

	WORD m_versionRequested;

	WSADATA	m_wsaData;

	X3DAUDIO_VECTOR m_look;
	X3DAUDIO_VECTOR m_position;
	X3DAUDIO_VECTOR m_up;

	typedef void (CClient::* TMethod)();

	TMethod m_event[CNetwork::ClientEvent::E_CE_MAX];
	TMethod m_frame[CClient::ClientState::E_MAX_CLIENT_STATE];

	CClient();
	CClient(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CKeyboardDevice* keyboardDevice, CErrorLog* errorLog, CLocal* local);
	~CClient();

	void Accepted();
	void AcceptedLobby();
	void AccountInfo();
	void AccountInfoEnd();
	void AllocateClientList();
	void CloseBrowser();
	void CompleteLobbyConnect();
	void Connect();
	void Connect(const char* address, const char* port);
	void ConnectLobby();
	void ConnectLobby(const char* address, const char* port);
	void Console();
	void ConsoleMessage();
	void DestroyClientList();
	void DestroyEnvironment();
	void Disconnect();
	void DisconnectLobby();
	void DisconnectMessage();
	void DrawFrame();
	void DrawGame();
	void DrawHud();
	void DrawLoadingScreen(CImage* loadingScreen, char* mapName, char* name, char* item);
	void DrawLobby();
	void DrawLogin();
	void DrawScene();
	void DrawText11On12();
	void Enter();
	void Exit();
	void ExitGame();
	void Frame();
	void InitializeGraphics();
	void InitializeLoginPanel();
	void InitializeManagers();
	void InitializeNetworking();
	void InitializePlayer(CServerInfo* serverInfo);
	void InitializeShaders();
	void InitializeSounds();
	void InitializeWinsock();
	void Loading();
	void LoadEnvironment();
	void LoadPlayer();
	void LobbyServerFull();
	void OpenBrowser();
	void ProcessEvent();
	void ProcessLobbyEvent();
	void QueSound();
	void QueSound(CSound* sound);
	void QueSound(CSound3D* sound);
	void ReadyCheck();
	void RequestAccountInfo();
	void Send(CNetwork* network);
	void SendActivity();
	void SendLobbyMessage();
	void SendLocal(CNetwork* network);
	void SendNullActivity();
	void ServerFull();
	void ServerShutdown();
	void SetData(const char* data);
	void SetLogin(const char* name);
	void SetModelName(const char* modelName);
	void Update();
	void UpdateCollectable();
	void WindowMode();

	static unsigned __stdcall LobbyThread(void* obj);
	static unsigned __stdcall GameThread(void* obj);
	static unsigned __stdcall LoadEnvironmentThread(void* obj);
	static unsigned __stdcall LoadingScreenThread(void* obj);
};