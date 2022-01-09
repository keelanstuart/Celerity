// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3EnvironmentModifierImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(EnvironmentModifier, EnvironmentModifierImpl);


EnvironmentModifierImpl::EnvironmentModifierImpl()
{
	m_pPos = nullptr;
}


EnvironmentModifierImpl::~EnvironmentModifierImpl()
{
}


void EnvironmentModifierImpl::Release()
{
	delete this;
}


props::TFlags64 EnvironmentModifierImpl::Flags()
{
	return m_Flags;
}


bool EnvironmentModifierImpl::Initialize(Object *pobject)
{
	if (nullptr == pobject)
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	return true;
}


void EnvironmentModifierImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool EnvironmentModifierImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	if (rendflags.AnySet(Object::OBJFLAG(Object::DRAW) || Object::OBJFLAG(Object::DRAWINEDITOR)))
		return true;

	return false;
}


void EnvironmentModifierImpl::Render(Object *pobject, props::TFlags64 rendflags)
{
	assert(pobject);

	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();
}


void EnvironmentModifierImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);
}


bool EnvironmentModifierImpl::HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const
{
	return false;
}


