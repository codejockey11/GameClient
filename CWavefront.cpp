#include "CWavefront.h"

/*
*/
CWavefront::CWavefront()
{
	memset(this, 0x00, sizeof(CWavefront));
}

/*
*/
CWavefront::CWavefront(const char* name, const char* filename)
{
	memset(this, 0x00, sizeof(CWavefront));

	m_name = new CString(name);

	m_vertex = new CVec3f[CWavefront::MAX_VERTEX]();
	m_normal = new CVec3f[CWavefront::MAX_NORMAL]();

	m_u = new float[CWavefront::MAX_UV]();
	m_v = new float[CWavefront::MAX_UV]();

	for (int32_t i = 0; i < CWavefront::MAX_MATERIAL; i++)
	{
		m_meshvertex[i] = new CVertexNT[CWavefront::MAX_VERTEX]();
	}

	m_materials = new CShaderMaterial[CWavefront::MAX_MATERIAL]();

	m_objectScript.InitBuffer(filename);

	m_objectScript.MoveToToken("mtllib");
	m_objectScript.Move(6);

	m_mtllib = m_objectScript.GetToken();

	m_objectScript.SkipEndOfLine();
}

/*
*/
CWavefront::~CWavefront()
{
	SAFE_DELETE_ARRAY(m_materials);

	for (int32_t i = 0; i < CWavefront::MAX_MATERIAL; i++)
	{
		SAFE_DELETE_ARRAY(m_meshvertex[i]);
	}

	SAFE_DELETE_ARRAY(m_v);
	SAFE_DELETE_ARRAY(m_u);
	SAFE_DELETE_ARRAY(m_normal);
	SAFE_DELETE_ARRAY(m_vertex);

	SAFE_DELETE(m_name);
}

/*
*/
void CWavefront::InitializeMaterialScript(const char* filename)
{
	m_materialScript.InitBuffer(filename);
}

/*
*/
void CWavefront::Load()
{
	CWavefront::ProcessMaterialScript();
	CWavefront::ProcessObjectScript();

	m_isInitialized = true;
}

/*
*/
void CWavefront::ProcessMaterialScript()
{
	m_firstTime = true;

	while (!m_materialScript.CheckEndOfBuffer())
	{
		if (strncmp(m_materialScript.m_buffer, "Ns ", 3) != 0)
			if (strncmp(m_materialScript.m_buffer, "Ka ", 3) != 0)
				if (strncmp(m_materialScript.m_buffer, "Kd ", 3) != 0)
					if (strncmp(m_materialScript.m_buffer, "Ks ", 3) != 0)
						if (strncmp(m_materialScript.m_buffer, "Ke ", 3) != 0)
							if (strncmp(m_materialScript.m_buffer, "Ni ", 3) != 0)
								if (strncmp(m_materialScript.m_buffer, "d ", 2) != 0)
									if (strncmp(m_materialScript.m_buffer, "illum ", 6) != 0)
										if (strncmp(m_materialScript.m_buffer, "map_Kd ", 7) != 0)
											if (strncmp(m_materialScript.m_buffer, "map_Ks ", 7) != 0)
												if (strncmp(m_materialScript.m_buffer, "map_Ns ", 7) != 0)
													if (strncmp(m_materialScript.m_buffer, "map_d ", 6) != 0)
														if (strncmp(m_materialScript.m_buffer, "map_bump ", 9) != 0)
															if (strncmp(m_materialScript.m_buffer, "map_Bump ", 9) != 0)
																if (strncmp(m_materialScript.m_buffer, "newmtl ", 7) != 0)
																{
																	m_materialScript.SkipEndOfLine();

																	continue;
																}

		if (strncmp(m_materialScript.m_buffer, "newmtl ", 7) == 0)
		{
			if (!m_firstTime)
			{
				m_materialCount++;
			}

			m_firstTime = false;

			m_materialScript.Move(7);

			m_materials[m_materialCount].SetName(m_materialScript.GetToken());
			m_materials[m_materialCount].SetNumber(m_materialCount);

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Ns ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_materials[m_materialCount].SetNs(strtof(m_materialScript.GetToken(), nullptr));

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Ka ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_tokenr = new CString(m_materialScript.GetToken());
			m_tokeng = new CString(m_materialScript.GetToken());
			m_tokenb = new CString(m_materialScript.GetToken());

			m_materials[m_materialCount].SetKa(strtof(m_tokenr->m_text, nullptr), strtof(m_tokeng->m_text, nullptr), strtof(m_tokenb->m_text, nullptr), 1.0f);

			SAFE_DELETE(m_tokenr);
			SAFE_DELETE(m_tokeng);
			SAFE_DELETE(m_tokenb);

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Kd ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_tokenr = new CString(m_materialScript.GetToken());
			m_tokeng = new CString(m_materialScript.GetToken());
			m_tokenb = new CString(m_materialScript.GetToken());

			m_materials[m_materialCount].SetKd(strtof(m_tokenr->m_text, nullptr), strtof(m_tokeng->m_text, nullptr), strtof(m_tokenb->m_text, nullptr), 1.0f);

			SAFE_DELETE(m_tokenr);
			SAFE_DELETE(m_tokeng);
			SAFE_DELETE(m_tokenb);

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Ks ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_tokenr = new CString(m_materialScript.GetToken());
			m_tokeng = new CString(m_materialScript.GetToken());
			m_tokenb = new CString(m_materialScript.GetToken());

			m_materials[m_materialCount].SetKs(strtof(m_tokenr->m_text, nullptr), strtof(m_tokeng->m_text, nullptr), strtof(m_tokenb->m_text, nullptr), 1.0f);

			SAFE_DELETE(m_tokenr);
			SAFE_DELETE(m_tokeng);
			SAFE_DELETE(m_tokenb);

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Ke ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_tokenr = new CString(m_materialScript.GetToken());
			m_tokeng = new CString(m_materialScript.GetToken());
			m_tokenb = new CString(m_materialScript.GetToken());

			m_materials[m_materialCount].SetKe(strtof(m_tokenr->m_text, nullptr), strtof(m_tokeng->m_text, nullptr), strtof(m_tokenb->m_text, nullptr), 1.0f);

			SAFE_DELETE(m_tokenr);
			SAFE_DELETE(m_tokeng);
			SAFE_DELETE(m_tokenb);

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "Ni ", 3) == 0)
		{
			m_materialScript.Move(3);

			m_materials[m_materialCount].SetNi(strtof(m_materialScript.GetToken(), nullptr));

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "d ", 2) == 0)
		{
			m_materialScript.Move(2);

			m_materials[m_materialCount].Setd(strtof(m_materialScript.GetToken(), nullptr));

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "illum ", 6) == 0)
		{
			m_materialScript.Move(6);

			m_materials[m_materialCount].SetIllum(atoi(m_materialScript.GetToken()));

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "map_Kd ", 7) == 0)
		{
			m_materialScript.Move(7);

			m_materials[m_materialCount].SetMapKd(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "map_Ka ", 7) == 0)
		{
			m_materialScript.Move(7);

			m_materials[m_materialCount].SetMapKa(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "map_Ks ", 7) == 0)
		{
			m_materialScript.Move(7);

			m_materials[m_materialCount].SetMapKs(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "map_Ns ", 7) == 0)
		{
			m_materialScript.Move(7);

			m_materials[m_materialCount].SetMapNs(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}

		if (strncmp(m_materialScript.m_buffer, "map_d ", 6) == 0)
		{
			m_materialScript.Move(6);

			m_materials[m_materialCount].SetMapD(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}

		if ((strncmp(m_materialScript.m_buffer, "map_bump ", 9) == 0) || (strncmp(m_materialScript.m_buffer, "map_Bump ", 9) == 0))
		{
			m_materialScript.Move(9);

			m_materials[m_materialCount].SetMapBump(m_materialScript.GetToken());

			m_materialScript.SkipEndOfLine();
		}
	}

	m_materialCount++;
}

/*
*/
void CWavefront::ProcessObjectScript()
{
	while (!m_objectScript.CheckEndOfBuffer())
	{
		if (strncmp(m_objectScript.m_buffer, "v ", 2) != 0)
			if (strncmp(m_objectScript.m_buffer, "vn ", 3) != 0)
				if (strncmp(m_objectScript.m_buffer, "vt ", 3) != 0)
					if (strncmp(m_objectScript.m_buffer, "f ", 2) != 0)
						if (strncmp(m_objectScript.m_buffer, "usemtl ", 7) != 0)
						{
							m_objectScript.SkipEndOfLine();

							continue;
						}

		if (strncmp(m_objectScript.m_buffer, "v ", 2) == 0)
		{
			while (strncmp(m_objectScript.m_buffer, "v ", 2) == 0)
			{
				m_objectScript.Move(2);

				m_vertex[m_vertexCount].m_p.x = strtof(m_objectScript.GetToken(), nullptr);
				m_vertex[m_vertexCount].m_p.y = strtof(m_objectScript.GetToken(), nullptr);
				m_vertex[m_vertexCount].m_p.z = strtof(m_objectScript.GetToken(), nullptr);

				m_vertexCount++;

				m_objectScript.SkipEndOfLine();
			}
		}

		if (strncmp(m_objectScript.m_buffer, "vn ", 3) == 0)
		{
			while (strncmp(m_objectScript.m_buffer, "vn ", 3) == 0)
			{
				m_objectScript.Move(3);

				m_normal[m_normalCount].m_p.x = strtof(m_objectScript.GetToken(), nullptr);
				m_normal[m_normalCount].m_p.y = strtof(m_objectScript.GetToken(), nullptr);
				m_normal[m_normalCount].m_p.z = strtof(m_objectScript.GetToken(), nullptr);

				m_normalCount++;

				m_objectScript.SkipEndOfLine();
			}
		}

		if (strncmp(m_objectScript.m_buffer, "vt ", 3) == 0)
		{
			while (strncmp(m_objectScript.m_buffer, "vt ", 3) == 0)
			{
				m_objectScript.Move(3);

				m_u[m_uvCount] = strtof(m_objectScript.GetToken(), nullptr);
				m_v[m_uvCount] = strtof(m_objectScript.GetToken(), nullptr);

				m_uvCount++;

				m_objectScript.SkipEndOfLine();
			}
		}

		m_faceCount = 0;

		if (strncmp(m_objectScript.m_buffer, "usemtl ", 7) == 0)
		{
			m_objectScript.Move(7);

			char* material = m_objectScript.GetToken();

			CShaderMaterial* mat = &m_materials[0];

			for (int32_t m = 1; m < m_materialCount; m++)
			{
				if (strncmp(material, m_materials[m].m_name, strlen(m_materials[m].m_name)) == 0)
				{
					mat = &m_materials[m];

					break;
				}
			}

			m_objectScript.SkipEndOfLine();

			int32_t vertex[3] = {};
			int32_t normal[3] = {};
			int32_t uv[3] = {};

			while (true)
			{
				if ((m_objectScript.CheckEndOfBuffer()) || (strncmp(m_objectScript.m_buffer, "v ", 2) == 0) || (strncmp(m_objectScript.m_buffer, "usemtl", 6) == 0))
				{
					break;
				}

				if (strncmp(m_objectScript.m_buffer, "s ", 2) == 0)
				{
					m_objectScript.SkipEndOfLine();

					continue;
				}

				if (strncmp(m_objectScript.m_buffer, "f ", 2) == 0)
				{
					m_objectScript.Move(2);

					sscanf_s(m_objectScript.m_buffer, "%i/%i/%i %i/%i/%i %i/%i/%i\n", &vertex[0], &uv[0], &normal[0], &vertex[1], &uv[1], &normal[1], &vertex[2], &uv[2], &normal[2]);

					for (int32_t p = 0; p < 3; p++)
					{
						m_meshvertex[mat->m_number][m_faceCount].m_p.x = m_vertex[vertex[p] - 1].m_p.x;
						m_meshvertex[mat->m_number][m_faceCount].m_p.y = m_vertex[vertex[p] - 1].m_p.y;
						m_meshvertex[mat->m_number][m_faceCount].m_p.z = m_vertex[vertex[p] - 1].m_p.z;

						m_meshvertex[mat->m_number][m_faceCount].m_n.x = m_normal[normal[p] - 1].m_p.x;
						m_meshvertex[mat->m_number][m_faceCount].m_n.y = m_normal[normal[p] - 1].m_p.y;
						m_meshvertex[mat->m_number][m_faceCount].m_n.z = m_normal[normal[p] - 1].m_p.z;

						m_meshvertex[mat->m_number][m_faceCount].m_uv.x = m_u[uv[p] - 1];
						m_meshvertex[mat->m_number][m_faceCount].m_uv.y = m_v[uv[p] - 1];

						m_faceCount++;

						m_meshvertexCount[mat->m_number] = m_faceCount;
					}

					m_objectScript.SkipEndOfLine();
				}
			}
		}
	}
}