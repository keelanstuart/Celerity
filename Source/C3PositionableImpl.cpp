// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3PositionableImpl.h>

using namespace c3;


DECLARE_COMPORTMENTTYPE(Positionable, PositionableImpl);


PositionableImpl::PositionableImpl()
{
	m_RecalcMatrix = false;
	m_Mat = glm::identity<glm::mat4x4>();
}


PositionableImpl::~PositionableImpl()
{
}


void PositionableImpl::Release()
{

}


props::TFlags64 PositionableImpl::Flags()
{
	return props::TFlags64(0);
}


bool PositionableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	m_ppos = propset->CreateReferenceProperty(_T("Position"), 'POS', &m_Pos, props::IProperty::PROPERTY_TYPE::PT_REAL_V3);
	if (m_ppos)
	{
		m_ppos->SetVec3F(props::TVec3F(0, 0, 0));
	}

	m_pori = propset->CreateReferenceProperty(_T("Orientation"), 'ORI', &m_Ori, props::IProperty::PROPERTY_TYPE::PT_REAL_V4);
	if (m_pori)
	{
		m_pori->SetVec4F(props::TVec4F(0, 0, 0, 1));
		m_pori->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_QUATERNION);
	}

	m_pscl = propset->CreateReferenceProperty(_T("Scale"), 'SCL', &m_Scl, props::IProperty::PROPERTY_TYPE::PT_REAL_V3);
	if (m_pscl)
	{
		m_pscl->SetVec3F(props::TVec3F(1, 1, 1));
	}

	return true;
}


void PositionableImpl::Update(Object *pobject, float elapsed_time)
{
	if (m_RecalcMatrix)
	{
		glm::mat4x4 tmp;

		// Next rotate...
		tmp = (glm::mat4x4)m_Ori;

		// Recalculate our facing vector in between...
		m_Facing = glm::normalize(glm::vec4(0, 1, 0, 0) * tmp);

		// Recalculate our local up vector after that...
		m_LocalUp = glm::normalize(glm::vec4(0, 0, 1, 0) * tmp);

		// Recalculate the local left vector
		m_LocalLeft = glm::normalize(glm::cross(m_LocalUp, m_Facing));

		// Scale first, then rotate...
		m_Mat = glm::scale(glm::identity<glm::mat4x4>(), m_Scl) * tmp;

		// Next translate for the rotational center...
		//m_Mat = m_Mat * glm::translate(glm::identity<glm::mat4x4>(), m_RotCenter);

		// Then translate last... 
		m_Mat = m_Mat * glm::translate(glm::identity<glm::mat4x4>(), m_Pos);

		//m_Bounds.Align(&m_Mat);

		m_RecalcMatrix = false;
	}
}


bool PositionableImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	Renderer *pr = pobject->GetSystem()->GetRenderer();

	pr->SetWorldMatrix(&m_Mat);

	return true;
}


void PositionableImpl::Render(Object *pobject, props::TFlags64 rendflags)
{
	Renderer *pr = pobject->GetSystem()->GetRenderer();
	Mesh *pbm = pr->GetBoundsMesh();
	if (pbm)
	{
		pbm->Draw();
	}
}


void PositionableImpl::PropertyChanged(const props::IPropertySet *ppropset, const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();
	if ((fcc == 'POS') || (fcc == 'ORI') || (fcc == 'SCL'))
		m_RecalcMatrix = true;
}


void PositionableImpl::SetPos(float x, float y, float z)
{

}


void PositionableImpl::SetPosVec(const C3VEC3 *pos)
{

}


void PositionableImpl::SetPosX(float x)
{

}


void PositionableImpl::SetPosY(float y)
{

}


void PositionableImpl::SetPosZ(float z)
{

}


const C3VEC3 *PositionableImpl::GetPosVec(C3VEC3 *pos)
{
	if (pos)
	{
		*pos = m_Pos;
		return pos;
	}

	return &m_Pos;
}


float PositionableImpl::GetPosX()
{
	return m_Pos.x;
}


float PositionableImpl::GetPosY()
{
	return m_Pos.y;
}


float PositionableImpl::GetPosZ()
{
	return m_Pos.z;
}


void PositionableImpl::AdjustPos(float xadj, float yadj, float zadj)
{
	m_Pos.x += xadj;
	m_Pos.y += yadj;
	m_Pos.z += zadj;
}


void PositionableImpl::SetRotCenter(float x, float y, float z)
{

}


void PositionableImpl::SetRotCenterVec(const C3VEC3 *rotc)
{

}


void PositionableImpl::SetRotCenterX(float x)
{

}


void PositionableImpl::SetRotCenterY(float y)
{

}


void PositionableImpl::SetRotCenterZ(float z)
{

}


const C3VEC3 *PositionableImpl::GetRotCenterVec(C3VEC3 *rotc)
{
	return nullptr;
}


float PositionableImpl::GetRotCenterX()
{
	return 0.0f;
}


float PositionableImpl::GetRotCenterY()
{
	return 0.0f;
}


float PositionableImpl::GetRotCenterZ()
{
	return 0.0f;
}


void PositionableImpl::AdjustRotCenter(float dx, float dy, float dz)
{

}


void PositionableImpl::SetOri(float x, float y, float z, float w)
{
	m_Ori.x = x;
	m_Ori.y = y;
	m_Ori.z = z;
	m_Ori.w = w;
}


void PositionableImpl::SetOriQuat(const C3QUAT *ori)
{
	if (!ori)
		return;

	m_Ori = *ori;
}


void PositionableImpl::SetYawPitchRoll(float y, float p, float r)
{

}


const C3QUAT *PositionableImpl::GetOriQuat(C3QUAT *ori)
{
	if (ori)
	{
		*ori = m_Ori;
		return ori;
	}

	return &m_Ori;
}


float PositionableImpl::GetYaw()
{
	return 0.0f;
}


float PositionableImpl::GetPitch()
{
	return 0.0f;
}


float PositionableImpl::GetRoll()
{
	return 0.0f;
}


void PositionableImpl::AdjustYaw(float dy)
{

}


void PositionableImpl::AdjustPitch(float dp)
{

}


void PositionableImpl::AdjustRoll(float dr)
{

}


const C3VEC3 *PositionableImpl::GetFacingVector(C3VEC3 *vec)
{
	return nullptr;
}


const C3VEC3 *PositionableImpl::GetLocalUpVector(C3VEC3 *vec)
{
	return nullptr;
}


const C3VEC3 *PositionableImpl::GetLocalLeftVector(C3VEC3 *vec)
{
	return nullptr;
}


void PositionableImpl::SetScl(float x, float y, float z)
{
	m_Scl.x = x;
	m_Scl.y = y;
	m_Scl.z = z;
}


void PositionableImpl::SetSclVec(const C3VEC3 *scl)
{
	if (!scl)
		return;

	m_Scl = *scl;
}


void PositionableImpl::SetSclX(float x)
{
	m_Scl.x = x;
}


void PositionableImpl::SetSclY(float y)
{
	m_Scl.y = y;
}


void PositionableImpl::SetSclZ(float z)
{
	m_Scl.z = z;
}


const C3VEC3 *PositionableImpl::GetScl(C3VEC3 *scl)
{
	if (scl)
	{
		*scl = m_Scl;
		return scl;
	}

	return &m_Scl;
}


float PositionableImpl::GetSclX()
{
	return m_Scl.x;
}


float PositionableImpl::GetSclY()
{
	return m_Scl.y;
}


float PositionableImpl::GetSclZ()
{
	return m_Scl.z;
}


void PositionableImpl::AdjustScl(float dx, float dy, float dz)
{
	m_Scl.x += dx;
	m_Scl.y += dy;
	m_Scl.z += dz;
}


