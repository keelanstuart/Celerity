// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3PositionableImpl.h>
#include <C3Math.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Positionable, PositionableImpl);


PositionableImpl::PositionableImpl()
{
	m_Mat = glm::identity<glm::fmat4x4>();
	m_MatN = glm::identity<glm::fmat4x4>();
	m_Ori = glm::identity<glm::fquat>();
	m_Flags = OF_DRAWINEDITOR;
	m_Facing = glm::fvec3(0, 1, 0);
	m_LocalRight = glm::fvec3(1, 0, 0);
	m_LocalUp = glm::fvec3(0, 0, 1);
}


PositionableImpl::~PositionableImpl()
{
}


void PositionableImpl::Release()
{

}


props::TFlags64 PositionableImpl::Flags() const
{
	return m_Flags;
}


bool PositionableImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *propset = m_pOwner->GetProperties();
	if (!propset)
		return false;

	props::IProperty *ppos = propset->CreateReferenceProperty(_T("Position"), 'POS', &m_Pos, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (ppos)
	{
		ppos->SetVec3F(props::TVec3F(0, 0, 0));
	}

	props::IProperty *pori = propset->CreateReferenceProperty(_T("Orientation"), 'ORI', &m_Ori, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4);
	if (pori)
	{
		pori->SetVec4F(props::TVec4F(m_Ori.x, m_Ori.y, m_Ori.z, m_Ori.w));
		pori->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_QUATERNION);
		pori->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	props::IProperty *pscl = propset->CreateReferenceProperty(_T("Scale"), 'SCL', &m_Scl, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pscl)
	{
		pscl->SetVec3F(props::TVec3F(1, 1, 1));
	}

	props::IProperty *pfacing = propset->CreateReferenceProperty(_T("Facing.Forward"), 'FFWD', &m_Facing, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pfacing)
	{
		pfacing->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	props::IProperty *pright = propset->CreateReferenceProperty(_T("Facing.Right"), 'FRGT', &m_LocalRight, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pright)
	{
		pright->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	props::IProperty *pup = propset->CreateReferenceProperty(_T("Facing.Up"), 'FUP', &m_LocalUp, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pup)
	{
		pup->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::HIDDEN));
	}

	m_Flags.SetAll(POSFLAG_REBUILDMATRIX);

	return true;
}


void PositionableImpl::Update(float elapsed_time)
{
	// if this object should track the camera, then move it here
	if (m_pOwner && m_pOwner->Flags().AnySet(OF_TRACKCAMX | OF_TRACKCAMY | OF_TRACKCAMZ))
	{
		Object *pcamobj = m_pOwner->GetSystem()->GetGlobalObjectRegistry()->GetRegisteredObject(GlobalObjectRegistry::OD_CAMERA);
		if (pcamobj)
		{
			Camera *pcamcomp = dynamic_cast<Camera *>(pcamobj->FindComponent(Camera::Type()));
			if (pcamcomp)
			{
				glm::fvec3 pos;
				// literal tracking uses the eye point itself, whereas non-literal tracking uses the focus point
				if (m_pOwner->Flags().AnySet(OF_TRACKCAMLITERAL))
					pcamcomp->GetEyePos(&pos);
				else
					pcamcomp->GetTargetPos(&pos);

				if (m_pOwner->Flags().AnySet(OF_TRACKCAMX))
					m_Pos.x = pos.x;

				if (m_pOwner->Flags().AnySet(OF_TRACKCAMY))
					m_Pos.y = pos.y;

				if (m_pOwner->Flags().AnySet(OF_TRACKCAMZ))
					m_Pos.z = pos.z;

				m_Flags.Set(POSFLAG_POSCHANGED);
			}
		}
	}

	Object *pparent = m_pOwner->GetParent();
	Positionable *pparentpos = nullptr;
	while (pparent)
	{
		if ((pparentpos = dynamic_cast<Positionable *>(pparent->FindComponent(Positionable::Type()))) != nullptr)
			break;

		pparent = pparent->GetParent();
	}

	// if any parent matrix changed, then update ours as well
	if (pparentpos && pparentpos->Flags().AnySet(POSFLAG_MATRIXCHANGED))
		m_Flags.Set(POSFLAG_REBUILDMATRIX);

	if (m_Flags.AnySet(POSFLAG_REBUILDMATRIX))
	{
		// Scale, rotate, then translate
		m_Mat = glm::translate(glm::identity<glm::fmat4x4>(), m_Pos) * ((glm::fmat4x4)(glm::normalize(m_Ori)) * glm::scale(glm::identity<glm::fmat4x4>(), m_Scl));

		// Make a normal matrix
		m_MatN = glm::inverseTranspose(m_Mat);

		glm::fmat4x4 tmp = (glm::fmat4x4)m_Ori;

		if (pparentpos)
		{
			m_Mat = *(pparentpos->GetTransformMatrix()) * m_Mat;
			m_MatN = *(pparentpos->GetTransformMatrixNormal()) * m_MatN;
			tmp = *(pparentpos->GetTransformMatrixNormal()) * tmp;
		}

		// Recalculate our facing vector in between...
		m_Facing = glm::normalize(tmp * glm::vec4(0, 1, 0, 0));

		// Recalculate our local up vector after that...
		m_LocalUp = glm::normalize(tmp * glm::vec4(0, 0, 1, 0));

		// Recalculate the local right vector
		m_LocalRight = glm::normalize(tmp * glm::vec4(1, 0, 0, 0));

		//m_Bounds.Align(&m_Mat);

		m_Flags.Clear(POSFLAG_REBUILDMATRIX);
		m_Flags.Set(POSFLAG_MATRIXCHANGED);
	}
	else
		m_Flags.Clear(POSFLAG_MATRIXCHANGED);
}


bool PositionableImpl::Prerender(Object::RenderFlags flags)
{
	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();
	pr->SetWorldMatrix(&m_Mat);

	if (flags.IsSet(RF_FORCE))
		return true;

	if (!m_Flags.IsSet(OF_DRAW))
		return false;

	return true;
}


void PositionableImpl::Render(Object::RenderFlags flags)
{
#if 0
	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();

	ShaderProgram *sp = pr->GetBoundsShader();
	pr->UseProgram(sp);
	pr->GetWhiteMaterial()->Apply(sp);

	pr->GetBoundsMesh()->Draw(c3::Renderer::PrimType::LINELIST);
#endif
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


void PositionableImpl::SetOri(float x, float y, float z, float w)
{
	if ((m_Ori.x != x) || (m_Ori.y != y) || (m_Ori.z != z) || (m_Ori.w != w))
	{
		m_Ori = glm::normalize(glm::fquat(x, y, z, w));

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

	m_Ori = glm::normalize((qr * qp) * qy);

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
	return math::GetYaw(&m_Ori);
}


float PositionableImpl::GetPitch()
{
	return math::GetPitch(&m_Ori);
}


float PositionableImpl::GetRoll()
{
	return math::GetRoll(&m_Ori);
}


void PositionableImpl::AdjustYaw(float dy)
{
	if (dy == 0)
		return;

	glm::fquat qy = glm::angleAxis(dy, m_LocalUp);

	m_Ori = glm::normalize(qy * m_Ori);

	m_Flags.Set(POSFLAG_ORICHANGED);
}


void PositionableImpl::AdjustYawFlat(float dy)
{
	if (dy == 0)
		return;

	glm::fquat qy = glm::angleAxis(dy, glm::vec3(0, 0, 1));

	m_Ori = glm::normalize(qy * m_Ori);

	m_Flags.Set(POSFLAG_ORICHANGED);
}


void PositionableImpl::AdjustPitch(float dp)
{
	if (dp == 0)
		return;

	glm::fquat qp = glm::angleAxis(dp, m_LocalRight);

	m_Ori = glm::normalize(qp * m_Ori);

	m_Flags.Set(POSFLAG_ORICHANGED);
}


void PositionableImpl::AdjustRoll(float dr)
{
	if (dr == 0)
		return;

	glm::fquat qr = glm::angleAxis(dr, m_Facing);

	m_Ori = glm::normalize(qr * m_Ori);

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


const glm::fvec3 *PositionableImpl::GetLocalRightVector(glm::fvec3 *vec)
{
	if (vec)
	{
		*vec = m_LocalRight;
		return vec;
	}

	return &m_LocalRight;
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


const glm::fmat4x4 *PositionableImpl::GetTransformMatrixNormal(glm::fmat4x4 *matn)
{
	if (matn)
	{
		*matn = m_MatN;
		return matn;
	}

	return &m_MatN;
}


bool PositionableImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const
{
	return false;
}
