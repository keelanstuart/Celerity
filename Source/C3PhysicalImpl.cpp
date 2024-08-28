// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3PhysicalImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Physical, PhysicalImpl);


PhysicalImpl::PhysicalImpl()
{
	m_LinVel = glm::fvec3(0, 0, 0);
	m_LinAcc = glm::fvec3(0, 0, 0);
	m_maxLinSpeed = 1000.0f;
	m_LinSpeedFalloff = 50.0f;

	m_RotVel = glm::fvec3(0, 0, 0);
	m_RotAcc = glm::fvec3(0, 0, 0);
	m_maxRotSpeed = glm::fvec3(3.0f, 3.0f, 3.0f);
	m_RotVelFalloff = glm::fvec3(6.0f, 6.0f, 6.0f);

	m_pPositionable = nullptr;

	m_Flags = 0;

	//m_MotionState = new b3DefaultMotionState(b3Transform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	//m_RigidBody = new btRigidBody(1.0f, m_MotionState, m_CollShape.has_value() ? *m_CollShape : nullptr /* get the collision shape from a mesh */);
}


PhysicalImpl::~PhysicalImpl()
{
	//delete m_RigidBody;
	//delete m_MotionState;
	//if (m_CollShape.has_value())
	//	delete *m_CollShape;
}


void PhysicalImpl::Release()
{
	delete this;
}


props::TFlags64 PhysicalImpl::Flags() const
{
	return m_Flags;
}


bool PhysicalImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	props::IProperty *plvel = propset->CreateReferenceProperty(_T("LinearVelocity"), 'LVEL', &m_LinVel, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	props::IProperty *placc = propset->CreateReferenceProperty(_T("LinearAcceleration"), 'LACC', &m_LinAcc, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	props::IProperty *plvelff = propset->CreateReferenceProperty(_T("LinearSpeedFalloffFactor"), 'LSFF', &m_LinSpeedFalloff, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	props::IProperty *plspdmax = propset->CreateReferenceProperty(_T("MaxLinearSpeed"), 'MXLS', &m_maxLinSpeed, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	props::IProperty *prvel = propset->CreateReferenceProperty(_T("RotationalVelocity"), 'RVEL', &m_RotVel, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (prvel)
	{
		prvel->SetAspect(props::IProperty::PA_ROTATION_DEG);
		prvel->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *pdpos = propset->CreateReferenceProperty(_T("DeltaPosition"), 'DPOS', &m_DeltaPos, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pdpos)
	{
		pdpos->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED) | props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	props::IProperty *pracc = propset->CreateReferenceProperty(_T("RotationalAcceleration"), 'RACC', &m_RotAcc, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pracc)
	{
		pracc->SetAspect(props::IProperty::PA_ROTATION_DEG);
		pracc->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *prvelff = propset->CreateReferenceProperty(_T("RotationalVelocityFalloffFactor"), 'RVFF', &m_RotVelFalloff, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);

	props::IProperty *prspdmax = propset->CreateReferenceProperty(_T("MaxRotationalSpeed"), 'MXRS', &m_maxRotSpeed, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (prspdmax)
	{
		prspdmax->SetAspect(props::IProperty::PA_ROTATION_DEG);
		prspdmax->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *prmass = propset->CreateReferenceProperty(_T("Mass"), 'MASS', &m_Mass, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	return true;
}


void PhysicalImpl::Update(float elapsed_time)
{
	if (!m_pPositionable)
		m_pPositionable = dynamic_cast<Positionable *>(m_pOwner->FindComponent(Positionable::Type()));

	if (m_pPositionable)
	{
		m_LinVel = glm::lerp(m_LinVel, glm::fvec3(0, 0, 0), glm::clamp<float>(m_LinSpeedFalloff * elapsed_time, 0, 1));
		m_LinVel += m_LinAcc * elapsed_time;
		float speed = glm::length(m_LinVel);
		if (speed > m_maxLinSpeed)
			m_LinVel = normalize(m_LinVel) * m_maxLinSpeed;

		m_RotVel = glm::mix(m_RotVel, glm::fvec3(0, 0, 0), m_RotVelFalloff * elapsed_time);
		m_RotVel += m_RotAcc * elapsed_time;
		m_RotVel = glm::min(glm::max(-m_maxRotSpeed, m_RotVel), m_maxRotSpeed);

		glm::fvec3 f, r, u;
		m_pPositionable->GetFacingVector(&f);
		m_pPositionable->GetLocalRightVector(&r);
		m_pPositionable->GetLocalUpVector(&u);

		m_DeltaPos =  f * m_LinVel.y * elapsed_time;
		m_DeltaPos += r * m_LinVel.x * elapsed_time;
		m_DeltaPos += u * m_LinVel.z * elapsed_time;

		m_pPositionable->AdjustPos(m_DeltaPos.x, m_DeltaPos.y, m_DeltaPos.z);
		m_pPositionable->AdjustYaw(m_RotVel.z * elapsed_time);
		m_pPositionable->AdjustPitch(m_RotVel.x * elapsed_time);
		m_pPositionable->AdjustRoll(m_RotVel.y * elapsed_time);

		//m_MotionState->setWorldTransform(m_pPositionable->GetTransformMatrix());
	}
}


bool PhysicalImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	return false;
}


void PhysicalImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
}


void PhysicalImpl::PropertyChanged(const props::IProperty *pprop)
{
	switch (pprop->GetID())
	{
		case 'LACC':
			//m_pOwner->GetSystem()->GetLog()->Print(_T("*"));
			break;
		case 'LVEL':
			//m_pOwner->GetSystem()->GetLog()->Print(_T("$"));
			break;
		case 'RACC':
			//m_pOwner->GetSystem()->GetLog()->Print(_T("!"));
			break;
		case 'RVEL':
			//m_pOwner->GetSystem()->GetLog()->Print(_T("@"));
			break;
	}
}


bool PhysicalImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const
{
	return false;
}


void PhysicalImpl::SetLinVel(float x, float y, float z)
{
	m_LinVel.x = x;
	m_LinVel.y = y;
	m_LinVel.z = z;
}


void PhysicalImpl::SetLinVelVec(const glm::fvec3 *lvel)
{
	m_LinVel = lvel ? *lvel : glm::fvec3(0, 0, 0);
}


void PhysicalImpl::SetLinVelX(float x)
{
	m_LinVel.x = x;
}


void PhysicalImpl::SetLinVelY(float y)
{
	m_LinVel.y = y;
}


void PhysicalImpl::SetLinVelZ(float z)
{
	m_LinVel.z = z;
}


const glm::fvec3 *PhysicalImpl::GetLinVel(glm::fvec3 *velptr)
{
	if (velptr)
	{
		*velptr = m_LinVel;
		return velptr;
	}

	return &m_LinVel;
}


float PhysicalImpl::GetLinVelX()
{
	return m_LinVel.x;
}


float PhysicalImpl::GetLinVelY()
{
	return m_LinVel.y;
}


float PhysicalImpl::GetLinVelZ()
{
	return m_LinVel.z;
}


void PhysicalImpl::AdjustLinVel(float xadj, float yadj, float zadj)
{
	m_LinVel.x += xadj;
	m_LinVel.y += yadj;
	m_LinVel.z += zadj;
}


void PhysicalImpl::SetMaxLinSpeed(float speed)
{
	m_maxLinSpeed = speed;
}


float PhysicalImpl::GetMaxLinSpeed()
{
	return m_maxLinSpeed;
}


void PhysicalImpl::SetLinSpeedFalloffFactor(float factor)
{
	m_LinSpeedFalloff = factor;
}


void PhysicalImpl::SetLinAcc(float x, float y, float z)
{
	m_LinAcc.x = x;
	m_LinAcc.y = y;
	m_LinAcc.z = z;
}


void PhysicalImpl::SetLinAccVec(const glm::fvec3 *lacc)
{
	m_LinAcc = lacc ? *lacc : glm::fvec3(0, 0, 0);
}


void PhysicalImpl::SetLinAccX(float x)
{
	m_LinAcc.x = x;
}


void PhysicalImpl::SetLinAccY(float y)
{
	m_LinAcc.y = y;
}


void PhysicalImpl::SetLinAccZ(float z)
{
	m_LinAcc.z = z;
}


const glm::fvec3 *PhysicalImpl::GetLinAcc(glm::fvec3 *laccptr)
{
	if (laccptr)
	{
		*laccptr = m_LinAcc;
		return laccptr;
	}

	return &m_LinAcc;
}


float PhysicalImpl::GetLinAccX()
{
	return m_LinAcc.x;
}


float PhysicalImpl::GetLinAccY()
{
	return m_LinAcc.y;
}


float PhysicalImpl::GetLinAccZ()
{
	return m_LinAcc.z;
}


void PhysicalImpl::AdjustLinAcc(float xadj, float yadj, float zadj)
{
	m_LinAcc.x += xadj;
	m_LinAcc.y += yadj;
	m_LinAcc.z += zadj;
}


void PhysicalImpl::SetRotVel(float y, float p, float r)
{
	m_RotVel.z = y;
	m_RotVel.x = p;
	m_RotVel.y = r;
}


void PhysicalImpl::SetRotVelVec(const glm::fvec3 *rvel)
{
	m_RotVel = rvel ? *rvel : glm::fvec3(0, 0, 0);
}


void PhysicalImpl::SetRotVelY(float y)
{
	m_RotVel.z = y;
}


void PhysicalImpl::SetRotVelP(float p)
{
	m_RotVel.x = p;
}


void PhysicalImpl::SetRotVelR(float r)
{
	m_RotVel.y = r;
}


const glm::fvec3 *PhysicalImpl::GetRotVel(glm::fvec3 *rvelptr)
{
	if (rvelptr)
	{
		*rvelptr = m_RotVel;
		return rvelptr;
	}

	return &m_RotVel;
}


float PhysicalImpl::GetRotVelY()
{
	return m_RotVel.z;
}


float PhysicalImpl::GetRotVelP()
{
	return m_RotVel.x;
}


float PhysicalImpl::GetRotVelR()
{
	return m_RotVel.y;
}


void PhysicalImpl::AdjustRotVel(float yadj, float padj, float radj)
{
	m_RotVel.z += yadj;
	m_RotVel.x += padj;
	m_RotVel.y += radj;
}


void PhysicalImpl::SetMaxRotSpeed(glm::fvec3 *speedptr)
{
	m_maxRotSpeed = speedptr ? *speedptr : glm::fvec3(0, 0, 0);
}


glm::fvec3 *PhysicalImpl::GetMaxRotSpeed(glm::fvec3 *speedptr)
{
	if (speedptr)
	{
		*speedptr = m_maxRotSpeed;
		return speedptr;
	}

	return &m_maxRotSpeed;
}


void PhysicalImpl::SetRotVelFalloffFactor(glm::fvec3 *factorptr)
{
	m_RotVelFalloff = factorptr ? *factorptr : glm::fvec3(0, 0, 0);
}


glm::fvec3 *PhysicalImpl::GetRotVelFalloffFactor(glm::fvec3 *factorptr)
{
	if (factorptr)
	{
		*factorptr = m_RotVelFalloff;
		return factorptr;
	}

	return &m_RotVelFalloff;
}


void PhysicalImpl::SetRotAcc(float y, float p, float r)
{
	m_RotAcc.z = y;
	m_RotAcc.x = p;
	m_RotAcc.y = r;
}


void PhysicalImpl::SetRotAccVec(const glm::fvec3 *racc)
{
	m_RotAcc = racc ? *racc : glm::fvec3(0, 0, 0);
}


void PhysicalImpl::SetRotAccY(float y)
{
	m_RotAcc.z = y;
}


void PhysicalImpl::SetRotAccP(float p)
{
	m_RotAcc.x = p;
}


void PhysicalImpl::SetRotAccR(float r)
{
	m_RotAcc.y = r;
}


const glm::fvec3 *PhysicalImpl::GetRotAcc(glm::fvec3 *raccptr)
{
	if (raccptr)
	{
		*raccptr = m_RotAcc;
		return raccptr;
	}

	return &m_RotAcc;
}


float PhysicalImpl::GetRotAccY()
{
	return m_RotAcc.z;
}


float PhysicalImpl::GetRotAccP()
{
	return m_RotAcc.x;
}


float PhysicalImpl::GetRotAccR()
{
	return m_RotAcc.y;
}


void PhysicalImpl::AdjustRotAcc(float yadj, float padj, float radj)
{
	m_RotAcc.z += yadj;
	m_RotAcc.x += padj;
	m_RotAcc.y += radj;
}


