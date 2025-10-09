// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3ExtrusionImpl.h>
#include <C3CommonVertexDefs.h>
#include <TRaster.h>
#include <C3MeshImpl.h>
#include <C3ModelImpl.h>
#include <C3BlobImpl.h>

using namespace c3;


bool ExtrusionDescription::Load(tinyxml2::XMLDocument *pdoc)
{
	if (!pdoc || pdoc->Error())
		return false;

	auto LoadTexture = [](tinyxml2::XMLElement *pel, std::optional<tstring> *texfn) -> bool
	{
		if (!pel)
			return false;

		TCHAR *tmp;
		CONVERT_MBCS2TCS(pel->GetText(), tmp);
		*texfn = tmp;

		return true;
	};

	auto LoadSurfaceVariant = [&](tinyxml2::XMLElement *pel, SurfaceType t) -> bool
	{
		if (!pel)
			return false;

		bool ret = true;

		SurfaceDescription &desc = m_SurfDesc[t].emplace_back();

		const tinyxml2::XMLAttribute *pa;

		pa = pel->FindAttribute("opacity");
		desc.m_Opacity = pa ? pa->FloatValue() : 1.0f;

		pa = pel->FindAttribute("tiling.u");
		desc.m_Tiling.x = pa ? pa->FloatValue() : 1.0f;

		pa = pel->FindAttribute("tiling.v");
		desc.m_Tiling.y = pa ? pa->FloatValue() : 1.0f;

		// only require the diffuse layer
		ret &= LoadTexture(pel->FirstChildElement("tex.diffuse"), &desc.m_TexFile[Material::TCT_DIFFUSE]);
		LoadTexture(pel->FirstChildElement("tex.normal"), &desc.m_TexFile[Material::TCT_NORMAL]);
		LoadTexture(pel->FirstChildElement("tex.emissive"), &desc.m_TexFile[Material::TCT_EMISSIVE]);
		LoadTexture(pel->FirstChildElement("tex.surfdesc"), &desc.m_TexFile[Material::TCT_SURFACEDESC]);

		return ret;
	};

	auto LoadSurfaceDescription = [&LoadSurfaceVariant](tinyxml2::XMLElement *pel) -> bool
	{
		if (!pel)
			return false;

		const tinyxml2::XMLAttribute *pa = pel->FindAttribute("dir");
		if (!pa)
			return false;

		SurfaceType st;
		if (!_stricmp(pa->Value(), "z+"))
		{
			st = SurfaceType::POSZ;
		}
		else if (!_stricmp(pa->Value(), "x+"))
		{
			st = SurfaceType::POSX;
		}
		else if (!_stricmp(pa->Value(), "z-"))
		{
			st = SurfaceType::NEGZ;
		}
		else if (!_stricmp(pa->Value(), "y+"))
		{
			st = SurfaceType::POSY;
		}
		else if (!_stricmp(pa->Value(), "y-"))
		{
			st = SurfaceType::NEGY;
		}
		else if (!_stricmp(pa->Value(), "x-"))
		{
			st = SurfaceType::NEGX;
		}

		bool ret = true;

		while ((pel = pel->FirstChildElement("variant")) != nullptr)
		{
			ret &= LoadSurfaceVariant(pel, st);
		}

		return ret;
	};

	tinyxml2::XMLElement *pel = pdoc->FirstChildElement("c3pg:extrusion");
	bool ret = pel != nullptr;

	TCHAR *tmp;
	const tinyxml2::XMLAttribute *pa;

	pa = pel->FindAttribute("name");
	if (pa)
	{
		CONVERT_MBCS2TCS(pa->Value(), tmp);
		m_Name = tmp;
	}

	pa = pel->FindAttribute("map");
	if (pa)
	{
		CONVERT_MBCS2TCS(pa->Value(), tmp);
		m_MapFile = tmp;
	}

	pel = pel->FirstChildElement("surface");
	while (pel)
	{
		ret &= LoadSurfaceDescription(pel);
		pel = pel->NextSiblingElement("surface");
	}

	return ret;
}


using ProcessMapFunc = std::function<void(int, int, ExtrusionDescription::SurfaceType)>;

void ProcessMapHorizontal(U8Raster &raw_map, ProcessMapFunc func)
{
	// count up all the horizontal changes
	for (int y = 0, maxy = raw_map.m_Dim.y; y < maxy; y++)
	{
		uint8_t *pr = raw_map.GetRow(y);
		if (!pr)
			continue;

		uint8_t prev = 0;

		uint8_t ident;
		int x = 0;

		for (int maxx = raw_map.m_Dim.x - 1; x < maxx; x++)
		{
			ident = pr[x];

			if (ident)
			{
				if (ident != prev)
					func(x, y, ExtrusionDescription::SurfaceType::POSX);

				if (ident != pr[x + 1])
					func(x, y, ExtrusionDescription::SurfaceType::NEGX);

				func(x, y, ExtrusionDescription::SurfaceType::POSZ);
				func(x, y, ExtrusionDescription::SurfaceType::NEGZ);
			}

			prev = ident;
		}

		ident = pr[x];

		if (ident)
		{
			if (ident != prev)
				func(x, y, ExtrusionDescription::SurfaceType::POSX);

			if (ident != 0)
				func(x, y, ExtrusionDescription::SurfaceType::NEGX);

			func(x, y, ExtrusionDescription::SurfaceType::POSZ);
			func(x, y, ExtrusionDescription::SurfaceType::NEGZ);
		}
	}
}

void ProcessMapVertical(U8Raster &raw_map, ProcessMapFunc func)
{
	for (int x = 0, maxx = raw_map.m_Dim.x; x < maxx; x++)
	{
		uint8_t *pr = &(raw_map.m_Image[x]);
		if (!pr)
			continue;

		uint8_t prev = 0;

		uint8_t ident;
		int y = 0;

		for (int maxy = raw_map.m_Dim.y - 1; y < maxy; y++)
		{
			ident = *pr;

			if (ident)
			{
				if (ident != prev)
					func(x, y, ExtrusionDescription::SurfaceType::POSY);

				if (ident != pr[maxx])
					func(x, y, ExtrusionDescription::SurfaceType::NEGY);
			}

			prev = ident;
			pr += maxx;
		}

		ident = *pr;

		if (ident)
		{
			if (ident != prev)
				func(x, y, ExtrusionDescription::SurfaceType::POSY);

			if (ident != 0)
				func(x, y, ExtrusionDescription::SurfaceType::NEGY);
		}
	}
}

Model *ExtrusionDescription::GenerateExtrusion(System *psys)
{
	assert(psys);

	Model *pmod = nullptr;

	Renderer *prend = psys->GetRenderer();
	ResourceManager *rm = psys->GetResourceManager();

	U8Raster raw_map;

	if (!m_MapFile.has_value())
		return nullptr;

	util::LoadU8Image(rm, m_MapFile->c_str(), raw_map);

	// We're going to build something that looks like this:
	/*

	                 ____
	                /   /|
                   /___/ |
         ____      |   | /
        /   /|     |___|/
	   /   /________   
	  /            /|
     /____________/ |
     |            | /
     |____________|/

	*/

	uint32_t wallct[SurfaceType::NUM_SURFTYPES] = { 0 };

	ProcessMapHorizontal(raw_map, [&wallct](int x, int y, SurfaceType s) { wallct[s]++; });
	ProcessMapVertical(raw_map, [&wallct](int x, int y, SurfaceType s) { wallct[s]++; });

	glm::fvec3 pos_ofs[SurfaceType::NUM_SURFTYPES][4] =
	{
		{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}},	// FLOOR
		{{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},	// CEILING
		{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},	// NORTH
		{{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},	// SOUTH
		{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},	// EAST
		{{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}}	// WEST
	};

	glm::fvec3 norm[SurfaceType::NUM_SURFTYPES][3] = // norm, tang, binorm
	{
		{ {  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 } },		// FLOOR
		{ {  0,  0, -1 }, {  0,  1,  0 }, {  1,  0,  0 } },		// CEILING
		{ {  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 } },		// NORTH
		{ {  0,  1,  0 }, {  0,  0,  1 }, {  1,  0,  0 } },		// SOUTH
		{ { -1,  0,  0 }, {  0,  1,  0 }, {  0,  0,  1 } },		// EAST
		{ {  1,  0,  0 }, {  0,  0,  1 }, {  0,  1,  0 } }		// WEST
	};

	glm::fvec2 uvs[SurfaceType::NUM_SURFTYPES][4] =
	{
		{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } },		// FLOOR
		{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } },		// CEILING
		{ { 1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 } },		// NORTH
		{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } },		// SOUTH
		{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } },		// EAST
		{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } }		// WEST
	};

	std::function<void(float, float, SurfaceType, c3::Vertex::PNYT1::s *)> BuildWall = [this, pos_ofs, norm, uvs](float x, float y, SurfaceType s, c3::Vertex::PNYT1::s *v)
	{
		for (size_t i = 0; i < 4; i++, v++)
		{
			v->pos.x = (float)x + pos_ofs[s][i].x;
			v->pos.y = (float)y + pos_ofs[s][i].y;
			v->pos.z = pos_ofs[s][i].z;

			v->norm = norm[s][0];
			v->tang = norm[s][1];
			v->binorm = norm[s][2];

			v->uv = (glm::fvec2(x, y) * m_SurfDesc[s][0].m_Tiling) + (uvs[s][i] * m_SurfDesc[s][0].m_Tiling);
		}
	};

	pmod = Model::Create(prend);

	for (size_t w = 0; w < SurfaceType::NUM_SURFTYPES; w++)
	{
		// if we have no surface description or we didn't count any walls for this surface type, go on...
		if (m_SurfDesc[w].empty() || !wallct[w])
			continue;

		VertexBuffer *pvb = prend->CreateVertexBuffer();
		void *buffer;

		uint32_t vcount = wallct[w] * 4;

		if (pvb && (pvb->Lock(&buffer, vcount, c3::Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK))
		{
			c3::Vertex::PNYT1::s *v = (c3::Vertex::PNYT1::s *)buffer;

			if ((w == SurfaceType::POSY) || (w == SurfaceType::NEGY))
			{
				ProcessMapVertical(raw_map, [&v, BuildWall, w](int x, int y, SurfaceType s)
				{
					if ((size_t)s == w)
					{
						BuildWall((float)x, (float)y, s, v);
						v += 4;
					}
				});
			}
			else
			{
				ProcessMapHorizontal(raw_map, [&v, BuildWall, w](int x, int y, SurfaceType s)
				{
					if ((size_t)s == w)
					{
						BuildWall((float)x, (float)y, s, v);
						v += 4;
					}
				});
			}

			pvb->Unlock();
		}

		IndexBuffer *pib = prend->CreateIndexBuffer();

		uint32_t numtris = vcount / 2;
		uint32_t numquads = numtris / 2;

		uint32_t *pibd = nullptr;
		if (pib->Lock((void **)&pibd, numtris * 3, IndexBuffer::IndexSize::IS_32BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
		{
			for (uint32_t i = 0, j = 0, k = 0; i < numquads; i++, j += 4)
			{
				pibd[k++] = j;
				pibd[k++] = j + 1;
				pibd[k++] = j + 2;

				pibd[k++] = j;
				pibd[k++] = j + 2;
				pibd[k++] = j + 3;
			}

			pib->Unlock();
		}

		Mesh *pmesh = prend->CreateMesh();
		if (pmesh)
		{
			pmesh->AttachVertexBuffer(pvb);
			pmesh->AttachIndexBuffer(pib);

			glm::fvec3 vmin(0, 0, std::min<float>(std::min<float>(0, 1), std::min<float>(0, 1)));
			glm::fvec3 vmax(raw_map.GetWidth(), raw_map.GetHeight(), std::max<float>(std::max<float>(0, 1), std::max<float>(0, 1)));
			((MeshImpl *)pmesh)->SetBounds(vmin, vmax);

			Model::MeshIndex mi = pmod->AddMesh(pmesh);
			Model::NodeIndex ni = pmod->AddNode();
			pmod->AssignMeshToNode(ni, mi);

			((ModelImpl *)pmod)->SetBounds(vmin, vmax);

			Material *pmtl = prend->GetMaterialManager()->CreateMaterial();
			if (pmtl)
			{
				SurfaceDescriptionArray &sda = m_SurfDesc.at(w);

				SurfaceDescription &sd = sda.at(0);

				for (size_t m = Material::TCT_DIFFUSE; m <= Material::TCT_EMISSIVE; m++)
				{
					if (sd.m_TexFile[m].has_value())
						pmtl->SetTexture((Material::TextureComponentType)m, rm->GetResource(sd.m_TexFile[m]->c_str()));
				}

				pmod->SetMaterial(mi, pmtl);
			}
		}
	}

	return pmod;
}


