#include "CPointer.h"

/*
*/
CPointer::CPointer()
{
	memset(this, 0x00, sizeof(CPointer));
}

/*
*/
CPointer::CPointer(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture,
	CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CPointer));
	
	m_image = new CImage(graphicsAdapter, errorLog, imageWvp, pipelineState, listGroup, texture, position, size);
}

/*
*/
CPointer::~CPointer()
{
	SAFE_DELETE(m_image);
}