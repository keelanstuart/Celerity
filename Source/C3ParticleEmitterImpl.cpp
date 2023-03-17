// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ParticleEmitterImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ParticleEmitter, ParticleEmitterImpl);


ParticleEmitterImpl::ParticleEmitterImpl()
{
}


ParticleEmitterImpl::~ParticleEmitterImpl()
{
}


void ParticleEmitterImpl::Release()
{

}


props::TFlags64 ParticleEmitterImpl::Flags() const
{
	return 0;
}


bool ParticleEmitterImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	m_Time = 0;

	m_Shape = EmitterShape::SPHERE;
	props::IProperty *pp = propset->CreateProperty(_T("Shape"), 'PEsh');
	if (pp)
	{
#if 0
		static class ParticleShapeEnumProvider : public props::IProperty::IEnumProvider
		{

			virtual size_t GetNumValues(const IProperty *pprop) const { return EmitterShape::NUM_SHAPES; }

			virtual const TCHAR *GetValue(const IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const = NULL;
		} psep;

		pp->SetEnumProvider(&psep);
#else
		pp->SetEnumStrings(_T("Sphere,Ray,Cone,Cylinder,Plane"));
#endif
		pp->SetEnumVal(m_Shape);
	}

	m_MaxParticles = 100;
	propset->CreateReferenceProperty(_T("MaxParticles"), 'PEmp', &m_MaxParticles, props::IProperty::PROPERTY_TYPE::PT_INT);

	m_Particles.reserve(m_MaxParticles);

	m_EmitRateMin = 0.9f;
	propset->CreateReferenceProperty(_T("EmitRateMin"), 'PEcn', &m_EmitRateMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitRateMax = 1.1f;
	propset->CreateReferenceProperty(_T("EmitRateMin"), 'PEcx', &m_EmitRateMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitSpeedMin = 0.5f;
	propset->CreateReferenceProperty(_T("EmitSpeedMin"), 'PEsn', &m_EmitSpeedMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitSpeedMax = 1.5f;
	propset->CreateReferenceProperty(_T("EmitSpeedMin"), 'PEsx', &m_EmitSpeedMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_ParticleLifeMin = 1.5f;
	pp = propset->CreateReferenceProperty(_T("ParticleLifeMin"), 'PEln', &m_ParticleLifeMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_ParticleLifeMax = 2.0f;
	pp = propset->CreateReferenceProperty(_T("ParticleLifeMax"), 'PElx', &m_ParticleLifeMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_AccelerationMin = -0.1f;
	propset->CreateReferenceProperty(_T("AccelerationMin"), 'PEan', &m_AccelerationMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_AccelerationMax = -0.01f;
	propset->CreateReferenceProperty(_T("AccelerationMax"), 'PEax', &m_AccelerationMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_RollMin = -0.1f;
	pp = propset->CreateReferenceProperty(_T("RollMin"), 'PErn', &m_RollMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_RollMax = 0.1f;
	pp = propset->CreateReferenceProperty(_T("RollMax"), 'PErx', &m_RollMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_StartScale = 1.0f;
	propset->CreateReferenceProperty(_T("StartScale"), 'PEss', &m_StartScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_PeakScale = 2.0f;
	propset->CreateReferenceProperty(_T("PeakScale"), 'PEps', &m_PeakScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EndScale = 0.0f;
	propset->CreateReferenceProperty(_T("EndScale"), 'PEes', &m_EndScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_StartColor = c3::Color::iDarkOrangeFT;
	pp = propset->CreateReferenceProperty(_T("StartColor"), 'PEsc', &m_StartColor, props::IProperty::PROPERTY_TYPE::PT_INT_V4);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_PeakColor = c3::Color::iYellowTT;
	pp = propset->CreateReferenceProperty(_T("PeakColor"), 'PEpc', &m_PeakColor, props::IProperty::PROPERTY_TYPE::PT_INT_V4);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_EndColor = c3::Color::iWhiteFT;
	pp = propset->CreateReferenceProperty(_T("EndColor"), 'PEec', &m_EndColor, props::IProperty::PROPERTY_TYPE::PT_INT_V4);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_InnerRadius = 2.0f;
	propset->CreateReferenceProperty(_T("InnerRadius"), 'PEir', &m_InnerRadius, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_OuterRadius = 2.0f;
	propset->CreateReferenceProperty(_T("OuterRadius"), 'PEor', &m_OuterRadius, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_ConeAngle = 1.571f;
	pp = propset->CreateReferenceProperty(_T("ConeAngle"), 'PEca', &m_ConeAngle, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	pp = propset->CreateProperty(_T("Texture"), 'PEtx');
	pp->SetString(_T("softparticle.tga"));

	m_Peak = 0.25f;
	pp = propset->CreateReferenceProperty(_T("Peak"), 'PEpp', &m_Peak, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_PERCENTAGE);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

//	m_Billboard = true;
//	pp = propset->CreateReferenceProperty(_T("Billboard"), 'PEbb', &m_Billboard, props::IProperty::PROPERTY_TYPE::PT_BOOLEAN);

	return true;
}


void ParticleEmitterImpl::Update(float elapsed_time)
{
}


bool ParticleEmitterImpl::Prerender(Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void ParticleEmitterImpl::Render(Object::RenderFlags flags)
{
	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();

	ShaderProgram *sp = pr->GetBoundsShader();
	pr->UseProgram(sp);
	pr->GetWhiteMaterial()->Apply(sp);

	//pr->GetBoundsMesh()->Draw(c3::Renderer::PrimType::LINELIST);
}


void ParticleEmitterImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();
}


bool ParticleEmitterImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const
{
	return false;
}
