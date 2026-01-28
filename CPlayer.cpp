#include "CPlayer.h"

/*
*/
CPlayer::CPlayer()
{
	memset(this, 0x00, sizeof(CPlayer));
}

/*
*/
CPlayer::~CPlayer()
{
	SAFE_DELETE(m_camera);
	SAFE_DELETE(m_model);
	SAFE_DELETE(m_modelName);
	SAFE_DELETE(m_name);
}

/*
*/
void CPlayer::Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CSoundManager* soundManager, CErrorLog* errorLog, CLocal* local,
	CWavefrontManager* wavefrontManager, CTextureManager* textureManager, CPipelineState* clientPipelineState, CConstantBuffer* cameraConstantBuffer,
	CConstantBuffer* overheadCameraConstantBuffer, bool useOverhead, int32_t listGroup, CShadowMap* shadowMap)
{
	memset(this, 0x00, sizeof(CPlayer));

	m_graphicsAdapter = graphicsAdapter;

	m_soundDevice = soundDevice;
	
	m_soundManager = soundManager;
	
	m_errorLog = errorLog;
	
	m_local = local;
	
	m_wavefrontManager = wavefrontManager;
	
	m_textureManager = textureManager;
	
	m_clientPipelineState = clientPipelineState;
	
	m_cameraConstantBuffer = cameraConstantBuffer;
	
	m_overheadCameraConstantBuffer = overheadCameraConstantBuffer;
	
	m_useOverhead = useOverhead;
	
	m_listGroup = listGroup;
	
	m_shadowMap = shadowMap;

	m_camera = new CCamera();

	m_camera->Constructor((float)m_graphicsAdapter->m_width,
		(float)m_graphicsAdapter->m_height,
		&m_position,
		45.0f,
		1.0f, 50000.0f,
		(16.0f / 9.0f));
}

/*
*/
void CPlayer::Deconstructor()
{
	CPlayer::~CPlayer();
}

/*
*/
void CPlayer::SetDirection(CVec3f* direction)
{
	m_model->m_rotation.y = direction->PointToDegree().m_p.y;

	m_model->Update();
}

/*
*/
void CPlayer::SetModel(const char* modelName)
{
	SAFE_DELETE(m_model);

	m_model = new CObject();

	CWavefront* wavefront = m_wavefrontManager->Create(modelName);

	m_model->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, modelName, wavefront, m_textureManager, m_clientPipelineState,
		m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_lightConstantBuffer, m_useOverhead, m_listGroup, m_shadowMap);

	m_sound = m_soundManager->Create("audio/mono/rifle.wav");

	m_model->AddSound(m_sound, false);

	m_sound = m_soundManager->Create("audio/mono/shotgun.wav");

	m_model->AddSound(m_sound, false);

	m_model->m_sounds[0]->SetPosition(m_position.m_p.x, m_position.m_p.y, m_position.m_p.z);
	m_model->m_sounds[1]->SetPosition(m_position.m_p.x, m_position.m_p.y, m_position.m_p.z);

	m_model->m_sounds[0]->SetRange(256.0f);
	m_model->m_sounds[1]->SetRange(256.0f);
}

/*
*/
void CPlayer::SetName(const char* name)
{
	SAFE_DELETE(m_name);

	m_name = new CString(name);
}

/*
*/
void CPlayer::SetPosition(CVec3f* position)
{
	m_position = *position;

	m_model->SetPosition(position);
	m_model->Update();

	m_model->m_sounds[0]->SetPosition(m_position.m_p.x, m_position.m_p.y, m_position.m_p.z);
	m_model->m_sounds[1]->SetPosition(m_position.m_p.x, m_position.m_p.y, m_position.m_p.z);

	m_camera->SetPosition(&m_position);
	m_camera->UpdateView();
}

/*
*/
void CPlayer::SetScale(float s)
{
	m_model->m_scale.x = s;
	m_model->m_scale.y = s;
	m_model->m_scale.z = s;
}

/*
*/
void CPlayer::SetTeam(BYTE team)
{
	m_team = team;

	m_node = m_model->m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		switch (m_team)
		{
		case CServerInfo::E_TEAM_RED:
		{
			m_model->m_shaderMaterials[m_mesh->m_material.m_number].SetKd(1.0f, 0.0f, 0.0f, 1.0f);
			m_model->m_shaderMaterials[m_mesh->m_material.m_number].SetKe(0.5f, 0.0f, 0.0f, 1.0f);

			break;
		}
		case CServerInfo::E_TEAM_BLUE:
		{
			m_model->m_shaderMaterials[m_mesh->m_material.m_number].SetKd(0.0f, 0.0f, 1.0f, 1.0f);
			m_model->m_shaderMaterials[m_mesh->m_material.m_number].SetKe(0.0f, 0.0f, 0.5f, 1.0f);

			break;
		}
		}

		m_node = m_node->m_next;
	}
}