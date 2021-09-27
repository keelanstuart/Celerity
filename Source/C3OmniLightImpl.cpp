// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3OmniLightImpl.h>

using namespace c3;


DECLARE_FEATURETYPE(OmniLight, OmniLightImpl);


OmniLightImpl::OmniLightImpl()
{

}


OmniLightImpl::~OmniLightImpl()
{

}


void OmniLightImpl::Release()
{
	delete this;
}


props::TFlags64 OmniLightImpl::Flags()
{
	return m_Flags;
}


bool OmniLightImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	return true;
}


void OmniLightImpl::Update(Object *pobject, float elapsed_time)
{
	// get a positionable feature from the object -- and if we can't, don't proceed
	if (!m_pcpos)
		m_pcpos = dynamic_cast<PositionableImpl *>(pobject->FindFeature(Positionable::Type()));

	if (!m_pcpos)
		return;
}


bool OmniLightImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	return false;
}


void OmniLightImpl::Render(Object *pobject, props::TFlags64 rendflags)
{

}


void OmniLightImpl::PropertyChanged(const props::IProperty *pprop)
{

}


bool OmniLightImpl::HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const
{
	return false;
}


void OmniLightImpl::SetShaderProgram(const ShaderProgram *pshader)
{

}


void OmniLightImpl::SetShaderProgram(const Resource *pshaderres)
{

}


const ShaderProgram *OmniLightImpl::GetShaderProgram() const
{
	return nullptr;
}
