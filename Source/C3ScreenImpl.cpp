// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3ScreenImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Screen, ScreenImpl);


ScreenImpl::ScreenImpl() : m_Reg(nullptr)
{
	m_pOwner = nullptr;
}


ScreenImpl::~ScreenImpl()
{
}


GlobalObjectRegistry *ScreenImpl::GetObjectRegistry()
{
	return &m_Reg;
}


void ScreenImpl::Release()
{
	delete this;
}


props::TFlags64 &ScreenImpl::Flags()
{
	return m_Flags;
}


bool ScreenImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	// TODO: create your required properties here...

	return true;
}


void ScreenImpl::Update(float elapsed_time)
{
}


bool ScreenImpl::Prerender(RenderFlags flags, int draworder)
{
	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void ScreenImpl::Render(RenderFlags rendflags, const glm::fmat4x4 *pmat)
{
}


void ScreenImpl::PropertyChanged(const props::IProperty *pprop)
{
}


bool ScreenImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance, glm::fvec3 *pNormal, bool force) const
{
	return false;
}
