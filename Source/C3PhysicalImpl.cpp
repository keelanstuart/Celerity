// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3PhysicalImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Physical, PhysicalImpl);


PhysicalImpl::PhysicalImpl()
{
	m_LinVel = glm::fvec3(0, 0, 0);
	m_LinAcc = glm::fvec3(0, 0, 0);
	m_maxLinSpeed = FLT_MAX;

	m_RotVel = glm::fvec3(0, 0, 0);
	m_RotAcc = glm::fvec3(0, 0, 0);
	m_maxRotSpeed = glm::fvec3(0, 0, 0);

	m_pPositionable = nullptr;

	m_Flags = 0;
}


PhysicalImpl::~PhysicalImpl()
{
}


void PhysicalImpl::Release()
{

}


props::TFlags64 PhysicalImpl::Flags() const
{
	return m_Flags;
}


bool PhysicalImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	props::IProperty *plvel = propset->CreateReferenceProperty(_T("LinearVelocity"), 'LVEL', &m_LinVel, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);

	props::IProperty *placc = propset->CreateReferenceProperty(_T("LinearAcceleration"), 'LACC', &m_LinAcc, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);

	props::IProperty *plspd = propset->CreateReferenceProperty(_T("MaxLinearSpeed"), 'MXLS', &m_maxLinSpeed, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	props::IProperty *prvel = propset->CreateReferenceProperty(_T("RotationalVelocity"), 'RVEL', &m_RotVel, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (prvel)
	{
		prvel->SetAspect(props::IProperty::PA_ROTATION_DEG);
		prvel->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *pracc = propset->CreateReferenceProperty(_T("RotationalAcceleration"), 'RACC', &m_RotAcc, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pracc)
	{
		pracc->SetAspect(props::IProperty::PA_ROTATION_DEG);
		pracc->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *prspd = propset->CreateReferenceProperty(_T("MaxRotationalSpeed"), 'MXRV', &m_maxRotSpeed, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (prspd)
	{
		prspd->SetAspect(props::IProperty::PA_ROTATION_DEG);
		prspd->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	return true;
}


void PhysicalImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPositionable)
		m_pPositionable = dynamic_cast<Positionable *>(pobject->FindComponent(Positionable::Type()));

	if (m_pPositionable)
	{
		m_LinVel += m_LinAcc * elapsed_time;
		m_pPositionable->AdjustPos(m_LinVel.x * elapsed_time, m_LinVel.y * elapsed_time, m_LinVel.z * elapsed_time);

		m_RotVel += m_RotAcc * elapsed_time;
		m_pPositionable->AdjustYaw(m_RotVel.z);
		m_pPositionable->AdjustPitch(m_RotVel.x);
		m_pPositionable->AdjustRoll(m_RotVel.y);
	}
}


bool PhysicalImpl::Prerender(Object *pobject, Object::RenderFlags flags)
{
	return false;
}


void PhysicalImpl::Render(Object *pobject, Object::RenderFlags flags)
{
}


void PhysicalImpl::PropertyChanged(const props::IProperty *pprop)
{
}




bool PhysicalImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const
{
	return false;
}
