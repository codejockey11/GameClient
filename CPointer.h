#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVec2f.h"

#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CTexture.h"

class CPointer
{
public:

	CImage* m_image;

	CPointer();
	CPointer(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture,
		CVec2f position, CVec2f size);
	~CPointer();

};