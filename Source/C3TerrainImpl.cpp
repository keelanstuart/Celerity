// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3ModelImpl.h>
#include <C3Resource.h>
#include <C3MaterialImpl.h>
#include <C3MeshImpl.h>
#include <C3CommonVertexDefs.h>
#include <C3Math.h>
#include <C3TerrainImpl.h>


using namespace c3;

using TerrainSampleFunc = std::function<float(const void *h, Texture2D::SLockInfo *li, props::TVec2I pos)>;

TerrainSampleFunc SampleF32_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I pos)
{
	assert(h);
	assert(pos.x < li->width);
	assert(pos.y < li->height);

	return ((float *)h)[pos.y * li->width + pos.x];
};

TerrainSampleFunc SampleS8_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I pos)
{
	assert(h);
	assert(pos.x < li->width);
	assert(pos.y < li->height);

	return (float)((int8_t *)h)[pos.y * li->width + pos.x] * 0.5f;
};

TerrainSampleFunc SampleU8_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I pos)
{
	assert(h);
	assert(pos.x < li->width);
	assert(pos.y < li->height);

	return (float)((uint8_t *)h)[pos.y * li->width + pos.x] * 0.5f;

};

TerrainSampleFunc SampleU8_2CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I pos)
{
	assert(h);
	assert(pos.x < li->width);
	assert(pos.y < li->height);

	return (float)((uint16_t *)h)[pos.y * li->width + pos.x] * 0.5f;
};



Mesh *Terrain::CreateTerrainChunk(System *psys, props::TVec2I dim, props::TVec2I ofs, Texture2D *pheights)
{
	assert(psys);
	if ((dim.x < 2) || (dim.y < 2))
		return nullptr;

	Renderer *rend = psys->GetRenderer();

	TerrainSampleFunc samp = nullptr;
	switch (pheights->Format())
	{
		case Renderer::TextureType::F16_1CH:
			break;

		case Renderer::TextureType::F32_1CH:
			samp = SampleF32_1CH;
			break;

		case Renderer::TextureType::S8_1CH:
			samp = SampleS8_1CH;
			break;

		case Renderer::TextureType::U8_1CH:
			samp = SampleU8_1CH;
			break;

		case Renderer::TextureType::U8_2CH:
			samp = SampleU8_2CH;
			break;
	}

	if (!samp)
		return nullptr;

	// We're going to build a lattice that looks like this:
	/*

	@---------@---------@---------@
	|\       /|\       /|\       /|
	| \     / | \     / | \     / |
	|  \   /  |  \   /  |  \   /  |
	|   \ /   |   \ /   |   \ /   |
	|    @    |    @    |    @    |
	|   / \   |   / \   |   / \   |
	|  /   \  |  /   \  |  /   \  |
	| /     \ | /     \ | /     \ |
	|/       \|/       \|/       \|
	@---------@---------@---------@
	|\       /|\       /|\       /|
	| \     / | \     / | \     / |
	|  \   /  |  \   /  |  \   /  |
	|   \ /   |   \ /   |   \ /   |
	|    @    |    @    |    @    |
	|   / \   |   / \   |   / \   |
	|  /   \  |  /   \  |  /   \  |
	| /     \ | /     \ | /     \ |
	|/       \|/       \|/       \|
	@---------@---------@---------@

	*/

	size_t facecount = 4 * (dim.x - 1) * (dim.y - 1);

	size_t vcount = (dim.x * dim.y) + ((dim.x - 1) * (dim.y - 1));
	glm::fvec2 vdimf((float)dim.x, (float)dim.y);
	glm::fvec2 texinc = 1.0f / vdimf;
	glm::fvec2 texinc_half = texinc / 2.0f;

	VertexBuffer *pvb = rend->CreateVertexBuffer();

	void *buffer;

	if (pvb && (pvb->Lock(&buffer, vcount, c3::Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK))
	{
		c3::Vertex::PNYT1::s *v = (c3::Vertex::PNYT1::s *)buffer;

		float ys = 0;
		float tv = 0;

		for (int y = 0; y < dim.y; y++)
		{
			float xs = 0;
			float tu = 0;

			for (int xo = 0; xo < dim.x; xo++, v++)
			{
				v->pos.x = xs;
				v->pos.y = ys;
				v->pos.z = 0;

				v->norm.x = 0;
				v->norm.y = 0;
				v->norm.z = 1;

				v->binorm.x = 0;
				v->binorm.y = 1;
				v->binorm.z = 0;

				v->tang.x = 1;
				v->tang.y = 0;
				v->tang.z = 0;

				v->uv.x = tu;
				v->uv.y = tv;

				xs += 1;
				tu += texinc.x;
			}

			ys += 0.5f;
			tv += texinc_half.y;

			xs += 0.5f;
			tu = 0;

			if (y < (dim.y - 1))
			{
				for (int xi = 0; xi < (dim.x - 1); xi++, v++)
				{
					v->pos.x = xs;
					v->pos.y = ys;
					v->pos.z = 0;

					v->norm.x = 0;
					v->norm.y = 0;
					v->norm.z = 0.5;

					v->binorm.x = 0;
					v->binorm.y = 0.5;
					v->binorm.z = 0;

					v->tang.x = 1;
					v->tang.y = 0;
					v->tang.z = 0;

					v->uv.x = tu;
					v->uv.y = tv;

					xs += 1;
					tu += texinc.x;
				}
			}

			ys += 0.5f;
			tv += texinc_half.y;
		}

		pvb->Unlock();
	}

	IndexBuffer *pib = rend->CreateIndexBuffer();

	uint16_t *i = nullptr;

	if (pib->Lock((void **)&i, facecount * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
	{
		size_t starty = 0, startx = 0;

		uint16_t ir0 = 0;
		uint16_t mid = (uint16_t)dim.x;
		uint16_t ir1 = mid + (uint16_t)dim.x - 1;

		// Build our indices...
		for (int64_t y = 0, maxy = (dim.y - 1); y < maxy; y++)
		{
			uint16_t _ir0 = ir0;
			uint16_t _mid = mid;
			uint16_t _ir1 = ir1;

			for (int64_t x = 0, maxx = (dim.x - 1); x < maxx; x++)
			{
				// Face 0 - top
				*(i++) = _ir0;
				*(i++) = _mid;
				*(i++) = _ir0 + 1;

				// Face 1 - left
				*(i++) = _ir0;
				*(i++) = _ir1;
				*(i++) = _mid;

				// Face 2 - bottom
				*(i++) = _ir1;
				*(i++) = _ir1 + 1;
				*(i++) = _mid;

				// Face 3 - right
				*(i++) = _mid;
				*(i++) = _ir1 + 1;
				*(i++) = _ir0 + 1;

				_ir0++;
				_mid++;
				_ir1++;
			}

			ir0 = ir1;
			mid = _ir1 + 1;
			ir1 = mid + (uint16_t)dim.x - 1;
		}

		pib->Unlock();
	}

	
}


Model *Terrain::CreateTerrainModel(System *psys, const tinyxml2::XMLNode *terrain_defs_root)
{
	tinyxml2::XMLElement *el;
	
	el = terrain_defs_root->FirstChildElement("heightmap");
	if (!el)
		return null;

	Renderer *rend = psys->GetRenderer();

	Model *ret = Model::Create(rend);

	for (size_t ym = 0; ym < dim.y; ym++)
	{
		for (size_t xm = 0; xm < dim.x; xm++)
		{
			Mesh *pmesh = rend->CreateMesh();
			pmesh->AttachVertexBuffer(pvb);
			pmesh->AttachIndexBuffer(pib);

			Model::MeshIndex mi = pmod->AddMesh(pmesh);
			Model::NodeIndex ni = pmod->AddNode();
			pmod->AssignMeshToNode(ni, mi);

			pmod->SetMaterial(mi, rend->GetWhiteMaterial());
			glm::fmat4x4 mat = glm::translate(glm::fvec3((float)xm * (float)dim.x, (float)ym * (float)dim.y, 0));
			pmod->SetTransform(ni, &mat);
		}
	}
}