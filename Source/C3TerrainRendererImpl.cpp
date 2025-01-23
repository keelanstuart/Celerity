// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3TerrainRendererImpl.h>
#include <C3CommonVertexDefs.h>

using namespace c3;


DECLARE_COMPONENTTYPE(TerrainRenderer, TerrainRendererImpl);


TerrainRendererImpl::TerrainRendererImpl() : m_Flags(0)
{
}


TerrainRendererImpl::~TerrainRendererImpl()
{
}


void TerrainRendererImpl::Release()
{
	delete this;
}


props::TFlags64 TerrainRendererImpl::Flags() const
{
	return m_Flags;
}


bool TerrainRendererImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(m_pOwner->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *ps = m_pOwner->GetProperties();
	if (!ps)
		return false;

	props::IProperty *p;

	if (p = ps->CreateProperty(_T("Terrain.Offset"), 'Tofs'))
	{
		p->SetVec2I(props::TVec2I(0, 0));
		p->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	if (p = ps->CreateProperty(_T("Terrain.Range"), 'Trng'))
	{
		p->SetVec2I(props::TVec2I(-1, -1));
		p->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	if (p = ps->CreateProperty(_T("Terrain.Cover.Image"), 'Tcvr'))
	{
		p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		p->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	if (p = ps->CreateProperty(_T("Terrain.Height.Image"), 'Thgt'))
	{
		p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		p->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	if (p = ps->CreateProperty(_T("Terrain.Height.Scale"), 'Tscl'))
	{
		p->SetFloat(1.0f);
		p->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	return true;
}


void TerrainRendererImpl::Update(float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool TerrainRendererImpl::Prerender(Object::RenderFlags flags, int draworder)
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


const TCHAR *TerrainRendererImpl::MakeTerrainModelFilename(TCHAR *namebuf, size_t namebuflen, size_t xdim, size_t ydim)
{
	_sntprintf(namebuf, namebuflen, _T("[terrain_%zux%zu.model]"), xdim, ydim);
}




void TerrainRendererImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

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

	Renderer::FillMode fillmode = prend->GetFillMode();
	prend->SetFillMode(Renderer::FillMode::FM_WIRE);
	RenderQuad(m_Root, flags);
	prend->SetFillMode(fillmode);
}

void TerrainRendererImpl::RenderQuad(CTerrainQuadNode *node, props::TFlags64 rendflags)
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


void TerrainRendererImpl::PropertyChanged(const props::IProperty *pprop)
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


void TerrainRendererImpl::DeleteQuadNode(CTerrainQuadNode *node)
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


TerrainRendererImpl::CTerrainQuadNode *TerrainRendererImpl::GenerateGeometryQuadSet(size_t startx, size_t starty, size_t xdim, size_t ydim)
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


bool TerrainRendererImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const
{
	return false;
}
