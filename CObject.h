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

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
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

	bool m_isInitialized;
	bool m_isVisible;
	bool m_useOverhead;

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_constantBuffer;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLinkList<CMesh>* m_meshs;
	CLinkListNode<CMesh>* m_meshNode;
	CLocal* m_local;
	CMesh* m_mesh;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CShader* m_overheadShader;
	CShader* m_overheadShaders;
	CShader* m_shader;
	CShader* m_shaders;
	CShaderBinary* m_pixelShaderBinary;
	CShaderBinary* m_vertexShaderBinary;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShaderMaterial* m_shaderMaterial;
	CShaderMaterial* m_shaderMaterials;
	CShadowMap* m_shadowMap;
	CSound3D* m_sounds[2];
	CSoundDevice* m_soundDevice;
	CString* m_name;
	CTexture* m_texture;
	CTextureManager* m_textureManager;
	CVertexBuffer* m_vertexBuffer;
	CVertexBuffer* m_vertexBuffers;

	D3D12_CULL_MODE m_cullMode;

	float m_amplitude;
	float m_count;
	float m_degree;
	float m_frequency;
	float m_spin;
	float m_wave;

	int m_materialCount;
	int m_soundCount;

	int* m_hasmap_bump;
	int* m_hasmap_d;
	int* m_hasmap_Ka;
	int* m_hasmap_Kd;
	int* m_hasmap_Ks;
	int* m_hasmap_Ns;

	XMFLOAT4 m_environmentPosition;
	XMFLOAT4 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;

	CObject();
	~CObject();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CErrorLog* errorLog, CLocal* local, const char* name, CWavefront* wavefront,
		CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, const char* shadername, bool useOverhead, CShadowMap* shadowMap, D3D12_CULL_MODE cullMode);

	void AddSound(CWavLoader* wavLoader, bool loop);
	void Animation(CFrametime* frametime);
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
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