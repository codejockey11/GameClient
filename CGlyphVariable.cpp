#include "CGlyphVariable.h"

/*
*/
CGlyphVariable::CGlyphVariable()
{
	memset(this, 0x00, sizeof(CGlyphVariable));
}

/*
*/
CGlyphVariable::CGlyphVariable(CTexture* glyphs, int32_t size)
{
	memset(this, 0x00, sizeof(CGlyphVariable));

	m_glyphs = glyphs;

	m_size = (float)size;

	m_glyphHeight = (float)m_glyphs->m_height;

	m_glyphV = 1.0f;

	m_sizeRatio = m_size / m_glyphHeight;

	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2

	m_glyphVertices = new CHeapArray(true, sizeof(CGlyphVertices), 1, 94);

	CGlyphVariable::DefineUForResourceUV();

	for (int32_t i = 0; i < 94; i++)
	{
		m_glyphVertice = (CGlyphVertices*)m_glyphVertices->GetElement(1, i);

		m_glyphVertice->m_width = (((m_glyphWidth[i + 1] - 1) - m_glyphWidth[i]) + 1.0f);

		m_glyphVertice->m_vertices[0].m_p.x = m_glyphWidth[i];
		m_glyphVertice->m_vertices[0].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_p.x = (((m_glyphWidth[i + 1] - 1) - m_glyphWidth[i]) + 1.0f) * m_sizeRatio;
		m_glyphVertice->m_vertices[1].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_p.x = (((m_glyphWidth[i + 1] - 1) - m_glyphWidth[i]) + 1.0f) * m_sizeRatio;
		m_glyphVertice->m_vertices[2].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[3].m_p.x = m_glyphWidth[i];
		m_glyphVertice->m_vertices[3].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[0].m_uv.x = m_glyphWidth[i] / m_glyphs->m_width;
		m_glyphVertice->m_vertices[0].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_uv.x = (m_glyphWidth[i + 1] - 1) / m_glyphs->m_width;
		m_glyphVertice->m_vertices[1].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_uv.x = (m_glyphWidth[i + 1] - 1) / m_glyphs->m_width;
		m_glyphVertice->m_vertices[2].m_uv.y = m_glyphV;

		m_glyphVertice->m_vertices[3].m_uv.x = m_glyphWidth[i] / m_glyphs->m_width;
		m_glyphVertice->m_vertices[3].m_uv.y = m_glyphV;
	}
}

/*
*/
CGlyphVariable::~CGlyphVariable()
{
	SAFE_DELETE(m_glyphVertices);
}

/*
*/
void CGlyphVariable::DefineUForResourceUV()
{
	m_glyphWidth[0] = 0.0f;
	m_glyphWidth[1] = 11.0f;
	m_glyphWidth[2] = 19.0f;
	m_glyphWidth[3] = 32.0f;
	m_glyphWidth[4] = 49.0f;
	m_glyphWidth[5] = 64.0f;
	m_glyphWidth[6] = 88.0f;
	m_glyphWidth[7] = 111.0f;
	m_glyphWidth[8] = 118.0f;
	m_glyphWidth[9] = 128.0f;
	m_glyphWidth[10] = 139.0f;
	m_glyphWidth[11] = 153.0f;
	m_glyphWidth[12] = 169.0f;
	m_glyphWidth[13] = 177.0f;
	m_glyphWidth[14] = 188.0f;
	m_glyphWidth[15] = 196.0f;
	m_glyphWidth[16] = 210.0f;
	m_glyphWidth[17] = 228.0f;
	m_glyphWidth[18] = 243.0f;
	m_glyphWidth[19] = 259.0f;
	m_glyphWidth[20] = 274.0f;
	m_glyphWidth[21] = 291.0f;
	m_glyphWidth[22] = 307.0f;
	m_glyphWidth[23] = 323.0f;
	m_glyphWidth[24] = 338.0f;
	m_glyphWidth[25] = 355.0f;
	m_glyphWidth[26] = 372.0f;
	m_glyphWidth[27] = 379.0f;
	m_glyphWidth[28] = 389.0f;
	m_glyphWidth[29] = 405.0f;
	m_glyphWidth[30] = 421.0f;
	m_glyphWidth[31] = 437.0f;
	m_glyphWidth[32] = 452.0f;
	m_glyphWidth[33] = 480.0f;
	m_glyphWidth[34] = 501.0f;
	m_glyphWidth[35] = 518.0f;
	m_glyphWidth[36] = 536.0f;
	m_glyphWidth[37] = 556.0f;
	m_glyphWidth[38] = 572.0f;
	m_glyphWidth[39] = 585.0f;
	m_glyphWidth[40] = 606.0f;
	m_glyphWidth[41] = 626.0f;
	m_glyphWidth[42] = 633.0f;
	m_glyphWidth[43] = 646.0f;
	m_glyphWidth[44] = 663.0f;
	m_glyphWidth[45] = 677.0f;
	m_glyphWidth[46] = 705.0f;
	m_glyphWidth[47] = 725.0f;
	m_glyphWidth[48] = 748.0f;
	m_glyphWidth[49] = 764.0f;
	m_glyphWidth[50] = 787.0f;
	m_glyphWidth[51] = 804.0f;
	m_glyphWidth[52] = 818.0f;
	m_glyphWidth[53] = 836.0f;
	m_glyphWidth[54] = 855.0f;
	m_glyphWidth[55] = 874.0f;
	m_glyphWidth[56] = 903.0f;
	m_glyphWidth[57] = 921.0f;
	m_glyphWidth[58] = 937.0f;
	m_glyphWidth[59] = 954.0f;
	m_glyphWidth[60] = 962.0f;
	m_glyphWidth[61] = 977.0f;
	m_glyphWidth[62] = 986.0f;
	m_glyphWidth[63] = 1002.0f;
	m_glyphWidth[64] = 1019.0f;
	m_glyphWidth[65] = 1029.0f;
	m_glyphWidth[66] = 1045.0f;
	m_glyphWidth[67] = 1062.0f;
	m_glyphWidth[68] = 1075.0f;
	m_glyphWidth[69] = 1092.0f;
	m_glyphWidth[70] = 1107.0f;
	m_glyphWidth[71] = 1117.0f;
	m_glyphWidth[72] = 1133.0f;
	m_glyphWidth[73] = 1150.0f;
	m_glyphWidth[74] = 1157.0f;
	m_glyphWidth[75] = 1166.0f;
	m_glyphWidth[76] = 1181.0f;
	m_glyphWidth[77] = 1189.0f;
	m_glyphWidth[78] = 1215.0f;
	m_glyphWidth[79] = 1232.0f;
	m_glyphWidth[80] = 1249.0f;
	m_glyphWidth[81] = 1266.0f;
	m_glyphWidth[82] = 1283.0f;
	m_glyphWidth[83] = 1294.0f;
	m_glyphWidth[84] = 1305.0f;
	m_glyphWidth[85] = 1317.0f;
	m_glyphWidth[86] = 1333.0f;
	m_glyphWidth[87] = 1348.0f;
	m_glyphWidth[88] = 1372.0f;
	m_glyphWidth[89] = 1387.0f;
	m_glyphWidth[90] = 1403.0f;
	m_glyphWidth[91] = 1415.0f;
	m_glyphWidth[92] = 1431.0f;
	m_glyphWidth[93] = 1442.0f;
	m_glyphWidth[94] = 1452.0f;
}