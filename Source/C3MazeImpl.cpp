// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3MazeImpl.h>
#include <C3CommonVertexDefs.h>
#include <TRaster.h>
#include <C3MeshImpl.h>
#include <C3ModelImpl.h>
#include <C3BlobImpl.h>

using namespace c3;


bool MazeDescription::Load(tinyxml2::XMLDocument *pdoc)
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
		if (!_stricmp(pa->Value(), "ceiling"))
		{
			st = SurfaceType::CEILING;
		}
		else if (!_stricmp(pa->Value(), "east"))
		{
			st = SurfaceType::EAST;
		}
		else if (!_stricmp(pa->Value(), "floor"))
		{
			st = SurfaceType::FLOOR;
		}
		else if (!_stricmp(pa->Value(), "north"))
		{
			st = SurfaceType::NORTH;
		}
		else if (!_stricmp(pa->Value(), "south"))
		{
			st = SurfaceType::SOUTH;
		}
		else if (!_stricmp(pa->Value(), "west"))
		{
			st = SurfaceType::WEST;
		}

		bool ret = true;

		while ((pel = pel->FirstChildElement("variant")) != nullptr)
		{
			ret &= LoadSurfaceVariant(pel, st);
		}

		return ret;
	};

	tinyxml2::XMLElement *pel = pdoc->FirstChildElement("c3pg:maze");
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

	if ((pa = pel->FindAttribute("minwallz")) != nullptr)
		m_MinWallZ = pa->FloatValue();

	if ((pa = pel->FindAttribute("maxwallz")) != nullptr)
		m_MaxWallZ = pa->FloatValue();

	if ((pa = pel->FindAttribute("floorz")) != nullptr)
		m_FloorZ = pa->FloatValue();

	if ((pa = pel->FindAttribute("ceilingz")) != nullptr)
		m_CeilingZ = pa->FloatValue();

	pel = pel->FirstChildElement("surface");
	while (pel)
	{
		ret &= LoadSurfaceDescription(pel);
		pel = pel->NextSiblingElement("surface");
	}

	return ret;
}


void LoadU8Image(ResourceManager *rm, const TCHAR *filename, U8Raster &img)
{
	Resource *hr = rm->GetResource(filename, RESF_DEMANDLOAD, RESOURCETYPE(Blob));

	if (hr && (hr->GetStatus() == Resource::RS_LOADED))
	{
		c3::Blob *pblob = dynamic_cast<Blob *>((Blob *)hr->GetData());

		int w, h, c;
		if (stbi_info_from_memory(pblob->Data(), (int)pblob->Size(), &w, &h, &c) && w && h && (c == 1))
		{
			stbi_uc *src = stbi_load_from_memory(pblob->Data(), (int)pblob->Size(), &w, &h, &c, 0);
			if (src)
			{
				img.Resize(w, h);
				memcpy(img.m_Image.data(), src, img.m_Image.size());
				free(src);
			}
		}

		hr->DelRef();
	}
}


using ProcessMapFunc = std::function<void(int, int, MazeDescription::SurfaceType)>;

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

			if (!ident)
			{
				if (ident != prev)
					func(x, y, MazeDescription::SurfaceType::EAST);

				if (ident != pr[x + 1])
					func(x, y, MazeDescription::SurfaceType::WEST);

				func(x, y, MazeDescription::SurfaceType::FLOOR);
				func(x, y, MazeDescription::SurfaceType::CEILING);
			}

			prev = ident;
		}

		ident = pr[x];

		if (!ident)
		{
			if (ident != prev)
				func(x, y, MazeDescription::SurfaceType::EAST);

			if (ident != 0)
				func(x, y, MazeDescription::SurfaceType::WEST);

			func(x, y, MazeDescription::SurfaceType::FLOOR);
			func(x, y, MazeDescription::SurfaceType::CEILING);
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

			if (!ident)
			{
				if (ident != prev)
					func(x, y, MazeDescription::SurfaceType::NORTH);

				if (ident != pr[maxx])
					func(x, y, MazeDescription::SurfaceType::SOUTH);
			}

			prev = ident;
			pr += maxx;
		}

		ident = *pr;

		if (!ident)
		{
			if (ident != prev)
				func(x, y, MazeDescription::SurfaceType::NORTH);

			if (ident != 0)
				func(x, y, MazeDescription::SurfaceType::SOUTH);
		}
	}
}

Model *MazeDescription::GenerateMaze(System *psys)
{
	assert(psys);

	Model *pmod = nullptr;

	Renderer *prend = psys->GetRenderer();
	ResourceManager *rm = psys->GetResourceManager();

	U8Raster raw_map;

	if (!m_MapFile.has_value())
		return nullptr;

	LoadU8Image(rm, m_MapFile->c_str(), raw_map);

	// We're going to build something that looks like this:
	/*

	        ____     ____
	       /   /|   /   /|
          /   / |__/___/ |
         /   /  /  |   | /
        /   /  /   |___|/
	   /   /__/_____   /
	  /            /| /
     /____________/ |/
     |            | /
     |____________|/

	*/

	uint32_t wallct[SurfaceType::NUM_SURFTYPES] = { 0 };

	ProcessMapHorizontal(raw_map, [&wallct](int x, int y, SurfaceType s) { wallct[s]++; });
	ProcessMapVertical(raw_map, [&wallct](int x, int y, SurfaceType s) { wallct[s]++; });

	glm::fvec3 pos_ofs[SurfaceType::NUM_SURFTYPES][4] =
	{
		{{0, 0, m_FloorZ},   {0, 1, m_FloorZ},   {1, 1, m_FloorZ},   {1, 0, m_FloorZ}},		// FLOOR
		{{0, 0, m_CeilingZ}, {1, 0, m_CeilingZ}, {1, 1, m_CeilingZ}, {0, 1, m_CeilingZ}},	// CEILING
		{{0, 0, m_MinWallZ}, {1, 0, m_MinWallZ}, {1, 0, m_MaxWallZ}, {0, 0, m_MaxWallZ}},	// NORTH
		{{0, 1, m_MinWallZ}, {0, 1, m_MaxWallZ}, {1, 1, m_MaxWallZ}, {1, 1, m_MinWallZ}},	// SOUTH
		{{0, 0, m_MinWallZ}, {0, 0, m_MaxWallZ}, {0, 1, m_MaxWallZ}, {0, 1, m_MinWallZ}},	// EAST
		{{1, 0, m_MinWallZ}, {1, 1, m_MinWallZ}, {1, 1, m_MaxWallZ}, {1, 0, m_MaxWallZ}}	// WEST
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

	std::function<void(float, float, SurfaceType, c3::Vertex::PNYT1::s *)> BuildWall = [this, &pos_ofs, &norm, &uvs](float x, float y, SurfaceType s, c3::Vertex::PNYT1::s *v)
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

			if ((w == SurfaceType::NORTH) || (w == SurfaceType::SOUTH))
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
				pibd[k++] = j + 2;
				pibd[k++] = j + 1;

				pibd[k++] = j;
				pibd[k++] = j + 3;
				pibd[k++] = j + 2;
			}

			pib->Unlock();
		}

		Mesh *pmesh = prend->CreateMesh();
		if (pmesh)
		{
			pmesh->AttachVertexBuffer(pvb);
			pmesh->AttachIndexBuffer(pib);

			glm::fvec3 vmin(0, 0, std::min<float>(std::min<float>(m_MinWallZ, m_MaxWallZ), std::min<float>(m_FloorZ, m_CeilingZ)));
			glm::fvec3 vmax(raw_map.GetWidth(), raw_map.GetHeight(), std::max<float>(std::max<float>(m_MinWallZ, m_MaxWallZ), std::max<float>(m_FloorZ, m_CeilingZ)));
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


