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

	props::IPropertySet *ps = pobject->GetProperties();
	if (!ps)
		return false;

	props::IProperty *pp;

	if ((pp = ps->GetPropertyById('eBGC')) || (pp = ps->CreateProperty(_T("uBackgroundColor"), 'eBGC')))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->SetVec4F(*(props::TVec4F *)&Color::fBlack);
	}

#if 1
	if ((pp = ps->GetPropertyById('eAMB')) || (pp = ps->CreateProperty(_T("uAmbientColor"), 'eAMB')))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR);
		pp->SetVec3F(props::SVec3(0.1f, 0.1f, 0.1f));
	}

	if ((pp = ps->GetPropertyById('eSNC')) || (pp = ps->CreateProperty(_T("uSunColor"), 'eSNC')))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR);
		pp->SetVec3F(props::SVec3(1.0f, 0.97f, 0.92f));
	}

	if ((pp = ps->GetPropertyById('eSND')) || (pp = ps->CreateProperty(_T("uSunDirection"), 'eSND')))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_DIRECTION);
		pp->SetVec3F(props::SVec3(-0.1f, -0.1f, -1.0f));
	}
#endif

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

#if 0
	if (apply && !m_bCameraInside)
	{
		props::IProperty *pp;
		if (pp = ps->GetPropertyById('eAMB'))
			penv->PushAmbientColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (pp = ps->GetPropertyById('eBGC'))
			penv->PushBackgroundColor(*((glm::fvec4 *)(pp->AsVec4F())));

		if (pp = ps->GetPropertyById('eSNC'))
			penv->PushSunColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (pp = ps->GetPropertyById('eSND'))
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
#else
	if (apply)
	{
		props::IProperty *pp;
		if (pp = ps->GetPropertyById('eAMB'))
			penv->SetAmbientColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (pp = ps->GetPropertyById('eBGC'))
			penv->SetBackgroundColor(*((glm::fvec4 *)(pp->AsVec4F())));

		if (pp = ps->GetPropertyById('eSNC'))
			penv->SetSunColor(*((glm::fvec3 *)(pp->AsVec3F())));

		if (pp = ps->GetPropertyById('eSND'))
			penv->SetSunDirection(*((glm::fvec3 *)(pp->AsVec3F())));
	}
#endif

	m_bCameraInside = apply;
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


