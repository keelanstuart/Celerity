// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3ModelRendererImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ModelRenderer, ModelRendererImpl);


ModelRendererImpl::ModelRendererImpl()
{
	m_pPos = nullptr;
	m_FS_defobj = m_VS_defobj = nullptr;
	m_SP_defobj = nullptr;
	m_Mod = TModOrRes(nullptr, nullptr);
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


props::TFlags64 ModelRendererImpl::Flags()
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

	return true;
}


void ModelRendererImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool ModelRendererImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	if (rendflags.AnySet(Object::OBJFLAG(Object::DRAW) || Object::OBJFLAG(Object::DRAWINEDITOR)))
		return true;

	return false;
}


void ModelRendererImpl::Render(Object *pobject, props::TFlags64 rendflags)
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

	if (!rendflags.IsSet(Object::OBJFLAG(Object::CASTSHADOW)))
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
	}

	if (pmod)
	{
		pmod->Draw(m_pPos->GetTransformMatrix());
	}
	else if (!rendflags.IsSet(Object::OBJFLAG(Object::CASTSHADOW)))
	{
		prend->GetWhiteMaterial()->Apply(m_SP_defobj);
		if (m_SP_defobj)
			m_SP_defobj->ApplyUniforms();
		prend->SetWorldMatrix(m_pPos->GetTransformMatrix());
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
	}
}


bool ModelRendererImpl::HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const
{
	return false;
}


