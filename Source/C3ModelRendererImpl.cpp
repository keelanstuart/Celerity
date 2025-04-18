// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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
	m_Inst = nullptr;

	m_Flags.SetAll(MRIF_REBUILDMATRIX);
}


ModelRendererImpl::~ModelRendererImpl()
{
	if (m_Inst)
	{
		m_Inst->Release();
		m_Inst = nullptr;
	}
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
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(m_pOwner->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = m_pOwner->GetProperties();
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

	pp = props->GetPropertyById('MODF');
	if (!pp)
	{
		pp = props->CreateProperty(_T("ModelFile"), 'MODF');
		if (pp)
		{
			pp->SetString(_T("[sphere_lo.model]"));
			pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		}
	}
	if (pp)
		m_Mod.second = pobject->GetSystem()->GetResourceManager()->GetResource(pp->AsString());

	return true;
}


void ModelRendererImpl::Update(float elapsed_time)
{
}


bool ModelRendererImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (!m_pMethod)
	{
		ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();
		m_TechIdx_Override.reset();

		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('C3RM');
		if (prm)
		{
			c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				m_pMethod = (RenderMethod *)(pres->GetData());

				std::function<void(size_t)> _SetTechIdx = [&](size_t tidx)
				{
					if (tidx < m_pMethod->GetNumTechniques())
					{
						if (m_TechIdx_Override.has_value())
							*m_TechIdx_Override = tidx;
						else
							m_TechIdx_Override = std::make_optional<size_t>(tidx);
					}
					else
					{
						m_TechIdx_Override.reset();
					}
				};

				if (props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('C3RT'))
				{
					size_t t;
					switch (pp->GetType())
					{
						case props::IProperty::PROPERTY_TYPE::PT_INT:
						{
							t = pp->AsInt();
							_SetTechIdx(t);
							break;
						}

						case props::IProperty::PROPERTY_TYPE::PT_STRING:
						{
							t = -1;
							m_pMethod->FindTechnique(pp->AsString(), t);
							_SetTechIdx(t);
							break;
						}
					}
				}

				m_pMethod->FindTechnique(_T("g"), m_TechIdx_G);
				m_pMethod->FindTechnique(_T("s"), m_TechIdx_S);
				m_pMethod->FindTechnique(_T("g+s"), m_TechIdx_GS);
			}
		}
	}

	if (flags.IsSet(RF_FORCE))
		return true;

	if (flags.IsSet(RF_EFFECT))
		return false;

	if (flags.IsSet(RF_LIGHT))
		return false;

	size_t t;
	if (m_TechIdx_Override.has_value())
		t = *m_TechIdx_Override;
	else
		t = (flags.IsSet(RF_SHADOW) && m_pOwner->Flags().IsSet(OF_CASTSHADOW)) ? m_TechIdx_S : m_TechIdx_G;

	RenderMethod::Technique *ptech = m_pMethod ? m_pMethod->GetTechnique(t) : nullptr;
	if (ptech && (draworder == ptech->GetDrawOrder()))
	{
		if (m_pOwner->Flags().IsSet(OF_DRAW))
			return true;

		if (flags.IsSet(RF_EDITORDRAW) && m_pOwner->Flags().IsSet(OF_DRAWINEDITOR))
			return true;
	}

	return false;
}


void ModelRendererImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

	Model *pmod = nullptr;
	if (m_Mod.second && (m_Mod.second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a model, use it
		pmod = dynamic_cast<Model *>((Model *)(m_Mod.second->GetData()));
		if (pmod)
		{

			// if we had some model instance data already, then free it now
			if (m_Inst && (m_Inst->GetSourceModel() != pmod))
			{
				m_Inst->Release();
				m_Inst = nullptr;
			}

			// make new instance data for this new model
			if (!m_Inst)
				m_Inst = (ModelImpl::InstanceDataImpl *)(((ModelImpl *)pmod)->CloneInstanceData());
		}
	}

	if (pmod)
	{
		if (!flags.IsSet(RF_LOCKSHADER) && m_pMethod)
		{
			size_t t;
			if (m_TechIdx_Override.has_value())
				t = *m_TechIdx_Override;
			else
				t = (flags.IsSet(RF_SHADOW) && m_pOwner->Flags().IsSet(OF_CASTSHADOW)) ? m_TechIdx_S : m_TechIdx_G;

			m_pMethod->SetActiveTechnique(t);
			prend->UseRenderMethod(m_pMethod);
			((RendererImpl *)prend)->SetModelInstanceData((const Model::InstanceData *)m_Inst);
		}

		glm::fmat4x4 mat = *pmat * *GetMatrix();

		pmod->Draw(&mat, !flags.IsSet(RF_LOCKMATERIAL), (Model::InstanceData *)m_Inst, flags.AnySet(RF_EDITORDRAW | RF_FORCE));
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
		case 'C3RT':
			// a change of override technique will also clear the RenderMethod
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

	m_Ori = normalize(*ori);
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
	if (m_Flags.IsSet(MRIF_REBUILDMATRIX))
	{
		m_Mat = glm::translate(m_Pos) * ((glm::fmat4x4)(glm::normalize(m_Ori)) * glm::scale(m_Scl));

		m_Flags.Clear(MRIF_REBUILDMATRIX);
	}

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


void ModelRendererImpl::SetModelByResName(const TCHAR *resname)
{
	m_Mod.first = nullptr;
	m_Mod.second = m_pOwner->GetSystem()->GetResourceManager()->GetResource(resname);
	if (props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('MODF'))
		pp->SetString(resname);
}


void ModelRendererImpl::SetModel(Model *pmod)
{
	m_Mod.first = pmod;
	if (props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('MODF'))
		pp->SetString(_T(""));
	m_Mod.second = nullptr;
}


Model::InstanceData *ModelRendererImpl::GetModelInstanceData()
{
	return (Model::InstanceData *)m_Inst;
}


bool ModelRendererImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, bool force) const
{
	bool ret = false;

	const Model *pmod = GetModel();

	if (pmod)
	{
		size_t meshidx;
		float dist;
		size_t faceidx;
		glm::vec2 uv;

		glm::fmat4x4 mat = pmat ? (*pmat * *GetMatrix()) : *GetMatrix();

		ret = pmod->Intersect(pRayPos, pRayDir, &mat, &meshidx, &dist, &faceidx, &uv, (const Model::InstanceData *)m_Inst, force);
		if (ret && pDistance)
		{
			if (dist < *pDistance)
				*pDistance = dist;
			else
				ret = false;
		}
	}

	return ret;
}
