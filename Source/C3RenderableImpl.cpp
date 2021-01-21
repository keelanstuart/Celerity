// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3RenderableImpl.h>

using namespace c3;


DECLARE_FEATURETYPE(Renderable, RenderableImpl);


RenderableImpl::RenderableImpl()
{

}


RenderableImpl::~RenderableImpl()
{

}


void RenderableImpl::Release()
{
	delete this;
}


props::TFlags64 RenderableImpl::Flags()
{
	return m_Flags;
}


bool RenderableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	return true;
}


void RenderableImpl::Update(Object *pobject, float elapsed_time)
{
	// get a positionable feature from the object -- and if we can't, don't proceed
	if (!m_pcpos)
		m_pcpos = dynamic_cast<PositionableImpl *>(pobject->FindFeature(Positionable::Type()));

	if (!m_pcpos)
		return;
}


bool RenderableImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	return false;
}


void RenderableImpl::Render(Object *pobject, props::TFlags64 rendflags)
{

}


void RenderableImpl::PropertyChanged(const props::IProperty *pprop)
{

}


bool RenderableImpl::HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const
{
	return false;
}


void RenderableImpl::SetMesh(const Mesh *pmesh)
{

}


const Mesh *RenderableImpl::GetMesh() const
{
	return nullptr;
}


void RenderableImpl::SetTexture(const Texture *ptex)
{

}


const Texture *RenderableImpl::GetTexture() const
{
	return nullptr;
}


void RenderableImpl::SetMaterial(const Material *pmaterial)
{

}


const Material *RenderableImpl::GetMaterial() const
{
	return nullptr;
}


void RenderableImpl::SetShaderProgram(const ShaderProgram *pshader)
{

}


const ShaderProgram *RenderableImpl::GetShaderProgram() const
{
	return nullptr;
}
