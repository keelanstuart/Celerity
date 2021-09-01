// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3PositionableImpl.h>

using namespace c3;


DECLARE_FEATURETYPE(Positionable, PositionableImpl);


PositionableImpl::PositionableImpl()
{
	m_Mat = glm::identity<glm::fmat4x4>();
	m_Ori = glm::identity<glm::fquat>();
}


PositionableImpl::~PositionableImpl()
{
}


void PositionableImpl::Release()
{

}


props::TFlags64 PositionableImpl::Flags()
{
	return m_Flags;
}


bool PositionableImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	m_ppos = propset->CreateReferenceProperty(_T("Position"), 'POS', &m_Pos, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (m_ppos)
	{
		m_ppos->SetVec3F(props::TVec3F(0, 0, 0));
	}

	m_pori = propset->CreateReferenceProperty(_T("Orientation"), 'ORI', &m_Ori, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4);
	if (m_pori)
	{
		m_pori->SetVec4F(props::TVec4F(m_Ori.x, m_Ori.y, m_Ori.z, m_Ori.w));
		m_pori->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_QUATERNION);
	}

	m_pscl = propset->CreateReferenceProperty(_T("Scale"), 'SCL', &m_Scl, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (m_pscl)
	{
		m_pscl->SetVec3F(props::TVec3F(1, 1, 1));
	}

	m_Flags.SetAll(POSFLAG_REBUILDMATRIX);

	return true;
}


void PositionableImpl::Update(Object *pobject, float elapsed_time)
{
	if (m_Flags.AnySet(POSFLAG_REBUILDMATRIX))
	{
		glm::fmat4x4 tmp;

		// Next rotate...
		tmp = (glm::fmat4x4)m_Ori;

		// Recalculate our facing vector in between...
		m_Facing = glm::normalize(glm::vec4(0, 1, 0, 0) * tmp);

		// Recalculate our local up vector after that...
		m_LocalUp = glm::normalize(glm::vec4(0, 0, 1, 0) * tmp);

		// Recalculate the local left vector
		m_LocalLeft = glm::normalize(glm::cross(m_LocalUp, m_Facing));

		// Scale first, then rotate...
		m_Mat = glm::scale(glm::identity<glm::fmat4x4>(), m_Scl) * tmp;

		// Next translate for the rotational center...
		//m_Mat = m_Mat * glm::translate(glm::identity<glm::fmat4x4>(), m_RotCenter);

		// Then translate last... 
		m_Mat = m_Mat * glm::translate(glm::identity<glm::fmat4x4>(), m_Pos);

		//m_Bounds.Align(&m_Mat);

		m_Flags.Clear(POSFLAG_REBUILDMATRIX);
		m_Flags.Set(POSFLAG_MATRIXCHANGED);
	}
	else
		m_Flags.Clear(POSFLAG_MATRIXCHANGED);
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
		pbm->Draw(c3::Renderer::PrimType::LINELIST);
	}
}


void PositionableImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();
	if ((fcc == 'POS') || (fcc == 'ORI') || (fcc == 'SCL'))
	{
		m_Flags.Set(POSFLAG_REBUILDMATRIX);
	}
}


void PositionableImpl::SetPos(float x, float y, float z)
{
	if ((x != m_Pos.x) || (y != m_Pos.y) || (z != m_Pos.z))
	{
		m_Pos.x = x;
		m_Pos.y = y;
		m_Pos.z = z;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


void PositionableImpl::SetPosVec(const glm::fvec3 *pos)
{
	if (pos && (*pos != m_Pos))
	{
		m_Pos = *pos;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


void PositionableImpl::SetPosX(float x)
{
	if (x != m_Pos.x)
	{
		m_Pos.x = x;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


void PositionableImpl::SetPosY(float y)
{
	if (y != m_Pos.y)
	{
		m_Pos.y = y;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


void PositionableImpl::SetPosZ(float z)
{
	if (z != m_Pos.z)
	{
		m_Pos.z = z;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


const glm::fvec3 *PositionableImpl::GetPosVec(glm::fvec3 *pos)
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
	if ((xadj != 0) || (yadj != 0) || (zadj != 0))
	{
		m_Pos.x += xadj;
		m_Pos.y += yadj;
		m_Pos.z += zadj;

		m_Flags.Set(POSFLAG_POSCHANGED);
	}
}


void PositionableImpl::SetRotCenter(float x, float y, float z)
{

}


void PositionableImpl::SetRotCenterVec(const glm::fvec3 *rotc)
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


const glm::fvec3 *PositionableImpl::GetRotCenterVec(glm::fvec3 *rotc)
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
	if ((m_Ori.x != x) || (m_Ori.y != y) || (m_Ori.z != z) || (m_Ori.w != w))
	{
		m_Ori.x = x;
		m_Ori.y = y;
		m_Ori.z = z;
		m_Ori.w = w;

		m_Flags.Set(POSFLAG_ORICHANGED);
	}
}


void PositionableImpl::SetOriQuat(const glm::fquat *ori)
{
	if (ori && (*ori != m_Ori))
	{
		m_Ori = *ori;

		m_Flags.Set(POSFLAG_ORICHANGED);
	}
}


void PositionableImpl::SetYawPitchRoll(float y, float p, float r)
{
	glm::fquat qy = glm::angleAxis(y, glm::fvec3(0, 0, 1));
	glm::fquat qp = glm::angleAxis(p, glm::fvec3(1, 0, 0));
	glm::fquat qr = glm::angleAxis(r, glm::fvec3(0, 1, 0));

	m_Ori = (qr * qp) * qy;

	m_Flags.Set(POSFLAG_ORICHANGED);
}


const glm::fquat *PositionableImpl::GetOriQuat(glm::fquat *ori)
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
	// Extract the tangent of the yaw angle (rotation about the z-axis) from the orientation quaternion
	float tanyaw = (2 * ((m_Ori.x * m_Ori.y) + (m_Ori.w * m_Ori.z))) / ((m_Ori.w * m_Ori.w) + (m_Ori.x * m_Ori.x) - (m_Ori.y * m_Ori.y) - (m_Ori.z * m_Ori.z));

	return (float)atan(tanyaw);
}


float PositionableImpl::GetPitch()
{
	// Extract the sine of the pitch angle (rotation about the x-axis) from the orientation quaternion
	float sinpitch = -2 * ((m_Ori.x * m_Ori.z) - (m_Ori.w * m_Ori.y));

	return (float)asin(sinpitch);
}


float PositionableImpl::GetRoll()
{
	// Extract the tangent of the roll angle (rotation about the y-axis) from the orientation quaternion
	float tanroll = (2 * ((m_Ori.w * m_Ori.x) + (m_Ori.y * m_Ori.z))) / ((m_Ori.w * m_Ori.w) - (m_Ori.x * m_Ori.x) - (m_Ori.y * m_Ori.y) + (m_Ori.z * m_Ori.z));

	return (float)atan(tanroll);
}


void PositionableImpl::AdjustYaw(float dy)
{
	if (dy == 0)
		return;

	glm::fquat qy = glm::angleAxis(dy, glm::fvec3(0, 0, 1));

	m_Ori = m_Ori * qy;

	m_Flags.Set(POSFLAG_ORICHANGED);
}


void PositionableImpl::AdjustPitch(float dp)
{
	if (dp == 0)
		return;

	glm::fquat qp = glm::angleAxis(dp, glm::fvec3(1, 0, 0));

	m_Ori = m_Ori * qp;

	m_Flags.Set(POSFLAG_ORICHANGED);
}


void PositionableImpl::AdjustRoll(float dr)
{
	if (dr == 0)
		return;

	glm::fquat qr = glm::angleAxis(dr, glm::fvec3(0, 1, 0));

	m_Ori = m_Ori * qr;

	m_Flags.Set(POSFLAG_ORICHANGED);
}


const glm::fvec3 *PositionableImpl::GetFacingVector(glm::fvec3 *vec)
{
	if (vec)
	{
		*vec = m_Facing;
		return vec;
	}

	return &m_Facing;
}


const glm::fvec3 *PositionableImpl::GetLocalUpVector(glm::fvec3 *vec)
{
	if (vec)
	{
		*vec = m_LocalUp;
		return vec;
	}

	return &m_LocalUp;
}


const glm::fvec3 *PositionableImpl::GetLocalLeftVector(glm::fvec3 *vec)
{
	if (vec)
	{
		*vec = m_LocalLeft;
		return vec;
	}

	return &m_LocalLeft;
}


void PositionableImpl::SetScl(float x, float y, float z)
{
	if ((m_Scl.x != x) || (m_Scl.y != y) || (m_Scl.z != z))
	{
		m_Scl.x = x;
		m_Scl.y = y;
		m_Scl.z = z;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


void PositionableImpl::SetSclVec(const glm::fvec3 *scl)
{
	if (scl && (m_Scl != *scl))
	{
		m_Scl = *scl;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


void PositionableImpl::SetSclX(float x)
{
	if (m_Scl.x != x)
	{
		m_Scl.x = x;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


void PositionableImpl::SetSclY(float y)
{
	if (m_Scl.y != y)
	{
		m_Scl.y = y;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


void PositionableImpl::SetSclZ(float z)
{
	if (m_Scl.z != z)
	{
		m_Scl.z = z;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


const glm::fvec3 *PositionableImpl::GetScl(glm::fvec3 *scl)
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
	if ((dx != 0) || (dy != 0) || (dz != 0))
	{
		m_Scl.x += dx;
		m_Scl.y += dy;
		m_Scl.z += dz;

		m_Flags.Set(POSFLAG_SCLCHANGED);
	}
}


const glm::fmat4x4 *PositionableImpl::GetTransformMatrix(glm::fmat4x4 *mat)
{
	if (mat)
	{
		*mat = m_Mat;
		return mat;
	}

	return &m_Mat;
}
