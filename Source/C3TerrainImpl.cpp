// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3TerrainImpl.h>
#include <C3CommonVertexDefs.h>
#include <TRaster.h>
#include <C3MeshImpl.h>
#include <C3ModelImpl.h>

using namespace c3;

// returns a normal in .xyz and an elevation in .w
using TerrainSampleFunc = std::function<glm::fvec4(const void *h, const glm::uvec2 &hdim, glm::fvec2 &uvpos)>;

inline void ComputeSampleLocations(const glm::uvec2 &imgdim, glm::fvec2 uv, glm::fvec2 &interp, glm::uvec3 &xsloc, glm::uvec3 &ysloc)
{
	if ((uv.x < 0) || (uv.x > 1))
		uv.x -= floorf(uv.x);
	if ((uv.y < 0) || (uv.y > 1))
		uv.y -= floorf(uv.y);

	glm::fvec2 adj(uv.x * (imgdim.x - 3) + 1.0f, uv.y * (imgdim.y - 3) + 1.0f);

	interp.x = adj.x - floorf(adj.x);
	interp.y = adj.y - floorf(adj.y);

	xsloc[1] = (unsigned int)adj.x;
	xsloc[0] = xsloc[1] - 1;
	xsloc[2] = xsloc[1] + 1;

	ysloc[1] = (unsigned int)adj.y;
	ysloc[0] = ysloc[1] - 1;
	ysloc[2] = ysloc[1] + 1;

	assert(xsloc[2] < imgdim.x);
	assert(ysloc[2] < imgdim.y);
}



TerrainSampleFunc TerrainSampleU8_1CH = [](const void *img, const glm::uvec2 &imgdim, glm::fvec2 &uv)
{
	const uint8_t *data = static_cast<const uint8_t *>(img);
	const uint32_t W = imgdim.x;

	glm::uvec3 xsloc, ysloc; glm::fvec2 interp;
	ComputeSampleLocations(imgdim, uv, interp, xsloc, ysloc);

	float h[3][3];
	for (int ry = 0; ry < 3; ++ry)
		for (int cx = 0; cx < 3; ++cx)
			h[ry][cx] = float(data[ysloc[ry] * W + xsloc[cx]]); // 0..255

	const float u = interp.x, v = interp.y;

	// bilinear elevation inside the center cell
	const float h00 = h[1][1], h10 = h[1][2], h01 = h[2][1], h11 = h[2][2];
	const float center = std::lerp(std::lerp(h00, h10, u), std::lerp(h01, h11, u), v) / 255.0f;

	// centered finite differences (bytes per *pixel*)
	const float hL = std::lerp(h[1][0], h[2][0], v);
	const float hR = std::lerp(h[1][2], h[2][2], v);
	const float hT = std::lerp(h[0][1], h[0][2], u);
	const float hB = std::lerp(h[2][1], h[2][2], u);

	const float dx_bytes_per_px = 0.5f * (hR - hL);
	const float dy_bytes_per_px = 0.5f * (hB - hT);

	// pack derivatives in x/y, keep z=1 as a placeholder
	return glm::fvec4(dx_bytes_per_px, dy_bytes_per_px, 1.0f, center);
};


bool TerrainDescription::Load(tinyxml2::XMLDocument *pdoc)
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

	auto LoadLayer = [&](tinyxml2::XMLElement *pel) -> bool
	{
		if (!pel)
			return false;

		bool ret = true;

		LayerDescription desc;

		const tinyxml2::XMLAttribute *pa;

		pa = pel->FindAttribute("tex.opacity");
		if (pa)
		{
			TCHAR *tmp;
			CONVERT_MBCS2TCS(pa->Value(), tmp);
			desc.m_LayerOpacityFile = tmp;
		}

		pa = pel->FindAttribute("opacity");
		desc.m_Opacity = pa ? pa->FloatValue() : 1.0f;

		pa = pel->FindAttribute("tiling");
		desc.m_Tiling = pa ? pa->FloatValue() : 1.0f;

		// only require the diffuse layer
		ret &= LoadTexture(pel->FirstChildElement("tex.diffuse"), &desc.m_TexFile[Material::TCT_DIFFUSE]);
		LoadTexture(pel->FirstChildElement("tex.normal"), &desc.m_TexFile[Material::TCT_NORMAL]);
		LoadTexture(pel->FirstChildElement("tex.emissive"), &desc.m_TexFile[Material::TCT_EMISSIVE]);
		LoadTexture(pel->FirstChildElement("tex.surfdesc"), &desc.m_TexFile[Material::TCT_SURFACEDESC]);

		m_Layers.emplace_back(desc);

		return ret;
	};

	auto LoadLayers = [&LoadLayer](tinyxml2::XMLElement *pel) -> bool
	{
		if (!pel)
			return false;

		pel = pel->FirstChildElement("layer");
		bool ret = pel != nullptr;

		while (pel)
		{
			ret &= LoadLayer(pel);
			pel = pel->NextSiblingElement("layer");
		}

		return ret;
	};

	tinyxml2::XMLElement *pel = pdoc->FirstChildElement("c3pg:terrain");
	bool ret = pel != nullptr;

	TCHAR *tmp;
	const tinyxml2::XMLAttribute *pa;

	pa = pel->FindAttribute("name");
	if (pa)
	{
		CONVERT_MBCS2TCS(pa->Value(), tmp);
		m_Name = tmp;
	}

	pa = pel->FindAttribute("heightmap");
	if (pa)
	{
		CONVERT_MBCS2TCS(pa->Value(), tmp);
		m_HeightmapFile = tmp;
	}

	if ((pa = pel->FindAttribute("xdim")) != nullptr)
		m_Dim.x = pa->IntValue();

	if ((pa = pel->FindAttribute("ydim")) != nullptr)
		m_Dim.y = pa->IntValue();

	if ((pa = pel->FindAttribute("xdimtex")) != nullptr)
		m_TexDim.x = pa->IntValue();

	if ((pa = pel->FindAttribute("ydimtex")) != nullptr)
		m_TexDim.y = pa->IntValue();

	if ((pa = pel->FindAttribute("minz")) != nullptr)
		m_MinZ = pa->FloatValue();
	
	if ((pa = pel->FindAttribute("maxz")) != nullptr)
		m_MaxZ = pa->FloatValue();

	if ((pa = pel->FindAttribute("sampledist")) != nullptr)
		m_UnitsBetweenSamples = pa->FloatValue();

	while (pel)
	{
		ret &= LoadLayers(pel->FirstChildElement("layers"));
		pel = pel->NextSiblingElement("c3pg:terrain");
	}

	return ret;
}


Model *TerrainDescription::GenerateTerrain(System *psys)
{
	assert(psys);

	Model *pmod = nullptr;

	Renderer *prend = psys->GetRenderer();
	ResourceManager *rm = psys->GetResourceManager();

	U8Raster raw_heights;

	if (m_HeightmapFile.has_value())
		util::LoadU8Image(rm, m_HeightmapFile->c_str(), raw_heights);

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

	size_t vcount = (m_Dim.x * m_Dim.y) + ((m_Dim.x - 1) * (m_Dim.y - 1));
	glm::fvec2 texinc(1.0f / (float)(m_Dim.x - 1), 1.0f / (float)(m_Dim.y - 1));
	glm::fvec2 texinc_half = texinc / 2.0f;
	float half_step = m_UnitsBetweenSamples / 2.0f;

	VertexBuffer *pvb = prend->CreateVertexBuffer();
	void *buffer;

	glm::fvec2 uv(0, 0);

	if (pvb && (pvb->Lock(&buffer, vcount, c3::Vertex::PNYT1::d, VBLOCKFLAG_WRITE | VBLOCKFLAG_CACHE) == VertexBuffer::RETURNCODE::RET_OK))
	{
		c3::Vertex::PNYT1::s *v = (c3::Vertex::PNYT1::s *)buffer;

		float ys = 0.0f;
		uv.y = 0.0f;
		float zrange = m_MaxZ - m_MinZ;

		// convert "bytes per pixel" -> "world Z per world X/Y"
		const float slope_scale = (zrange / 255.0f) / m_UnitsBetweenSamples;

		for (size_t y = 0; y < m_Dim.y; y++)
		{
			float xs = 0.0f;
			uv.x = 0.0f;

			for (size_t xo = 0; xo < m_Dim.x; xo++, v++)
			{
				glm::fvec4 s = TerrainSampleU8_1CH(raw_heights.m_Image.data(), raw_heights.m_Dim, uv);

				v->pos.x = xs;
				v->pos.y = ys;
				v->pos.z = s.w * zrange + m_MinZ;

				const float gx = s.x * slope_scale; // dz/dx (world units)
				const float gy = s.y * slope_scale; // dz/dy (world units)
				glm::fvec3 n = glm::normalize(glm::fvec3(-gx, -gy, 1.0f));
				v->norm = n;

				if (std::abs(v->norm.z) > 0.99f)
					v->tang = glm::fvec3(1, 0, 0);
				else
					v->tang = glm::normalize(glm::cross(glm::fvec3(0, 0, 1), n));

				v->binorm = glm::normalize(glm::cross(n, v->tang));

				v->uv = uv;

				xs += m_UnitsBetweenSamples;
				uv.x += texinc.x;
			}

			ys += half_step;
			uv.y += texinc_half.y;

			xs = half_step;
			uv.x = texinc_half.x;

			if (y < (m_Dim.y - 1))
			{
				for (int xi = 0, max_xi = (m_Dim.x - 1); xi < max_xi; xi++, v++)
				{
					glm::fvec4 s = TerrainSampleU8_1CH(raw_heights.m_Image.data(), raw_heights.m_Dim, uv);

					v->pos.x = xs;
					v->pos.y = ys;
					v->pos.z = s.w * zrange + m_MinZ;

					const float gx = s.x * slope_scale; // dz/dx (world units)
					const float gy = s.y * slope_scale; // dz/dy (world units)
					glm::fvec3 n = glm::normalize(glm::fvec3(-gx, -gy, 1.0f));
					v->norm = n;

					if (std::abs(v->norm.z) > 0.99f)
						v->tang = glm::fvec3(1, 0, 0);
					else
						v->tang = glm::normalize(glm::cross(glm::fvec3(0, 0, 1), n));

					v->binorm = glm::normalize(glm::cross(n, v->tang));

					v->uv = uv;

					xs += m_UnitsBetweenSamples;
					uv.x += texinc.x;
				}
			}

			ys += half_step;
			uv.y += texinc_half.y;
		}

		pvb->Unlock();
	}


	IndexBuffer *pib = prend->CreateIndexBuffer();

	size_t numfaces = (m_Dim.x - 1) * (m_Dim.y - 1) * 4;

	uint32_t *pibd = nullptr;
	if (pib->Lock((void **)&pibd, numfaces * 3, IndexBuffer::IndexSize::IS_32BIT, IBLOCKFLAG_WRITE | IBLOCKFLAG_CACHE) == IndexBuffer::RETURNCODE::RET_OK)
	{
		uint32_t ir0 = 0;
		uint32_t mid = (uint32_t)m_Dim.x;
		uint32_t ir1 = mid + (uint32_t)m_Dim.x - 1;

		int64_t maxy = m_Dim.y - 1;
		int64_t maxx = m_Dim.x - 1;

		// Build our indices...
		for (int64_t y = 0; y < maxy; y++)
		{
			uint32_t _ir0 = ir0;
			uint32_t _mid = mid;
			uint32_t _ir1 = ir1;

			for (int64_t x = 0; x < maxx; x++)
			{
				// Face 0 - top
				*(pibd++) = _ir0;
				*(pibd++) = _ir0 + 1;
				*(pibd++) = _mid;

				// Face 1 - left
				*(pibd++) = _ir0;
				*(pibd++) = _mid;
				*(pibd++) = _ir1;

				// Face 2 - bottom
				*(pibd++) = _ir1;
				*(pibd++) = _mid;
				*(pibd++) = _ir1 + 1;

				// Face 3 - right
				*(pibd++) = _mid;
				*(pibd++) = _ir0 + 1;
				*(pibd++) = _ir1 + 1;

				_ir0++;
				_mid++;
				_ir1++;
			}

			ir0 = ir1;
			mid = _ir1 + 1;
			ir1 = mid + (uint32_t)maxx;
		}

		pib->Unlock();
	}

	Mesh *pmesh = prend->CreateMesh();
	if (pmesh)
	{
		pmesh->AttachVertexBuffer(pvb);
		pmesh->AttachIndexBuffer(pib);

		glm::fvec3 vmin(0, 0, m_MinZ);
		glm::fvec3 vmax((m_Dim.x - 1) * m_UnitsBetweenSamples, (m_Dim.y - 1) * m_UnitsBetweenSamples, m_MaxZ);
		((MeshImpl *)pmesh)->SetBounds(vmin, vmax);

		pmod = Model::Create(prend);

		Model::MeshIndex mi = pmod->AddMesh(pmesh);
		Model::NodeIndex ni = pmod->AddNode();
		pmod->AssignMeshToNode(ni, mi);

		((ModelImpl *)pmod)->SetBounds(vmin, vmax);

		Material *pmtl = prend->GetMaterialManager()->CreateMaterial();
		if (pmtl)
		{
			CompositeTextures(psys, pmtl);
			pmod->SetMaterial(mi, pmtl);
		}
	}

	return pmod;
}

// multiplies two bytes as if they were floats in the [0..1] range
inline uint8_t MulUnitBytes(uint8_t a, uint8_t b)
{
	uint16_t t = (uint16_t)a * (uint16_t)b + 128;     // round

	return (uint8_t)((t + (t >> 8)) >> 8);            // == floor((a * b + 128) / 255)
}

// t: 0 = a, t: 255 = very close to b
inline uint8_t LerpU8(uint8_t a, uint8_t b, uint8_t t)
{
	return a + ((t * (b - a) + 128) >> 8); // +128 for rounding
}

inline uint8_t SampleU8_1CH(const void *img, const glm::uvec2 &imgdim, glm::fvec2 &uv)
{
	assert(img);
	const uint8_t *data = reinterpret_cast<const uint8_t *>(img);
	const uint32_t W = imgdim.x;

	glm::uvec3 xsloc, ysloc;
	glm::fvec2 interp;
	ComputeSampleLocations(imgdim, uv, interp, xsloc, ysloc);

	const float h00 = data[ysloc[1] * imgdim.x + xsloc[1]];
	const float h10 = data[ysloc[1] * imgdim.x + xsloc[2]];
	const float h01 = data[ysloc[2] * imgdim.x + xsloc[1]];
	const float h11 = data[ysloc[2] * imgdim.x + xsloc[2]];
	const float ret = std::lerp(std::lerp(h00, h10, interp.x), std::lerp(h01, h11, interp.x), interp.y);

	return (uint8_t)ret;
};

inline glm::fvec3 UnpackNrm_u8(glm::u8vec3 b)
{
	// Map 0..255 -> [-1, 1]
	glm::fvec3 n = glm::fvec3(b) * (2.0f / 255.0f) - 1.0f;

	// In case textures aren’t perfectly unit-length
	return glm::normalize(n);
}

inline glm::u8vec3 PackNrm_u8(glm::fvec3 n)
{
	n = glm::normalize(n);
	glm::fvec3 s = (n * 0.5f + 0.5f) * 255.0f;
	return glm::u8vec3(glm::clamp(glm::ivec3(s + 0.5f), glm::ivec3(0), glm::ivec3(255)));
}


inline glm::u8vec3 SampleTextureU8_3CH(const glm::u8vec3 *img, const glm::uvec2 &imgdim, glm::fvec2 uv)
{
	glm::uvec3 xsloc, ysloc;
	glm::fvec2 interp;
	ComputeSampleLocations(imgdim, uv, interp, xsloc, ysloc);
	glm::u8vec2 binterp((uint8_t)(interp.x * 255.0f), (uint8_t)(interp.y * 255.0f));

	glm::u8vec3 h00 = img[ysloc[1] * imgdim.x + xsloc[1]];
	glm::u8vec3 h10 = img[ysloc[1] * imgdim.x + xsloc[2]];
	glm::u8vec3 h01 = img[ysloc[2] * imgdim.x + xsloc[1]];
	glm::u8vec3 h11 = img[ysloc[2] * imgdim.x + xsloc[2]];

	return glm::u8vec3(
		LerpU8(LerpU8(h00.x, h10.x, binterp.x), LerpU8(h01.x, h11.x, binterp.x), binterp.y),
		LerpU8(LerpU8(h00.y, h10.y, binterp.x), LerpU8(h01.y, h11.y, binterp.x), binterp.y),
		LerpU8(LerpU8(h00.z, h10.z, binterp.x), LerpU8(h01.z, h11.z, binterp.x), binterp.y));
}

inline glm::u8vec4 SampleTextureU8_4CH(const glm::u8vec4 *img, const glm::uvec2 &imgdim, glm::fvec2 uv)
{
	glm::uvec3 xsloc, ysloc;
	glm::fvec2 interp;
	ComputeSampleLocations(imgdim, uv, interp, xsloc, ysloc);
	glm::u8vec2 binterp((uint8_t)(interp.x * 255.0f), (uint8_t)(interp.y * 255.0f));

	glm::u8vec4 h00 = img[ysloc[1] * imgdim.x + xsloc[1]];
	glm::u8vec4 h10 = img[ysloc[1] * imgdim.x + xsloc[2]];
	glm::u8vec4 h01 = img[ysloc[2] * imgdim.x + xsloc[1]];
	glm::u8vec4 h11 = img[ysloc[2] * imgdim.x + xsloc[2]];

	return glm::u8vec4(
		LerpU8(LerpU8(h00.x, h10.x, binterp.x), LerpU8(h01.x, h11.x, binterp.x), binterp.y),
		LerpU8(LerpU8(h00.y, h10.y, binterp.x), LerpU8(h01.y, h11.y, binterp.x), binterp.y),
		LerpU8(LerpU8(h00.z, h10.z, binterp.x), LerpU8(h01.z, h11.z, binterp.x), binterp.y),
		LerpU8(LerpU8(h00.w, h10.w, binterp.x), LerpU8(h01.w, h11.w, binterp.x), binterp.y)
	);
}

bool TerrainDescription::CompositeTextures(System *psys, Material *pmtl)
{
	assert(psys);
	assert(pmtl);

	Renderer *pr = psys->GetRenderer();
	ResourceManager *rm = psys->GetResourceManager();

	RGBARaster tex_diff(m_TexDim.x, m_TexDim.y, glm::u8vec4(0, 0, 0, 255));
	RGBRaster tex_norm(m_TexDim.x, m_TexDim.y, glm::u8vec3(128, 128, 255));
	RGBRaster tex_emis(m_TexDim.x, m_TexDim.y, glm::u8vec3(0, 0, 0));
	RGBRaster tex_surf(m_TexDim.x, m_TexDim.y, glm::u8vec3(0, 128, 255));

	glm::fvec2 uvstep(1.0f / (float)m_TexDim.x, 1.0f / (float)m_TexDim.y);

	for (const auto &layer : m_Layers)
	{
		std::atomic_size_t thcount{0};
		TRaster<uint8_t> opac(m_TexDim.x, m_TexDim.y, (uint8_t)(layer.m_Opacity * 255.0f));

		if (layer.m_LayerOpacityFile.has_value())
		{
			TRaster<uint8_t> tmp;
			util::LoadU8Image(rm, layer.m_LayerOpacityFile->c_str(), tmp);
			opac.Fill([&](size_t x, size_t y) -> uint8_t
			{
				glm::fvec2 opacuv((float)x / (float)m_TexDim.x, (float)y / (float)m_TexDim.y);
				uint8_t o = SampleU8_1CH(tmp.m_Image.data(), tmp.m_Dim, opacuv);
				return MulUnitBytes(o, *opac.GetPixel(x, y));
			});
		}

		glm::fvec2 uvstep_layer = uvstep * layer.m_Tiling;

		if (layer.m_TexFile[Material::TCT_DIFFUSE].has_value())
		{
			RGBARaster diff;
			util::LoadRGBAImage(rm, layer.m_TexFile[Material::TCT_DIFFUSE]->c_str(), diff);

			thcount.fetch_add(1, std::memory_order_relaxed);
			psys->GetThreadPool()->RunTask(
				[diff, &opac, &tex_diff, uvstep_layer, dim = m_TexDim, &thcount](size_t task_number)
				{
					glm::fvec2 uv(0, 0);
					for (size_t y = 0; y < dim.y; y++)
					{
						uv.x = 0;
	
						for (size_t x = 0; x < dim.x; x++)
						{
							glm::u8vec4 c = SampleTextureU8_4CH(diff.m_Image.data(), diff.m_Dim, uv);
	
							// modulate the layer's opacity by the diffuse texture's alpha channel
							uint8_t o = MulUnitBytes(*opac.GetPixel(x, y), c.w);
							opac.SetPixel(x, y, o);
	
							glm::u8vec4 op = *tex_diff.GetPixel(x, y);
							glm::u8vec4 cp = glm::u8vec4(
								LerpU8(op.x, c.x, o),
								LerpU8(op.y, c.y, o),
								LerpU8(op.z, c.z, o),
								255);
	
							tex_diff.SetPixel(x, y, cp);
	
							uv.x += uvstep_layer.x;
						}
	
						uv.y += uvstep_layer.y;
					}
	
					thcount.fetch_sub(1, std::memory_order_acq_rel);
	
					return pool::IThreadPool::TR_OK;
				});
		}

		if (layer.m_TexFile[Material::TCT_NORMAL].has_value())
		{
			RGBRaster norm;
			util::LoadRGBImage(rm, layer.m_TexFile[Material::TCT_NORMAL]->c_str(), norm);

			thcount.fetch_add(1, std::memory_order_relaxed);
			psys->GetThreadPool()->RunTask(
				[norm, &opac, &tex_norm, uvstep_layer, dim = m_TexDim, &thcount](size_t task_number)
				{
					glm::fvec2 uv(0, 0);
					for (size_t y = 0; y < dim.y; y++)
					{
						uv.x = 0;
	
						for (size_t x = 0; x < dim.x; x++)
						{
							glm::u8vec3 n = SampleTextureU8_3CH(norm.m_Image.data(), norm.m_Dim, uv);
							glm::fvec3 nn = UnpackNrm_u8(n);
							glm::fvec3 on = UnpackNrm_u8(*tex_norm.GetPixel(x, y));
	
							uint8_t o = *opac.GetPixel(x, y);
							float w = float(o) * (1.0f / 255.0f);
	
							if (w > 0.0f)
							{
								nn = glm::normalize((on * (1.0f - w)) + (nn * w));
	
								// Store to your packed normal raster
								tex_norm.SetPixel(x, y, PackNrm_u8(nn));
							}
	
							uv.x += uvstep_layer.x;
						}
	
						uv.y += uvstep_layer.y;
					}
	
					thcount.fetch_sub(1, std::memory_order_acq_rel);
	
					return pool::IThreadPool::TR_OK;
				});
		}

		if (layer.m_TexFile[Material::TCT_SURFACEDESC].has_value())
		{
			RGBRaster surfdesc;
			util::LoadRGBImage(rm, layer.m_TexFile[Material::TCT_SURFACEDESC]->c_str(), surfdesc);

			thcount.fetch_add(1, std::memory_order_relaxed);
			psys->GetThreadPool()->RunTask(
				[surfdesc, &opac, &tex_surf, uvstep_layer, dim = m_TexDim, &thcount](size_t task_number)
				{
					glm::fvec2 uv(0, 0);
					for (size_t y = 0; y < dim.y; y++)
					{
						uv.x = 0;
	
						for (size_t x = 0; x < dim.x; x++)
						{
							glm::u8vec3 c = SampleTextureU8_3CH(surfdesc.m_Image.data(), surfdesc.m_Dim, uv);
	
							uint8_t o = *opac.GetPixel(x, y);
	
							glm::u8vec3 op = *tex_surf.GetPixel(x, y);
							glm::u8vec3 cp = glm::u8vec3(
								LerpU8(op.x, c.x, o),
								LerpU8(op.y, c.y, o),
								LerpU8(op.z, c.z, o));
	
							tex_surf.SetPixel(x, y, cp);
	
							uv.x += uvstep_layer.x;
						}
	
						uv.y += uvstep_layer.y;
					}
	
					thcount.fetch_sub(1, std::memory_order_acq_rel);
	
					return pool::IThreadPool::TR_OK;
				});
		}

		// wait while the textures for this layer are composited
		Sleep(0);
		while (thcount)
			Sleep(0);
	}

	Texture2D *ptex;
	uint8_t *dst = nullptr;
	Texture2D::SLockInfo li;

	ptex = pr->CreateTexture2D(m_TexDim.x, m_TexDim.y, Renderer::ETextureType::U8_4CH,
		0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);
	if (ptex)
	{
		if ((ptex->Lock((void **)&dst, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture::RET_OK) && dst)
		{
			memcpy(dst, tex_diff.m_Image.data(), m_TexDim.x * m_TexDim.y * sizeof(glm::u8vec4));
			ptex->Unlock();
		}
		pmtl->SetTexture(Material::TCT_DIFFUSE, ptex);
	}

	ptex = pr->CreateTexture2D(m_TexDim.x, m_TexDim.y, Renderer::ETextureType::U8_3CH,
		0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V);
	if (ptex)
	{
		if ((ptex->Lock((void **)&dst, li, 0, TEXLOCKFLAG_WRITE) == Texture::RET_OK) && dst)
		{
			memcpy(dst, tex_norm.m_Image.data(), m_TexDim.x * m_TexDim.y * sizeof(glm::u8vec3));
			ptex->Unlock();
		}
		pmtl->SetTexture(Material::TCT_NORMAL, ptex);
	}

	ptex = pr->CreateTexture2D(m_TexDim.x, m_TexDim.y, Renderer::ETextureType::U8_3CH,
		0, TEXFLAG_WRAP_U | TEXFLAG_WRAP_V | TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR);
	if (ptex)
	{
		if ((ptex->Lock((void **)&dst, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_GENMIPS) == Texture::RET_OK) && dst)
		{
			memcpy(dst, tex_surf.m_Image.data(), m_TexDim.x * m_TexDim.y * sizeof(glm::u8vec3));
			ptex->Unlock();
		}
		pmtl->SetTexture(Material::TCT_SURFACEDESC, ptex);
	}

	return true;
}

