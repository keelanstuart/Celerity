// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3EnvironmentModifierImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(EnvironmentModifier, EnvironmentModifierImpl);


EnvironmentModifierImpl::EnvironmentModifierImpl() :
	m_Gravity(0.0f, 0.0f, -9.8f),
	m_BackgroundColor(Color::fBlack),
	m_AmbColor(Color::fDarkGrey),
	m_SunColor(Color::fNaturalSunlight),
	m_SunDir(glm::normalize(glm::fvec3(0.2, 0.1, -0.7)))
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

	if (pp = ps->CreateReferenceProperty(_T("uBackgroundColor"), 'eBGC', &m_BackgroundColor, props::IProperty::PT_FLOAT_V3))
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB);

	if (pp = ps->CreateReferenceProperty(_T("uAmbientColor"), 'eAMB', &m_AmbColor, props::IProperty::PT_FLOAT_V3))
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR);

	if (pp = ps->CreateReferenceProperty(_T("uSunColor"), 'eSNC', &m_SunColor, props::IProperty::PT_FLOAT_V3))
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR);

	if (pp = ps->CreateReferenceProperty(_T("uSunDirection"), 'eSND', &m_SunDir, props::IProperty::PT_FLOAT_V3))
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_DIRECTION);

	pp = ps->CreateReferenceProperty(_T("Gravity"), 'GRAV', &m_Gravity, props::IProperty::PT_FLOAT_V3);

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

	if (apply)
	{
		Environment *penv = m_pOwner->GetSystem()->GetEnvironment();

		penv->SetAmbientColor(m_AmbColor);
		penv->SetBackgroundColor(m_BackgroundColor);
		penv->SetSunColor(m_SunColor);
		penv->SetSunDirection(m_SunDir);
		penv->SetGravity(m_Gravity);
	}

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


