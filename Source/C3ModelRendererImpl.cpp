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

	if (!m_SP_defobj)
	{
		ResourceManager *prm = pobject->GetSystem()->GetResourceManager();

		props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

		props::IProperty *pvsh = pobject->GetProperties()->GetPropertyById('VSHF');
		props::IProperty *pfsh = pobject->GetProperties()->GetPropertyById('FSHF');
		if (!m_VS_defobj)
			m_VS_defobj = (c3::ShaderComponent *)((prm->GetResource(pvsh ? pvsh->AsString() : _T("def-obj.vsh"), rf))->GetData());

		if (!m_FS_defobj)
			m_FS_defobj = (c3::ShaderComponent *)((prm->GetResource(pfsh ? pfsh->AsString() : _T("def-obj.fsh"), rf))->GetData());

		m_SP_defobj = pobject->GetSystem()->GetRenderer()->CreateShaderProgram();

		if (m_SP_defobj && m_VS_defobj && m_FS_defobj)
		{
			m_SP_defobj->AttachShader(m_VS_defobj);
			m_SP_defobj->AttachShader(m_FS_defobj);
			m_SP_defobj->Link();
		}
	}

	if (m_SP_defobj)
		prend->UseProgram(m_SP_defobj);

	if (pmod)
	{
		pmod->Draw(m_pPos->GetTransformMatrix());
	}
	else
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


