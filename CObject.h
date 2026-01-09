#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CFrametime.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CMesh.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CGraphicsAdapter.h"
#include "CLight.h"
#include "COverhead.h"
#include "CPipelineState.h"
#include "CShader.h"
#include "CShaderHeap.h"
#include "CShadowMap.h"
#include "CSound3D.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"
#include "CWavefront.h"

class CObject
{
public:

	bool m_isInitialized;
	bool m_isVisible;
	bool m_useOverhead;

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_blendConstantBuffer;
	CConstantBuffer* m_cameraConstantBuffer;
	CConstantBuffer* m_lightConstantBuffer;
	CConstantBuffer* m_materialConstantBuffers;
	CConstantBuffer* m_overheadCameraConstantBuffer;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLight* m_light;
	CList* m_lights;
	CList* m_meshs;
	CListNode* m_node;
	CLocal* m_local;
	CMesh* m_mesh;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderHeap** m_overheadShaderHeaps;
	CShaderHeap** m_shaderHeaps;
	CShaderMaterial* m_shaderMaterials;
	CShadowMap* m_shadowMap;
	CSound3D* m_sounds[2];
	CSoundDevice* m_soundDevice;
	CString* m_name;
	CTexture* m_texture;
	CTextureManager* m_textureManager;
	CVertexBuffer* m_vertexBuffers;

	float m_amplitude;
	float m_count;
	float m_degree;
	float m_frequency;
	float m_spin;
	float m_wave;

	int32_t m_listGroup;
	int32_t m_materialCount;
	int32_t m_soundCount;

	XMFLOAT4 m_environmentPosition;
	XMFLOAT4 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;

	CObject();
	~CObject();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CErrorLog* errorLog, CLocal* local, const char* name, CWavefront* wavefront,
		CTextureManager* textureManager, CPipelineState* pipelineState, CConstantBuffer* camera, CConstantBuffer* overheadCamera, CConstantBuffer* light,
		bool useOverhead, int32_t listGroup, CShadowMap* shadowMap);

	void AddSound(CWavLoader* wavLoader, bool loop);
	void Animation(CFrametime* frametime);
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void LoadMeshBuffers();
	void Record();
	void Record(COverhead* overhead);
	void RecordConstantBuffers();
	void SetAmbient(CVec3f* ambient);
	void SetAmplitude(float amplitude);
	void SetDiffuse(CVec3f* diffuse);
	void SetDiffuse(XMFLOAT4* diffuse);
	void SetFrequency(float frequency);
	void SetPosition(CVec3f* position);
	void SetPosition(float x, float y, float z);
	void SetRotation(CVec3f* rotation);
	void SetRotation(float x, float y, float z);
	void SetScale(CVec3f* scale);
	void SetScale(float x, float y, float z);
	void SetSpin(float spin);
	void Update();
	void UploadConstantBuffers();
};