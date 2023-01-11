// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"

#include <C3ModelRendererImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ModelRenderer, ModelRendererImpl);


ModelRendererImpl::ModelRendererImpl() : m_Pos(0, 0, 0), m_Ori(1, 0, 0, 0), m_Scl(1, 1, 1)
{
	m_pPos = nullptr;
	m_FS_defobj = m_VS_defobj = nullptr;
	m_SP_defobj = nullptr;
	m_Mod = TModOrRes(nullptr, nullptr);
	m_Mat = glm::identity<glm::fmat4x4>();
	m_MatN = glm::identity<glm::fmat4x4>();

	m_Flags.SetAll(MRIF_REBUILDMATRIX);
}


ModelRendererImpl::~ModelRendererImpl()
{
	if (m_SP_defobj)
	{
		m_SP_defobj->Release();
		m_SP_defobj = nullptr;
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
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	props::IProperty *pp;
	pp = props->CreateReferenceProperty(_T("ModelPosition"), 'MPOS', &m_Pos, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	pp = props->CreateReferenceProperty(_T("ModelOrientation"), 'MORI', &m_Ori, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4);
	pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_QUATERNION);
	pp = props->CreateReferenceProperty(_T("ModelScale"), 'MSCL', &m_Scl, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);

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

	if (!pobject->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void ModelRendererImpl::Render(Object *pobject, Object::RenderFlags flags)
{
	assert(pobject);

	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();

	Model *pmod = nullptr;
	if (m_Mod.second && (m_Mod.second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a model, use it
		pmod = dynamic_cast<Model *>((Model *)(m_Mod.second->GetData()));
	}

	ResourceManager *prm = pobject->GetSystem()->GetResourceManager();

	props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (!flags.IsSet(RF_SHADOW) && pobject->Flags().IsSet(OF_CASTSHADOW))
		{
			if (!m_SP_defobj)
			{
				props::IProperty *pvsh = pobject->GetProperties()->GetPropertyById('VSHF');
				props::IProperty *pfsh = pobject->GetProperties()->GetPropertyById('FSHF');
				if (!m_VS_defobj)
				{
					c3::Resource *pres = prm->GetResource(pvsh ? pvsh->AsString() : _T("def-obj.vsh"), rf);
					if (pres)
						m_VS_defobj = (c3::ShaderComponent *)(pres->GetData());
				}

				if (!m_FS_defobj)
				{
					c3::Resource *pres = prm->GetResource(pfsh ? pfsh->AsString() : _T("def-obj.fsh"), rf);
					if (pres)
						m_FS_defobj = (c3::ShaderComponent *)(pres->GetData());
				}

				m_SP_defobj = pobject->GetSystem()->GetRenderer()->CreateShaderProgram();

				if (m_SP_defobj && m_VS_defobj && m_FS_defobj)
				{
					m_SP_defobj->AttachShader(m_VS_defobj);
					m_SP_defobj->AttachShader(m_FS_defobj);
					if (m_SP_defobj->Link() == ShaderProgram::RETURNCODE::RET_OK)
					{
						// anything special to do when the shader links correctly
					}
				}
			}

			prend->UseProgram(m_SP_defobj);
		}
		else
		{
			if (!m_SP_shadowobj)
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
						m_Flags.Clear(OF_CASTSHADOW);
						return;
					}
				}
				else
				{
					m_Flags.Clear(OF_CASTSHADOW);
					return;
				}
			}

			prend->UseProgram(m_SP_shadowobj);
		}
	}

	glm::fmat4x4 mat = *m_pPos->GetTransformMatrix() * m_Mat;
	if (pmod)
	{
		pmod->Draw(&mat);
	}
	else if (!flags.IsSet(RF_SHADOW))
	{
		prend->GetWhiteMaterial()->Apply(m_SP_defobj);
		if (m_SP_defobj)
			m_SP_defobj->ApplyUniforms();
		prend->SetWorldMatrix(&mat);
		prend->GetCubeMesh()->Draw();
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
		case 'VSHF':
			m_VS_defobj = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
			break;

		case 'FSHF':
			m_FS_defobj = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
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
		size_t meshidx;
		float dist;
		size_t faceidx;
		glm::vec2 uv;

		ret = pmod->Intersect(pRayPos, pRayDir, &meshidx, &dist, &faceidx, &uv);
		if (ret && pDistance)
			*pDistance = dist;
	}

	return ret;
}
