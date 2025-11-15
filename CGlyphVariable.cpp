#include "CGlyphVariable.h"

/*
*/
CGlyphVariable::CGlyphVariable()
{
	memset(this, 0x00, sizeof(CGlyphVariable));
}

/*
*/
CGlyphVariable::CGlyphVariable(CTexture* glyphs, int size)
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

	for (int i = 0; i < 94; i++)
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
	m_glyphWidth[2] = 24.0f;
	m_glyphWidth[3] = 41.0f;
	m_glyphWidth[4] = 56.0f;
	m_glyphWidth[5] = 80.0f;
	m_glyphWidth[6] = 103.0f;
	m_glyphWidth[7] = 110.0f;
	m_glyphWidth[8] = 120.0f;
	m_glyphWidth[9] = 131.0f;
	m_glyphWidth[10] = 145.0f;
	m_glyphWidth[11] = 161.0f;
	m_glyphWidth[12] = 169.0f;
	m_glyphWidth[13] = 180.0f;
	m_glyphWidth[14] = 188.0f;
	m_glyphWidth[15] = 202.0f;
	m_glyphWidth[16] = 220.0f;
	m_glyphWidth[17] = 235.0f;
	m_glyphWidth[18] = 251.0f;
	m_glyphWidth[19] = 266.0f;
	m_glyphWidth[20] = 284.0f;
	m_glyphWidth[21] = 299.0f;
	m_glyphWidth[22] = 315.0f;
	m_glyphWidth[23] = 330.0f;
	m_glyphWidth[24] = 347.0f;
	m_glyphWidth[25] = 364.0f;
	m_glyphWidth[26] = 371.0f;
	m_glyphWidth[27] = 381.0f;
	m_glyphWidth[28] = 397.0f;
	m_glyphWidth[29] = 413.0f;
	m_glyphWidth[30] = 429.0f;
	m_glyphWidth[31] = 444.0f;
	m_glyphWidth[32] = 472.0f;
	m_glyphWidth[33] = 493.0f;
	m_glyphWidth[34] = 510.0f;
	m_glyphWidth[35] = 528.0f;
	m_glyphWidth[36] = 548.0f;
	m_glyphWidth[37] = 564.0f;
	m_glyphWidth[38] = 577.0f;
	m_glyphWidth[39] = 598.0f;
	m_glyphWidth[40] = 618.0f;
	m_glyphWidth[41] = 625.0f;
	m_glyphWidth[42] = 638.0f;
	m_glyphWidth[43] = 655.0f;
	m_glyphWidth[44] = 669.0f;
	m_glyphWidth[45] = 697.0f;
	m_glyphWidth[46] = 717.0f;
	m_glyphWidth[47] = 740.0f;
	m_glyphWidth[48] = 756.0f;
	m_glyphWidth[49] = 779.0f;
	m_glyphWidth[50] = 796.0f;
	m_glyphWidth[51] = 810.0f;
	m_glyphWidth[52] = 828.0f;
	m_glyphWidth[53] = 847.0f;
	m_glyphWidth[54] = 866.0f;
	m_glyphWidth[55] = 895.0f;
	m_glyphWidth[56] = 913.0f;
	m_glyphWidth[57] = 930.0f;
	m_glyphWidth[58] = 946.0f;
	m_glyphWidth[59] = 954.0f;
	m_glyphWidth[60] = 969.0f;
	m_glyphWidth[61] = 978.0f;
	m_glyphWidth[62] = 994.0f;
	m_glyphWidth[63] = 1011.0f;
	m_glyphWidth[64] = 1021.0f;
	m_glyphWidth[65] = 1037.0f;
	m_glyphWidth[66] = 1054.0f;
	m_glyphWidth[67] = 1067.0f;
	m_glyphWidth[68] = 1084.0f;
	m_glyphWidth[69] = 1099.0f;
	m_glyphWidth[70] = 1109.0f;
	m_glyphWidth[71] = 1125.0f;
	m_glyphWidth[72] = 1142.0f;
	m_glyphWidth[73] = 1149.0f;
	m_glyphWidth[74] = 1158.0f;
	m_glyphWidth[75] = 1173.0f;
	m_glyphWidth[76] = 1181.0f;
	m_glyphWidth[77] = 1207.0f;
	m_glyphWidth[78] = 1224.0f;
	m_glyphWidth[79] = 1241.0f;
	m_glyphWidth[80] = 1258.0f;
	m_glyphWidth[81] = 1275.0f;
	m_glyphWidth[82] = 1286.0f;
	m_glyphWidth[83] = 1297.0f;
	m_glyphWidth[84] = 1309.0f;
	m_glyphWidth[85] = 1325.0f;
	m_glyphWidth[86] = 1340.0f;
	m_glyphWidth[87] = 1364.0f;
	m_glyphWidth[88] = 1379.0f;
	m_glyphWidth[89] = 1395.0f;
	m_glyphWidth[90] = 1407.0f;
	m_glyphWidth[91] = 1418.0f;
	m_glyphWidth[92] = 1434.0f;
	m_glyphWidth[93] = 1444.0f;
	m_glyphWidth[94] = 1461.0f;
}