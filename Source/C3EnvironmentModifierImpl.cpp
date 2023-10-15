// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


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


props::TFlags64 EnvironmentModifierImpl::Flags() const
{
	return m_Flags;
}


bool EnvironmentModifierImpl::Initialize(Object *pobject)
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


void EnvironmentModifierImpl::Update(float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool EnvironmentModifierImpl::Prerender(Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void EnvironmentModifierImpl::Render(Object::RenderFlags flags)
{
	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();
}


void EnvironmentModifierImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);
}


bool EnvironmentModifierImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const
{
	return false;
}


