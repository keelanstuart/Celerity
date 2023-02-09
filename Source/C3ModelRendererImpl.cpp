// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ModelRendererImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ModelRenderer, ModelRendererImpl);


ModelRendererImpl::ModelRendererImpl() : m_Pos(0, 0, 0), m_Ori(1, 0, 0, 0), m_Scl(1, 1, 1)
{
	m_pPos = nullptr;
	m_pMethod = nullptr;
	m_Mod = TModOrRes(nullptr, nullptr);
	m_Mat = glm::identity<glm::fmat4x4>();
	m_MatN = glm::identity<glm::fmat4x4>();

	m_Flags.SetAll(MRIF_REBUILDMATRIX);
}


ModelRendererImpl::~ModelRendererImpl()
{
}


void ModelRendererImpl::Release()
{
	delete this;
}


props::TFlags64 ModelRendererImpl::Flags() const
{
	return m_Flags;
}


bool ModelRendererImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	props::IProperty *pp;
	pp = props->CreateReferenceProperty(_T("ModelPosition"), 'MPOS', &m_Pos, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pp)
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	pp = props->CreateReferenceProperty(_T("ModelOrientation"), 'MORI', &m_Ori, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_QUATERNION);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN) | props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	pp = props->CreateReferenceProperty(_T("ModelScale"), 'MSCL', &m_Scl, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pp)
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	return true;
}


void ModelRendererImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPos)
		return;

	if (m_Flags.IsSet(MRIF_REBUILDMATRIX))
	{
		// Scale first, then rotate...
		m_Mat = glm::scale(glm::identity<glm::fmat4x4>(), m_Scl) * (glm::fmat4x4)m_Ori;

		// Then translate last... 
		m_Mat = glm::translate(glm::identity<glm::fmat4x4>(), m_Pos) * m_Mat;

		m_MatN = glm::inverseTranspose(m_Mat);

		m_Flags.Clear(MRIF_REBUILDMATRIX);
	}
}


bool ModelRendererImpl::Prerender(Object *pobject, Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (flags.IsSet(RF_EDITORDRAW) && pobject->Flags().IsSet(OF_DRAWINEDITOR))
		return true;

	if (!pobject->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void ModelRendererImpl::Render(Object *pobject, Object::RenderFlags flags)
{
	assert(pobject);
	if (!Prerender(pobject, flags))
		return;

	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();

	Model *pmod = nullptr;
	if (m_Mod.second && (m_Mod.second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a model, use it
		pmod = dynamic_cast<Model *>((Model *)(m_Mod.second->GetData()));
	}

	ResourceManager *prm = pobject->GetSystem()->GetResourceManager();

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (!m_pMethod)
		{
			props::IProperty *pmethod = pobject->GetProperties()->GetPropertyById('C3RM');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"));
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				{
					m_pMethod = (RenderMethod *)(pres->GetData());

					m_pMethod->FindTechnique(_T("g"), m_TechIdx_G);
					m_pMethod->FindTechnique(_T("s"), m_TechIdx_S);
					m_pMethod->FindTechnique(_T("g+s"), m_TechIdx_GS);
				}
			}
		}
		else
		{
			m_pMethod->SetActiveTechnique((flags.IsSet(RF_SHADOW) && pobject->Flags().IsSet(OF_CASTSHADOW)) ? m_TechIdx_S : m_TechIdx_G);
			prend->UseRenderMethod(m_pMethod);
		}
	}

	if (pmod)
	{
		glm::fmat4x4 mat;

		// Handle the case where we don't want models scaling... like lights...
		// they stay the same size in the user-interactable view
		if (!(pobject->Flags().IsSet(OF_NOMODELSCALE)))
		{
			mat = *m_pPos->GetTransformMatrix() * m_Mat;
		}
		else
		{
			glm::fvec3 invscl = 1.0f / *(m_pPos->GetScl());
			
			mat = *m_pPos->GetTransformMatrix() * m_Mat * glm::scale(glm::identity<glm::fmat4x4>(), invscl);
		}

		pmod->Draw(&mat, !flags.IsSet(RF_LOCKMATERIAL));
	}
}


void ModelRendererImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);
	if (!m_pOwner)
		return;

	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	switch (pprop->GetID())
	{
		case 'C3RM':
			m_pMethod = nullptr;
			break;

		case 'MODF':
			m_Mod.second = prm->GetResource(pprop->AsString());
			break;

		case 'MPOS':
		case 'MORI':
		case 'MSCL':
			m_Flags.Set(MRIF_REBUILDMATRIX);
			break;
	}
}


void ModelRendererImpl::SetPos(float x, float y, float z)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_Pos.z = z;
	m_Flags.Set(MRIF_REBUILDMATRIX);
}


const glm::fvec3 *ModelRendererImpl::GetPosVec(glm::fvec3 *pos)
{
	if (!pos)
		return &m_Pos;
	
	*pos = m_Pos;
	return pos;
}


void ModelRendererImpl::SetOriQuat(const glm::fquat *ori)
{
	if (!ori)
		return;

	m_Ori = *ori;
	m_Flags.Set(MRIF_REBUILDMATRIX);
}


const glm::fquat *ModelRendererImpl::GetOriQuat(glm::fquat *ori)
{
	if (!ori)
		return &m_Ori;

	*ori = m_Ori;
	return ori;
}


void ModelRendererImpl::SetScl(float x, float y, float z)
{
	m_Scl.x = x;
	m_Scl.y = y;
	m_Scl.z = z;
	m_Flags.Set(MRIF_REBUILDMATRIX);
}


const glm::fvec3 *ModelRendererImpl::GetScl(glm::fvec3 *scl)
{
	if (!scl)
		return &m_Scl;

	*scl = m_Scl;
	return scl;
}

const glm::fmat4x4 *ModelRendererImpl::GetMatrix(glm::fmat4x4 *mat) const
{
	if (!mat)
		return &m_Mat;

	*mat = m_Mat;
	return mat;
}

const Model *ModelRendererImpl::GetModel() const
{
	Model *pmod = m_Mod.first;
	if (!pmod && m_Mod.second && (m_Mod.second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a model, use it
		pmod = dynamic_cast<Model *>((Model *)(m_Mod.second->GetData()));
	}

	return pmod;
}


bool ModelRendererImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const
{
	bool ret = false;

	const Model *pmod = GetModel();

	if (pmod)
	{
		glm::fmat4x4 mat;

		// Handle the case where we don't want models scaling... like lights...
		// they stay the same size in the user-interactable view
		if (!(m_pOwner->Flags().IsSet(OF_NOMODELSCALE)))
		{
			mat = m_Mat;
		}
		else
		{
			glm::fvec3 invscl = 1.0f / *(m_pPos->GetScl());

			mat = m_Mat * glm::scale(glm::identity<glm::fmat4x4>(), invscl);
		}

		glm::fmat4x4 invt = glm::inverse(mat);
		glm::fmat4x4 invtn = glm::inverse(glm::inverseTranspose(mat));

		glm::vec3 raypos = invt * glm::vec4(pRayPos->x, pRayPos->y, pRayPos->z, 1);
		glm::vec3 raydir = glm::normalize(invtn * glm::vec4(pRayDir->x, pRayDir->y, pRayDir->z, 0));

		size_t meshidx;
		float dist;
		size_t faceidx;
		glm::vec2 uv;

		ret = pmod->Intersect(&raypos, &raydir, &meshidx, &dist, &faceidx, &uv);
		dist = fabs(dist);
		if (ret && pDistance)
			*pDistance = dist;
	}

	return ret;
}
