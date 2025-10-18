#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CFrametime.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CMesh.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CGraphicsAdapter.h"
#include "COverhead.h"
#include "CShaderBinaryManager.h"
#include "CShadowMap.h"
#include "CSound3D.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"
#include "CWavefront.h"

class CObject
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLinkList<CMesh>* m_meshs;
	CLocal* m_local;
	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderBinary* m_pixelShaderBinary;
	CShaderBinary* m_vertexShaderBinary;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShaderMaterial* m_shaderMaterial;
	CShadowMap* m_shadowMap;
	CSound3D* m_sounds[2];
	CSoundDevice* m_soundDevice;
	CString* m_name;
	CTextureManager* m_textureManager;
	CVertexBuffer* m_vertexBuffers;

	bool m_isInitialized;
	bool m_isVisible;
	bool m_useOverhead;

	float m_amplitude;
	float m_frequency;
	float m_wave;
	float m_count;
	float m_degree;
	float m_spin;

	int m_materialCount;
	int m_soundCount;

	int* m_hasmap_Kd;
	int* m_hasmap_Ka;
	int* m_hasmap_Ks;
	int* m_hasmap_Ns;
	int* m_hasmap_d;
	int* m_hasmap_bump;

	D3D12_CULL_MODE m_cullMode;

	XMFLOAT4 m_environmentPosition;
	XMFLOAT4 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CObject();
	~CObject();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CErrorLog* errorLog, CLocal* local, const char* name, CWavefront* wavefront,
		CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, const char* shadername, bool useOverhead, CShadowMap* shadowMap, D3D12_CULL_MODE cullMode);

	void AddSound(CWavLoader* wavLoader, bool loop);
	void Animation(CFrametime* frametime);
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList>);
	void InitMaterialBuffer();
	void LoadMeshBuffers(CTextureManager* textureManager);
	void Record();
	void Record(COverhead* overhead);
	void SetAmplitude(float amplitude);
	void SetCurrentCamera(CCamera* camera);
	void SetDiffuse(CVec3f* diffuse);
	void SetDiffuse(XMFLOAT4* diffuse);
	void SetFrequency(float frequency);
	void SetOverheadCamera(CCamera* camera);
	void SetPosition(CVec3f* position);
	void SetPosition(float x, float y, float z);
	void SetRotation(CVec3f* rotation);
	void SetRotation(float x, float y, float z);
	void SetScale(CVec3f* scale);
	void SetScale(float x, float y, float z);
	void SetSpin(float spin);
	void Update();
};