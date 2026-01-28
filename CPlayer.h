#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CServerInfo.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CConstantBuffer.h"
#include "CGraphicsAdapter.h"
#include "CShadowMap.h"
#include "CSoundDevice.h"
#include "CSoundManager.h"
#include "CObject.h"
#include "CPipelineState.h"
#include "CTextureManager.h"
#include "CWavefrontManager.h"

class CPlayer
{
public:

	bool m_isInitialized;
	bool m_useOverhead;

	BYTE m_team;

	CCamera* m_camera;
	CConstantBuffer* m_cameraConstantBuffer;
	CConstantBuffer* m_lightConstantBuffer;
	CConstantBuffer* m_overheadCameraConstantBuffer;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CListNode* m_node;
	CLocal* m_local;
	CMesh* m_mesh;
	CObject* m_model;
	CPipelineState* m_clientPipelineState;
	CShadowMap* m_shadowMap;
	CSoundDevice* m_soundDevice;
	CSoundManager* m_soundManager;
	CString* m_modelName;
	CString* m_name;
	CTextureManager* m_textureManager;
	CVec3f m_position;
	CVec3f m_rotation;
	CWavefrontManager* m_wavefrontManager;
	CWavLoader* m_sound;

	int32_t m_listGroup;

	CPlayer();
	~CPlayer();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CSoundManager* soundManager, CErrorLog* errorLog, CLocal* local,
		CWavefrontManager* wavefrontManager, CTextureManager* textureManager, CPipelineState* clientPipelineState, CConstantBuffer* cameraConstantBuffer,
		CConstantBuffer* overheadCameraConstantBuffer, bool useOverhead, int32_t listGroup, CShadowMap* shadowMap);
	void Deconstructor();
	void SetDirection(CVec3f* direction);
	void SetModel(const char* modelName);
	void SetName(const char* name);
	void SetPosition(CVec3f* position);
	void SetScale(float s);
	void SetTeam(BYTE team);
};