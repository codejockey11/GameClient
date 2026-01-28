#include "CSurface.h"

/*
*/
CSurface::CSurface()
{
	memset(this, 0x00, sizeof(CSurface));
}

/*
*/
CSurface::CSurface(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CSurface));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;
}

/*
*/
CSurface::~CSurface()
{
	SAFE_DELETE(m_vertexBuffer);
	SAFE_DELETE(m_indexBuffer);
}

/*
*/
void CSurface::Read(FILE* file)
{
	fread_s(&m_indexCount, sizeof(int32_t), sizeof(int32_t), 1, file);

	m_indices = new int16_t[m_indexCount]();

	int16_t* pi = m_indices;

	int32_t iv = 0;

	for (int32_t i = 0; i < m_indexCount; i++)
	{
		fread_s(&iv, sizeof(int32_t), sizeof(int32_t), 1, file);

		*pi = (int16_t)iv;

		pi++;
	}

	m_indexBuffer = new CIndexBuffer();

	m_indexBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_indexCount, m_indices);

	m_indexBuffer->CreateStaticResource();

	m_indexBuffer->RecordStatic();

	m_indexBuffer->UploadStaticResources();

	m_indexBuffer->ReleaseStaticCPUResource();

	fread_s(&m_vertexCount, sizeof(int32_t), sizeof(int32_t), 1, file);

	m_vertices = new CVertexNT[m_vertexCount]();

	fread_s(m_vertices, sizeof(CVertexNT) * m_vertexCount, sizeof(CVertexNT), m_vertexCount, file);

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_material, CPipelineState::VertexType::E_VT_VERTEXNT, m_vertexCount, (void*)m_vertices);

	m_vertexBuffer->CreateStaticResource();

	m_vertexBuffer->RecordStatic();

	m_vertexBuffer->UploadStaticResources();

	m_vertexBuffer->ReleaseStaticCPUResource();

	//m_vertexBuffer->CreateDynamicResource();
	
	//m_vertexBuffer->LoadDynamicBuffer();

	SAFE_DELETE_ARRAY(m_vertices);
	SAFE_DELETE_ARRAY(m_indices);
}