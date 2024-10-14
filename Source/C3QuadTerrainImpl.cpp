// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3QuadTerrainImpl.h>
#include <C3CommonVertexDefs.h>

using namespace c3;


DECLARE_COMPONENTTYPE(QuadTerrain, QuadTerrainImpl);


QuadTerrainImpl::QuadTerrainImpl()
{
	m_Flags.Set(QTFLAG_DIRTY);
}


QuadTerrainImpl::~QuadTerrainImpl()
{
	if (m_SP_terr)
	{
		m_SP_terr->Release();
		m_SP_terr = nullptr;
	}

	DeleteQuadNode(m_Root);
	m_Root = nullptr;

	if (m_HeightTex)
	{
		m_HeightTex->Release();
		m_HeightTex = nullptr;
	}

	if (m_VB)
	{
		m_VB->Release();
		m_VB = nullptr;
	}
}


void QuadTerrainImpl::Release()
{
	delete this;
}


props::TFlags64 QuadTerrainImpl::Flags() const
{
	return m_Flags;
}


bool QuadTerrainImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(m_pOwner->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = m_pOwner->GetProperties();
	if (!props)
		return false;

	props::IProperty *pp;
	
	if (pp = props->CreateProperty(_T("FragmentShader"), 'FSHF'))
		pp->SetString(_T("def-terrain.fsh"));

	if (pp = props->CreateProperty(_T("VertexShader"), 'VSHF'))
		pp->SetString(_T("def-terrain.vsh"));

	if (pp = props->CreateProperty(_T("HeightMap"), 'HtMp'))
		pp->SetString(_T("test_terrain.png"));

	return true;
}


void QuadTerrainImpl::Update(float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool QuadTerrainImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (flags.IsSet(RF_LIGHT))
		return false;

	//if (!m_pMethod || (draworder == m_pMethod->GetActiveTechnique()->GetDrawOrder()))
	{
		if (m_pOwner->Flags().IsSet(OF_DRAW))
			return true;

		if (flags.IsSet(RF_EDITORDRAW) && m_pOwner->Flags().IsSet(OF_DRAWINEDITOR))
			return true;
	}

	return false;
}


using TerrainSampleFunc = std::function<float(const void *h, Texture2D::SLockInfo *li, props::TVec2I &pos)>;

TerrainSampleFunc SampleF32_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return ((float *)h)[pos.y * li->width + pos.x];
};

TerrainSampleFunc SampleS8_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return (float)((int8_t *)h)[pos.y * li->stride + pos.x] * 0.5f;
};

TerrainSampleFunc SampleU8_1CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return (float)((uint8_t *)h)[pos.y * li->stride + pos.x] * 0.5f;

};

TerrainSampleFunc SampleU8_2CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return (float)((uint16_t *)h)[pos.y * li->stride + pos.x] * 0.5f;
};

TerrainSampleFunc SampleU8_3CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return (float)((uint8_t *)h)[pos.y * li->stride + (pos.x * 3)] * 0.5f;
};

TerrainSampleFunc SampleU8_4CH = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	assert(h);
	assert((size_t)pos.x < li->width);
	assert((size_t)pos.y < li->height);

	return (float)((uint8_t *)h)[pos.y * li->stride + (pos.x * 4)] * 0.5f;
};

TerrainSampleFunc SampleDummy = [](const void *h, const Texture2D::SLockInfo *li, props::TVec2I &pos)
{
	return 0.0f;
};


void QuadTerrainImpl::LoadHeightMap(const TCHAR *filename)
{
	Renderer *rend = m_pOwner->GetSystem()->GetRenderer();
	ResourceManager *rm = m_pOwner->GetSystem()->GetResourceManager();

	Resource *r = rm->GetResource(filename, RESF_DEMANDLOAD);
	Texture2D *readtex = nullptr;
	void *readbuf = nullptr;
	Texture2D::SLockInfo readli;
	TerrainSampleFunc samp = SampleDummy;
	m_HeightTexDim.x = m_HeightTexDim.y = 128;

	if (r && (r->GetStatus() == Resource::RS_LOADED))
	{
		readtex = dynamic_cast<Texture2D *>((Texture2D *)(r->GetData()));
		if (readtex)
		{
			m_HeightTexDim.x = readtex->Width();
			m_HeightTexDim.y = readtex->Height();
			readtex->Lock(&readbuf, readli, 0, TEXLOCKFLAG_READ);

			switch (readtex->Format())
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

				case Renderer::TextureType::U8_3CH:
					samp = SampleU8_3CH;
					break;

				case Renderer::TextureType::U8_4CH:
					samp = SampleU8_4CH;
					break;
			}
		}
	}

	// if the height texture existed and the size changed, free it
	if (m_HeightTex && ((m_HeightTexDim.x != m_HeightTex->Width()) || (m_HeightTexDim.y != m_HeightTex->Height())))
		C3_SAFERELEASE(m_HeightTex);

	C3_SAFERELEASE(m_VB);
	DeleteQuadNode(m_Root);

	// if we don't have a height texture any more, make a new one
	if (!m_HeightTex)
		m_HeightTex = rend->CreateTexture2D(m_HeightTexDim.x, m_HeightTexDim.y, Renderer::TextureType::F32_1CH, 1, 0);

	if (m_HeightTex)
	{
		float *phtpix = nullptr;
		Texture2D::SLockInfo li;

		// lock the height texture and fill it by sampling the read texture given then appropriate sampler
		if (m_HeightTex && m_HeightTex->Lock((void **)&phtpix, li, 0, TEXLOCKFLAG_WRITE | TEXLOCKFLAG_CACHE) == Texture2D::RETURNCODE::RET_OK)
		{
			for (size_t y = 0; y < li.height; y++)
			{
				for (size_t x = 0; x < li.width; x++)
				{
					*(phtpix++) = samp(readbuf, &readli, props::TVec2I(x, y));
				}
			}

			m_HeightTex->Unlock();
		}
	}

	// if we loaded a read texture, then unlock it now
	if (readbuf && readtex)
	{
		readtex->Unlock();
	}
}


void QuadTerrainImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

	if (m_Flags.IsSet(QTFLAG_DIRTY))
	{
		m_Flags.Clear(QTFLAG_DIRTY);

		if (!m_VB)
		{
			m_VB = prend->CreateVertexBuffer();

			void *buffer;

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

			size_t vcount = (m_VertDim.x * m_VertDim.y) + ((m_VertDim.x - 1) * (m_VertDim.y - 1));
			glm::fvec2 vdimf((float)m_VertDim.x, (float)m_VertDim.y);
			glm::fvec2 texinc(1.0f / (float)m_HeightTexDim.x, 1.0f / (float)m_HeightTexDim.y);
			glm::fvec2 texinc_half = texinc / 2.0f;

			// I used to center the terrain around the origin... but that makes collisions more irritating
			glm::fvec2 ofs(0, 0); // = vdimf / 2.0f;

			if (m_VB && (m_VB->Lock(&buffer, vcount, c3::Vertex::PNYT1::d, VBLOCKFLAG_WRITE) == VertexBuffer::RETURNCODE::RET_OK))
			{
				c3::Vertex::PNYT1::s *v = (c3::Vertex::PNYT1::s *)buffer;

				float ys = -ofs.y;
				float tv = -texinc_half.y;

				for (int y = 0; y < m_VertDim.y; y++)
				{
					float xs = -ofs.x;
					float tu = -texinc_half.x;

					for (int xo = 0; xo < m_VertDim.x; xo++, v++)
					{
						v->pos.x = xs;
						v->pos.y = ys;
						v->pos.z = 0;

						v->norm.x = texinc_half.x;
						v->norm.y = texinc_half.y;
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

					xs = -ofs.x + 0.5f;
					tu = 0.0f;

					if (y < (m_VertDim.y - 1))
					{
						for (int xi = 0; xi < (m_VertDim.x - 1); xi++, v++)
						{
							v->pos.x = xs;
							v->pos.y = ys;
							v->pos.z = 0;

							v->norm.x = 0;
							v->norm.y = 0;
							v->norm.z = 0.5f;

							v->binorm.x = 0;
							v->binorm.y = 0.5f;
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

				m_VB->Unlock();
			}
		}

		if (!m_Root)
			m_Root = GenerateGeometryQuadSet(0, 0, m_VertDim.x, m_VertDim.y);

		if (!m_SP_terr)
		{
			ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

			props::IProperty *pvsh = m_pOwner->GetProperties()->GetPropertyById('VSHF');
			props::IProperty *pfsh = m_pOwner->GetProperties()->GetPropertyById('FSHF');
			if (!m_VS_terr)
			{
				c3::Resource *pres = prm->GetResource(pvsh ? pvsh->AsString() : _T("def-terrain.vsh"), RESF_DEMANDLOAD);
				if (pres)
					m_VS_terr = (c3::ShaderComponent *)(pres->GetData());
			}

			if (!m_FS_terr)
			{
				c3::Resource *pres = prm->GetResource(pfsh ? pfsh->AsString() : _T("def-terrain.fsh"), RESF_DEMANDLOAD);
				if (pres)
					m_FS_terr = (c3::ShaderComponent *)(pres->GetData());
			}

			m_SP_terr = m_pOwner->GetSystem()->GetRenderer()->CreateShaderProgram();

			if (m_SP_terr && m_VS_terr && m_FS_terr)
			{
				m_SP_terr->AttachShader(m_VS_terr);
				m_SP_terr->AttachShader(m_FS_terr);
				if (m_SP_terr->Link() == ShaderProgram::RETURNCODE::RET_OK)
				{
					// anything special to do when the shader links correctly
					m_uSamplerHeight = m_SP_terr->GetUniformLocation(_T("uSamplerHeight"));
					m_uSamplerHeightStep = m_SP_terr->GetUniformLocation(_T("uSamplerHeightStep"));
				}

				m_pMtl = prend->GetMaterialManager()->CreateMaterial();
				m_pMtl->SetColor(Material::CCT_DIFFUSE, Color::iWhite);
				m_pMtl->SetTexture(Material::TCT_DIFFUSE, prend->GetWhiteTexture());
				//m_pMtl->RenderModeFlags().Set(Material::RENDERMODEFLAG(Material::RMF_WIREFRAME));
			}
		}
	}

	prend->SetWorldMatrix(m_pPos->GetTransformMatrix());

	if (!flags.IsSet(OF_CASTSHADOW))
	{
		prend->UseProgram(m_SP_terr);

		glm::fvec2 step(1.0f / (float)m_HeightTexDim.x, 1.0f / (float)m_HeightTexDim.y);
		m_SP_terr->SetUniform2(m_uSamplerHeightStep, &step);
		m_SP_terr->SetUniformTexture(m_HeightTex, m_uSamplerHeight, -1, 0);

		m_pMtl->Apply(m_SP_terr);
	}
	else
	{
#if 0
		if (!m_SP_obj)
		{
			props::IProperty *pvsh = m_pOwner->GetProperties()->GetPropertyById('VSSF');
			props::IProperty *pfsh = m_pOwner->GetProperties()->GetPropertyById('FSSF');
			if (!m_VS_shadowobj)
			{
				c3::Resource *pres = prm->GetResource(pvsh ? pvsh->AsString() : _T("def-obj-shadow.vsh"), rf);
				if (pres)
					m_VS_shadowobj = (c3::ShaderComponent *)(pres->GetData());
			}

			if (!m_FS_shadowobj)
			{
				c3::Resource *pres = prm->GetResource(pfsh ? pfsh->AsString() : _T("def-obj-shadow.fsh"), rf);
				if (pres)
					m_FS_shadowobj = (c3::ShaderComponent *)(pres->GetData());
			}

			m_SP_shadowobj = m_pOwner->GetSystem()->GetRenderer()->CreateShaderProgram();

			if (m_SP_shadowobj && m_VS_shadowobj && m_FS_shadowobj)
			{
				m_SP_shadowobj->AttachShader(m_VS_shadowobj);
				m_SP_shadowobj->AttachShader(m_FS_shadowobj);
				if (m_SP_shadowobj->Link() == ShaderProgram::RETURNCODE::RET_OK)
				{
					m_Flags.Clear(Object::OBJFLAG(Object::CASTSHADOW));
					return;
				}
			}
			else
			{
				m_Flags.Clear(Object::OBJFLAG(Object::CASTSHADOW));
				return;
			}
		}

		prend->UseProgram(m_SP_shadowobj);
#endif
	}

	prend->UseMaterial(m_pMtl);
	prend->UseRenderMethod(nullptr);
	RenderQuad(m_Root, flags);
}

void QuadTerrainImpl::RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags)
{
	if (node)
	{
		TerrainQuad *quad = node->GetData();

		const BoundingBox *clipfrust = m_pOwner->GetSystem()->GetRenderer()->GetClipFrustum();

		if (quad && clipfrust->IsBoxInside(&quad->m_Bounds))
		{
			bool allvis = true;

			for (size_t qidx = CTerrainQuadNode::ChildQuad::POSX_POSY; qidx < CTerrainQuadNode::ChildQuad::NUM_QUADS; qidx++)
			{
				CTerrainQuadNode *pqn = node->GetChild((CTerrainQuadNode::ChildQuad)qidx);
				TerrainQuad *ptq = pqn ? pqn->GetData() : nullptr;
				allvis &= clipfrust->IsBoxInside(&ptq->m_Bounds);
			}

			c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

			// If there are any child nodes that either don't exist or if all children are visible...
			// then render this quad object, not the children!

			if (allvis)
			{
				TerrainQuad *ptq = node->GetData();

				ptq->m_Mtl->Apply(prend->GetActiveProgram());

				prend->UseVertexBuffer(m_VB);
				prend->UseIndexBuffer(ptq->m_IB);
				prend->DrawIndexedPrimitives(Renderer::PrimType::TRILIST, 0, (m_VertDim.x - 1) * (m_VertDim.x - 1) * 4 * 3);
			}
			else
			{
				// Draw the individual child quads...
				for (size_t qidx = CTerrainQuadNode::ChildQuad::POSX_POSY; qidx < CTerrainQuadNode::ChildQuad::NUM_QUADS; qidx++)
					RenderQuad(node->GetChild((CTerrainQuadNode::ChildQuad)qidx), rendflags);
			}
		}
	}
}



void QuadTerrainImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);
	if (!m_pOwner)
		return;

	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	switch (pprop->GetID())
	{
		case 'VSHF':
			m_VS_terr = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
			break;

		case 'FSHF':
			m_FS_terr = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
			break;

		case 'HtMp':
			LoadHeightMap(pprop->AsString());
			break;
	}
}


void QuadTerrainImpl::DeleteQuadNode(CTerrainQuadNode *node)
{
	if (node)
	{
		DeleteQuadNode(node->GetChild(CTerrainQuadNode::ChildQuad::POSX_POSY));
		DeleteQuadNode(node->GetChild(CTerrainQuadNode::ChildQuad::POSX_NEGY));
		DeleteQuadNode(node->GetChild(CTerrainQuadNode::ChildQuad::NEGX_POSY));
		DeleteQuadNode(node->GetChild(CTerrainQuadNode::ChildQuad::NEGX_NEGY));

		TerrainQuad *ptq = node->GetData();
		if (ptq)
		{
			if (ptq->m_IB)
			{
				ptq->m_IB->Release();
				ptq->m_IB = nullptr;
			}

			delete ptq;
			node->SetData(nullptr);
		}

		delete node;
	}
}


QuadTerrainImpl::CTerrainQuadNode *QuadTerrainImpl::GenerateGeometryQuadSet(size_t startx, size_t starty, size_t xdim, size_t ydim)
{
	CTerrainQuadNode *ret = new CTerrainQuadNode;

	// create index buffer
	TerrainQuad *ptq = new TerrainQuad;
	if (!ptq)
		return nullptr;

	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

	ptq->m_NumVerts = xdim * ydim;
	ptq->m_NumFaces = (xdim - 1) * (ydim - 1) * 4;

	ptq->m_Mtl = prend->GetMaterialManager()->CreateMaterial();
	if (ptq->m_Mtl)
	{
		ptq->m_Mtl->SetWindingOrder(Renderer::WindingOrder::WO_CCW);
		Resource *pnt = m_pOwner->GetSystem()->GetResourceManager()->GetResource(_T("[blue.tex]" /*"terrain-normal.jpg"*/), RESF_DEMANDLOAD);
		ptq->m_Mtl->SetTexture(Material::TextureComponentType::TCT_NORMAL, pnt);

		glm::fvec4 tc(0.25f, 0.68f, 0.12f, 1.0f);
		ptq->m_Mtl->SetColor(Material::ColorComponentType::CCT_DIFFUSE, &tc);
	}

	ptq->m_IB = prend->CreateIndexBuffer();

	uint16_t *pib = nullptr;
	if (ptq->m_IB->Lock((void **)&pib, ptq->m_NumFaces * 3, IndexBuffer::IndexSize::IS_16BIT, IBLOCKFLAG_WRITE) == IndexBuffer::RETURNCODE::RET_OK)
	{
		uint16_t ir0 = (uint16_t)starty * (uint16_t)startx;
		uint16_t mid = (uint16_t)m_VertDim.x;
		uint16_t ir1 = mid + (uint16_t)m_VertDim.x - 1;

		// Build our indices...
		for (int64_t y = 0, maxy = ydim - 1; y < maxy; y++)
		{
			uint16_t _ir0 = ir0;
			uint16_t _mid = mid;
			uint16_t _ir1 = ir1;

			for (int64_t x = 0, maxx = xdim - 1; x < maxx; x++)
			{
				// Face 0 - top
				*(pib++) = _ir0;
				*(pib++) = _mid;
				*(pib++) = _ir0 + 1;

				// Face 1 - left
				*(pib++) = _ir0;
				*(pib++) = _ir1;
				*(pib++) = _mid;

				// Face 2 - bottom
				*(pib++) = _ir1;
				*(pib++) = _ir1 + 1;
				*(pib++) = _mid;

				// Face 3 - right
				*(pib++) = _mid;
				*(pib++) = _ir1 + 1;
				*(pib++) = _ir0 + 1;

				_ir0++;
				_mid++;
				_ir1++;
			}

			ir0 = ir1;
			mid = _ir1 + 1;
			ir1 = mid + (uint16_t)m_VertDim.x - 1;
		}

		ptq->m_IB->Unlock();
	}

	ret->SetData(ptq);

#if 0
#define SMALLEST_DIM	16

	if ((xdim < SMALLEST_DIM) || (ydim < SMALLEST_DIM))
		return;

	xdim /= 2;
	ydim /= 2;

	ret->SetChild(CTerrainQuadNode::NEGX_NEGY, GenerateGeometryQuadSet(startx, starty, xdim, ydim));
	ret->SetChild(CTerrainQuadNode::NEGX_POSY, GenerateGeometryQuadSet(startx, starty + ydim, xdim, ydim));
	ret->SetChild(CTerrainQuadNode::POSX_NEGY, GenerateGeometryQuadSet(startx + xdim, starty, xdim, ydim));
	ret->SetChild(CTerrainQuadNode::POSX_POSY, GenerateGeometryQuadSet(startx + xdim, starty + ydim, xdim, ydim));
#endif

	return ret;
}


bool QuadTerrainImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const
{
	return false;

	if (!m_HeightTex)
		return false;

	if (!pRayPos || !pRayDir)
		return false;

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	// Inverse of the transformation matrix to transform the ray to local space
	glm::fmat4x4 invMat = glm::inverse(*pmat);

	// Transform ray position and direction to local space
	glm::vec3 localRayPos = glm::vec3(invMat * glm::vec4(*pRayPos, 1.0f));
	glm::vec3 localRayDir = glm::normalize(glm::vec3(invMat * glm::vec4(*pRayDir, 0.0f)));

	bool ret = false;

	float *pheights;
	Texture2D::SLockInfo li;
	if (m_HeightTex->Lock((void **)&pheights, li, 0, TEXLOCKFLAG_READ | TEXLOCKFLAG_CACHE) == Texture::RETURNCODE::RET_OK)
	{
		glm::vec3 pos = localRayPos;
		while (((pos.x <= 0) && (localRayDir.x > 0)) || ((pos.x >= li.width) && (localRayDir.x < 0)) &&
			   ((pos.y <= 0) && (localRayDir.y > 0)) || ((pos.y >= li.height) && (localRayDir.y < 0)) &&
			   ((pos.z > m_MinHeight) && (localRayDir.z > 0)) || ((pos.z < m_MaxHeight) && (localRayDir.z < 0)))
		{
			int64_t y = (int64_t)floor(pos.y);
			int64_t x = (int64_t)floor(pos.x);

			if ((y < 0) && (y >= m_HeightTexDim.y) && (x < 0) && (x >= m_HeightTexDim.x))
			{
				pos += localRayDir;
				continue;
			}

			int64_t ny = (int64_t)floor(pos.y + localRayDir.y);
			int64_t nx = (int64_t)floor(pos.y + localRayDir.y);

			if ((ny < 0) && (ny >= m_HeightTexDim.y) && (nx < 0) && (nx >= m_HeightTexDim.x))
			{
				pos += localRayDir;
				continue;
			}

			size_t swi = (y * li.stride) * x;
			size_t nwi = (ny * li.stride) * x;
			size_t sei = (y * li.stride) * nx;
			size_t nei = (ny * li.stride) * nx;

			float hsw = pheights[swi];
			float hnw = pheights[nwi];
			float hse = pheights[sei];
			float hne = pheights[nei];
			float hc = (hsw + hse + hnw + hne) / 4.0f;

			glm::vec3 sw(x, y, hsw);
			glm::vec3 nw(x, ny, hnw);
			glm::vec3 se(nx, y, hse);
			glm::vec3 ne(nx, ny, hne);
			glm::vec3 c(x + 0.5f, y + 0.5f, hc);

			// check for a collision
			bool hit = false;
			glm::vec2 luv;
			float ldist;

			hit = glm::intersectRayTriangle(pos, localRayDir, c, nw, sw, luv, ldist);
			if (!hit)
				hit = glm::intersectRayTriangle(pos, localRayDir, c, sw, se, luv, ldist);
			if (!hit)
				hit = glm::intersectRayTriangle(pos, localRayDir, c, sw, ne, luv, ldist);
			if (!hit)
				hit = glm::intersectRayTriangle(pos, localRayDir, c, ne, nw, luv, ldist);

			if (hit)
			{
				// Transform distance back to the original coordinate space
				glm::vec3 hitPoint = localRayPos + ldist * localRayDir;
				glm::vec3 transformedHitPoint = glm::vec3(*pmat * glm::vec4(hitPoint, 1.0f));
				float worldDistance = glm::length(transformedHitPoint - *pRayPos);

				float cdist = FLT_MAX, *pcdist = pDistance ? pDistance : &cdist;

				// Get the nearest collision
				if ((worldDistance >= 0) && (worldDistance < *pcdist))
				{
					*pcdist = worldDistance;
					ret = true;
				}
			}

			pos += localRayDir;
		}



	}

	return ret;
}
