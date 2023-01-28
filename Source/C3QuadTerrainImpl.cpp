// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


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
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	return true;
}


void QuadTerrainImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool QuadTerrainImpl::Prerender(Object *pobject, Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!pobject->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void QuadTerrainImpl::Render(Object *pobject, Object::RenderFlags flags)
{
	assert(pobject);

	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();

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
			glm::fvec2 vdimf((float)m_VertDim.x * m_Scale.x, (float)m_VertDim.y * m_Scale.y);
			glm::fvec2 ofs = vdimf / 2.0f;
			glm::fvec2 texinc(1.0f / (float)m_HeightTexDim.x, 1.0f / (float)m_HeightTexDim.y);
			glm::fvec2 texinc_half = texinc / 2.0f;

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

					ys += (0.5f * m_Scale.y);
					tv += texinc_half.y;

					xs = -ofs.x + (0.5f * m_Scale.x);
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
							v->norm.z = 0.5;

							v->binorm.x = 0;
							v->binorm.y = 0.5;
							v->binorm.z = 0;

							v->tang.x = 1;
							v->tang.y = 0;
							v->tang.z = 0;

							v->uv.x = tu;
							v->uv.y = tv;

							xs += m_Scale.x;
							tu += texinc.x;
						}
					}

					ys += (0.5f * m_Scale.y);
					tv += texinc_half.y;
				}

				m_VB->Unlock();
			}
		}

		if (!m_Root)
			m_Root = GenerateGeometryQuadSet(0, 0, m_VertDim.x, m_VertDim.y);

		if (!m_SP_terr)
		{
			ResourceManager *prm = pobject->GetSystem()->GetResourceManager();

			props::IProperty *pvsh = pobject->GetProperties()->GetPropertyById('VSHF');
			props::IProperty *pfsh = pobject->GetProperties()->GetPropertyById('FSHF');
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

			m_SP_terr = pobject->GetSystem()->GetRenderer()->CreateShaderProgram();

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
			}
		}

		if (!m_HeightTex)
		{
			m_HeightTex = prend->CreateTexture2D(m_HeightTexDim.x, m_HeightTexDim.y, Renderer::TextureType::F32_1CH, 1, 0);

			float *phtpix = nullptr;
			Texture2D::SLockInfo li;
			if (m_HeightTex && m_HeightTex->Lock((void **)&phtpix, li) == Texture2D::RETURNCODE::RET_OK)
			{
				for (size_t y = 0; y < li.height; y++)
				{
					float *pp = phtpix;

					for (size_t x = 0; x < li.width; x++)
					{
						pp[x] = (sinf((float)x / 10.0f) + cosf((float)y / 10.0f)) * 4.0f;
					}

					phtpix += li.stride / sizeof(float);
				}

				m_HeightTex->Unlock();
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

		prend->GetWhiteMaterial()->Apply(m_SP_terr);
	}
	else
	{
#if 0
		if (!m_SP_obj)
		{
			props::IProperty *pvsh = pobject->GetProperties()->GetPropertyById('VSSF');
			props::IProperty *pfsh = pobject->GetProperties()->GetPropertyById('FSSF');
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

			m_SP_shadowobj = pobject->GetSystem()->GetRenderer()->CreateShaderProgram();

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

	Renderer::FillMode fillmode = prend->GetFillMode();
	prend->SetFillMode(Renderer::FillMode::FM_WIRE);
	RenderQuad(m_Root, flags);
	prend->SetFillMode(fillmode);
}

void QuadTerrainImpl::RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags)
{
	if (node)
	{
		TerrainQuad *quad = node->GetData();

		if (quad)// && sys->Renderer()->IsVolumeVisible(quad->min_bounds, quad->max_bounds, &matrix))
		{
			bool allvis = true;
			for (size_t qidx = CTerrainQuadNode::ChildQuad::POSX_POSY; qidx < CTerrainQuadNode::ChildQuad::NUM_QUADS; qidx++)
			{
				CTerrainQuadNode *pqn = node->GetChild((CTerrainQuadNode::ChildQuad)qidx);
				TerrainQuad *ptq = pqn ? pqn->GetData() : nullptr;
				allvis &= !ptq ? true : true; //ptq->m_Bounds.IsBoxInside();
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


bool QuadTerrainImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const
{
	return false;
}
