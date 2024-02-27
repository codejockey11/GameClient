#pragma once

#include "framework.h"

#include "CCamera.h"
#include "CErrorLog.h"
#include "CFloatBuffer.h"
#include "CFrametime.h"
#include "CLinkList.h"
#include "CMaterial.h"
#include "CModelManager.h"
#include "COverhead.h"
#include "CPipelineState.h"
#include "CDescriptorRange.h"
#include "CRootSignature.h"
#include "CShaderHeap.h"
#include "CShaderManager.h"
#include "CTextureManager.h"
#include "CTimer.h"
#include "CVertexBuffer.h"
#include "CVideoDevice.h"
#include "CWavefrontManager.h"

class CObject
{
public:

	CErrorLog* m_errorLog;
	CFloat4x4Buffer* m_overheadWvp;
	CFloat4x4Buffer* m_wvp;
	CFloatBuffer* m_floats[CMaterial::E_MATERIAL_COUNT];
	CFloatBuffer* m_overheadFloats[CMaterial::E_MATERIAL_COUNT];
	CIntBuffer* m_ints[CMaterial::E_MATERIAL_COUNT];
	CIntBuffer* m_overheadInts[CMaterial::E_MATERIAL_COUNT];
	CShaderHeap* m_shaderHeap[CMaterial::E_MATERIAL_COUNT];
	CShaderHeap* m_overheadHeap[CMaterial::E_MATERIAL_COUNT];
	CString* m_name;
	CTextureManager* m_textureManager;
	CTimer* m_limboTimer;
	CVertexBuffer* m_vertexBuffers[CMaterial::E_MATERIAL_COUNT];
	CVideoDevice* m_videoDevice;

	bool m_isRecordable;
	
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	
	XMFLOAT4 m_position;
	XMFLOAT4 m_environmentPosition;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;
	
	CObject();
	CObject(CModelManager* modelManager, CWavefrontManager* wavefrontManager, const char* filename, const char* materialname);
	CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, UINT descriptorCount);
	CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, CModelManager* modelManager, CTextureManager* textureManager, CShaderManager* shaderManager,
		const char* filename,
		const char* vertexShadername, const char* vertexEntrypoint, const char* vertexVersion,
		const char* pixelShaderName, const char* pixelEntryPoint, const char* pixelVersion,
		bool alphaToCoverageEnable, bool blendEnable, bool useOverhead);
	CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, CWavefrontManager* wavefrontManager, CTextureManager* textureManager, CShaderManager* shaderManager,
		const char* filename, const char* materialname,
		const char* vertexShadername, const char* vertexEntrypoint, const char* vertexVersion,
		const char* pixelShaderName, const char* pixelEntryPoint, const char* pixelVersion,
		bool alphaToCoverageEnable, bool blendEnable, bool useOverhead);
	~CObject();

	void Animation(CFrametime* frametime);
	void CreateDeviceResources(bool alphaToCoverageEnable, bool blendEnable);
	void LoadMeshBuffers(CTextureManager* textureManager, CLinkListNode<CMesh>* meshs);
	void Record();
	void Record(COverhead* overhead);
	void SetCurrentCamera(CCamera* camera);
	void SetOverheadCamera(CCamera* camera);
	void SetPosition(float x, float y, float z);
	void SetPosition(CVertex* position);
	void SetRotation(float x, float y, float z);
	void SetRotation(CVertex* position);
	void SetScale(float x, float y, float z);
	void Update();
	void UpdateServer();

private:

	CLinkList<CMesh>* m_meshs;
	CModel* m_model;
	CPipelineState* m_pipelineState;
	CDescriptorRange* m_range;
	CRootSignature* m_rootSignature;
	CShader* m_pixelShader;
	CShader* m_vertexShader;
	CTexture* texture;
	CWavefront* m_wavefront;

	bool m_useOverhead;
	
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CVideoDevice::E_BACKBUFFER_COUNT];

	float m_bob;
	float m_count;
	float m_spin;
};