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

	bool apply = m_pOwner->GetParent() ? false : true;

	System *psys = m_pOwner->GetSystem();

	Object *pco = psys->GetGlobalObjectRegistry()->GetRegisteredObject(GlobalObjectRegistry::OD_CAMERA_ROOT);
	Positionable *pcp = pco ? (Positionable *)pco->FindComponent(Positionable::Type()) : nullptr;
	if (pcp)
	{
		glm::fmat4x4 tm = *m_pPos->GetTransformMatrix();
		glm::fmat4x4 tn = *m_pPos->GetTransformMatrixNormal();

		static glm::fvec3 p[6] ={ glm::fvec3(1, 0, 0), glm::fvec3(0, 1, 0), glm::fvec3(0, 0, 1), glm::fvec3(-1, 0, 0), glm::fvec3(0, -1, 0), glm::fvec3(0, 0, -1) };

		size_t i;
		for (i = 0; i < 6; i++)
		{
			glm::fvec3 q = glm::fvec4(p[i], 1) * tm;
			glm::fvec3 n = glm::fvec4(p[i], 0) * tn;
			glm::fvec3 d = q - *(pcp->GetPosVec());
			if (glm::dot(n, d) <= 0)
				break;
		}

		apply = (i >= 6);
	}

	Environment *penv = m_pOwner->GetSystem()->GetEnvironment();
	props::IPropertySet *ps = m_pOwner->GetProperties();

	if (apply && !m_bCameraInside)
	{
		if (props::IProperty *pp = ps->GetPropertyById('eAMB'))
			penv->PushAmbientColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (props::IProperty *pp = ps->GetPropertyById('eBGC'))
			penv->PushBackgroundColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (props::IProperty *pp = ps->GetPropertyById('eSNC'))
			penv->PushSunColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (props::IProperty *pp = ps->GetPropertyById('eSND'))
			penv->PushSunDirection(*((glm::fvec3 *)(pp->AsVec3F())));
	}
	else if (!apply && m_bCameraInside)
	{
		if (ps->GetPropertyById('eAMB'))
			penv->PopAmbientColor();

		if (ps->GetPropertyById('eBGC'))
			penv->PopBackgroundColor();

		if (ps->GetPropertyById('eSNC'))
			penv->PopSunColor();

		if (ps->GetPropertyById('eSND'))
			penv->PopSunDirection();
	}
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


