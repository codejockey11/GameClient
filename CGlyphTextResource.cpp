#include "CGlyphTextResource.h"

/*
*/
CGlyphTextResource::CGlyphTextResource()
{
	memset(this, 0x00, sizeof(CGlyphTextResource));
}

/*
*/
CGlyphTextResource::~CGlyphTextResource()
{
	delete m_vertexBuffer;
}