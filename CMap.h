#pragma once

#include "framework.h"

#include "../GameCommon/CEntity.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CGraphicsAdapter.h"
#include "CObject.h"
#include "COverhead.h"
#include "CPipelineState.h"
#include "CShader.h"
#include "CShaderHeap.h"
#include "CShadowMap.h"
#include "CSoundDevice.h"
#include "CSoundManager.h"
#include "CSurface.h"
#include "CTerrain.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"
#include "CWavefrontManager.h"

class CMap
{
public:

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_blendConstantBuffer;
	CConstantBuffer* m_cameraConstantBuffer;
	CConstantBuffer* m_lightConstantBuffer;
	CConstantBuffer* m_materialConstantBuffers;
	CConstantBuffer* m_overheadCameraConstantBuffer;
	CEntity* m_entity;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_key[CKeyValue::MAX_KEY];
	char m_value[CKeyValue::MAX_VALUE];

	char* m_objectName;

	CList* m_collectables;
	CList* m_lights;
	CList* m_sectors;
	CList* m_statics;
	CList* m_surfaces;
	CListNode* m_node;
	CLight* m_light;
	CLocal* m_local;
	CObject* m_object;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CPipelineState* m_pipelineStateCollectable;
	CPipelineState* m_pipelineStateTerrain;
	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderHeap* m_overheadShaderHeap;
	CShaderHeap** m_overheadShaderHeaps;
	CShaderHeap* m_shaderHeap;
	CShaderHeap** m_shaderHeaps;

	CShaderMaterial* m_shaderMaterial;
	CShadowMap* m_shadowMap;
	CSoundDevice* m_soundDevice;
	CSoundManager* m_soundManager;
	CSurface* m_surface;
	CString* m_filename;
	CString* m_loadMessage;
	CString* m_name;
	CTerrain* m_terrain;
	CTexture* m_texture;
	CTextureManager* m_textureManager;
	CVec3f m_ambient;
	CVec3f m_vec3f;
	CVec3i m_index;
	CVec3i m_mapSize;
	CWavefront* m_wavefront;
	CWavefrontManager* m_wavefrontManager;
	CWavLoader* m_wavLoader;

	errno_t m_err;

	FILE* m_fMap;

	float m_float;

	int32_t m_listGroup;
	int32_t m_entityCount;
	int32_t m_keyValueCount;
	int32_t m_materialNumber;
	int32_t m_maxEntityCount;
	int32_t m_numberOfMaterials;
	int32_t m_sectorSize;
	int32_t m_vertexBufferCount;
	int32_t m_verticesCount;

	XMFLOAT4 m_xmfloat4;

	CMap();
	CMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CSoundDevice* soundDevice, CWavefrontManager* wavefrontManager,
		CPipelineState* pipelineState, CPipelineState* m_pipelineStateCollectable, CPipelineState* m_pipelineStateTerrain, CSoundManager* m_soundManager,
		int32_t listGroup, CShadowMap* shadowMap, CConstantBuffer* camera, CConstantBuffer* overheadCamera, const char* name, const char* filename);
	~CMap();

	void Load();
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Record();
	void Record(COverhead* overhead);
	void RecordConstantBuffers();
	void SetLights(CList* lights);
};