// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3CameraImpl.h>

using namespace c3;


DECLARE_FEATURETYPE(Camera, CameraImpl);


CameraImpl::CameraImpl()
{
	m_pcpos = nullptr;

	m_proj = glm::identity<glm::fmat4x4>();
	m_view = glm::identity<glm::fmat4x4>();

	m_viewmode = ViewMode::VM_POLAR;
	m_projmode = ProjectionMode::PM_PERSPECTIVE;

	m_dim = glm::vec2(2048.0f, 2048.0f);
	m_fov = 60.0f;
	m_eyepos = glm::vec3(0, -10.0f, 0);
	m_targpos = glm::vec3(0, 0, 0);

	m_orbitdist = 10.0f;

	m_pviewmode = nullptr;
	m_pprojpmode = nullptr;
	m_pdim = nullptr;
	m_pfov = nullptr;
	m_porbitdist = nullptr;

	m_Flags.SetAll(CAMFLAG_REBUILDMATRICES);
}


CameraImpl::~CameraImpl()
{

}


void CameraImpl::Release()
{
	delete this;
}


props::TFlags64 CameraImpl::Flags()
{
	return m_Flags;
}


bool CameraImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	m_pviewmode = props->CreateReferenceProperty(_T("ViewMode"), 'C:VM', &m_viewmode, props::IProperty::PT_INT);
	m_pprojpmode = props->CreateReferenceProperty(_T("ProjectionMode"), 'C:PM', &m_projmode, props::IProperty::PT_INT);
	m_pdim = props->CreateReferenceProperty(_T("Dimensions(Orthographic)"), 'C:DM', &m_dim, props::IProperty::PT_FLOAT_V2);
	m_pfov = props->CreateReferenceProperty(_T("FOV(Perspective)"), 'C:FV', &m_viewmode, props::IProperty::PT_FLOAT);
	m_porbitdist = props->CreateReferenceProperty(_T("OrbitDistance(Polar)"), 'C:OD', &m_viewmode, props::IProperty::PT_FLOAT);

	return true;
}


void CameraImpl::Update(Object *pobject, float elapsed_time)
{
	// get a positionable feature from the object -- and if we can't, don't proceed
	if (!m_pcpos)
		m_pcpos = dynamic_cast<PositionableImpl *>(pobject->FindFeature(Positionable::Type()));

	if (!m_pcpos)
		return;

	if (m_Flags.IsSet(CAMFLAG_REBUILDMATRICES) || m_pcpos->Flags().IsSet(POSFLAG_MATRIXCHANGED))
	{
		m_pcpos->GetPosVec(&m_eyepos);
		m_targpos = m_eyepos;

		glm::vec3 adj;
		m_pcpos->GetFacingVector(&adj);
		adj *= m_orbitdist;

		switch (m_viewmode)
		{
			// In LOOKAT m_Mode, the eye position is actually located at camera position,
			// and the lookat position is the position plus the orientation vector
			case VM_LOOKAT:
				m_targpos += adj;
				break;

			// In POLAR m_Mode, the lookat position is the camera position...
			// the eye location is derived by multiplying the orientation by the distance
			default:
			case VM_POLAR:
				m_eyepos -= adj;
				break;
		}

		glm::fvec3 up;
		m_pcpos->GetLocalUpVector(&up);
		m_view = glm::lookAtLH(m_eyepos, m_targpos, up);

		switch (m_projmode)
		{
			default:
			case PM_PERSPECTIVE:
				m_proj = glm::perspectiveFovLH(m_fov, m_dim.x, m_dim.y, 0.01f, 500.0f);
				break;

			case PM_ORTHOGRAPHIC:
				m_proj = glm::orthoLH(0.0f, 2048.0f, 0.0f, 2048.0f, 0.01f, 1.0f);
				break;
		}

		m_Flags.Clear(CAMFLAG_REBUILDMATRICES);
	}
}


bool CameraImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	return false;
}


void CameraImpl::Render(Object *pobject, props::TFlags64 rendflags)
{

}


void CameraImpl::PropertyChanged(const props::IProperty *pprop)
{

}


void CameraImpl::SetViewMode(Camera::ViewMode mode)
{
	if (m_viewmode != mode)
	{
		m_viewmode = mode;
		m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


Camera::ViewMode CameraImpl::GetViewMode()
{
	return (Camera::ViewMode)m_viewmode;
}


void CameraImpl::SetProjectionMode(Camera::ProjectionMode mode)
{
	if (m_projmode != mode)
	{
		m_projmode = mode;
		m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


Camera::ProjectionMode CameraImpl::GetProjectionMode()
{
	return (Camera::ProjectionMode)m_projmode;
}


void CameraImpl::SetPolarDistance(float distance)
{
	if (m_orbitdist != distance)
	{
		m_orbitdist = distance;

		if (m_viewmode == Camera::ViewMode::VM_POLAR)
			m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


float CameraImpl::GetPolarDistance()
{
	return m_orbitdist;
}


void CameraImpl::SetOrthoDimensions(const glm::fvec2 *dim)
{
	if (m_dim != *dim)
	{
		m_dim = *dim;

		if (m_projmode == Camera::ProjectionMode::PM_ORTHOGRAPHIC)
			m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


const glm::fvec2 *CameraImpl::GetOrthoDimensions(glm::fvec2 *dim)
{
	if (dim)
	{
		*dim = m_dim;
		return dim;
	}

	return &m_dim;
}


float CameraImpl::GetOrthoWidth()
{
	return m_dim.x;
}


float CameraImpl::GetOrthoHeight()
{
	return m_dim.y;
}


void CameraImpl::SetFOV(float fov)
{
	if (m_fov != fov)
	{
		m_fov = fov;

		if (m_projmode == Camera::ProjectionMode::PM_PERSPECTIVE)
			m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


float CameraImpl::GetFOV()
{
	return m_fov;
}


const glm::fvec3 *CameraImpl::GetEyePos(glm::fvec3 *pos)
{
	if (pos)
	{
		*pos = m_eyepos;
		return pos;
	}

	return &m_eyepos;
}


const glm::fvec3 *CameraImpl::GetTargetPos(glm::fvec3 *pos)
{
	if (pos)
	{
		*pos = m_targpos;
		return pos;
	}

	return &m_targpos;
}


const glm::fmat4x4 *CameraImpl::GetViewMatrix(glm::fmat4x4 *mat)
{
	if (mat)
	{
		*mat = m_view;
		return mat;
	}

	return &m_view;
}


const glm::fmat4x4 *CameraImpl::GetProjectionMatrix(glm::fmat4x4 *mat)
{
	if (mat)
	{
		*mat = m_proj;
		return mat;
	}

	return &m_proj;
}
