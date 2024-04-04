// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3CameraImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(Camera, CameraImpl);


CameraImpl::CameraImpl()
{
	m_pcpos = nullptr;

	m_proj = glm::identity<glm::fmat4x4>();
	m_view = glm::identity<glm::fmat4x4>();

	m_viewmode = ViewMode::VM_POLAR;
	m_projmode = ProjectionMode::PM_PERSPECTIVE;

	m_dim = glm::vec2(2048.0f, 2048.0f);
	m_fov = 68.0f;
	m_eyepos = glm::vec3(0, 0, 0);
	m_targpos = glm::vec3(0, 0, 0);
	m_nearclip = 0.1f;
	m_farclip = 1400.0f;

	m_orbitdist = 10.0f;

	m_Flags.SetAll(CAMFLAG_REBUILDMATRICES);
}


CameraImpl::~CameraImpl()
{

}


void CameraImpl::Release()
{
	props::IPropertySet *pps = m_pOwner->GetProperties();
	props::IProperty *pp;

	pp = pps->GetPropertyById('C:DM');
	if (pp)	pp->ExternalizeReference();

	pp = pps->GetPropertyById('C:FV');
	if (pp)	pp->ExternalizeReference();

	pp = pps->GetPropertyById('C:OD');
	if (pp)	pp->ExternalizeReference();

	pp = pps->GetPropertyById('C:NC');
	if (pp)	pp->ExternalizeReference();

	pp = pps->GetPropertyById('C:FC');
	if (pp)	pp->ExternalizeReference();

	delete this;
}


props::TFlags64 CameraImpl::Flags() const
{
	return m_Flags;
}


bool CameraImpl::Initialize(Object *pobject)
{
	if (!pobject)
		return false;

	m_pOwner = pobject;

	props::IPropertySet *props = m_pOwner->GetProperties();
	if (!props)
		return false;

	props::IProperty *pp;

	pp = props->CreateProperty(_T("ViewMode"), 'C:VM');
	if (pp)
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_viewmode);
	}

	pp = props->CreateProperty(_T("ProjectionMode"), 'C:PM');
	if (pp)
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_projmode);
	}

	pp = props->CreateReferenceProperty(_T("Dimensions(Orthographic)"), 'C:DM', &m_dim, props::IProperty::PT_FLOAT_V2);

	pp = props->CreateReferenceProperty(_T("FOV(Perspective)"), 'C:FV', &m_fov, props::IProperty::PT_FLOAT);
	if (pp)
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG);

	pp = props->CreateReferenceProperty(_T("OrbitDistance(Polar)"), 'C:OD', &m_orbitdist, props::IProperty::PT_FLOAT);

	pp = props->CreateReferenceProperty(_T("NearClip"), 'C:NC', &m_nearclip, props::IProperty::PT_FLOAT);

	pp = props->CreateReferenceProperty(_T("FarClip"), 'C:FC', &m_farclip, props::IProperty::PT_FLOAT);

	return true;
}


void CameraImpl::Update(float elapsed_time)
{
	// get a positionable feature from the object -- and if we can't, don't proceed
	if (!m_pcpos)
		m_pcpos = dynamic_cast<PositionableImpl *>(m_pOwner->FindComponent(Positionable::Type()));

	if (!m_pcpos)
		return;

	if (m_Flags.IsSet(CAMFLAG_REBUILDMATRICES) || m_pcpos->Flags().IsSet(POSFLAG_MATRIXCHANGED))
	{
		glm::vec3 facing;
		m_pcpos->GetFacingVector(&facing);

		switch (m_viewmode)
		{
			// In LOOKAT m_Mode, the eye position is actually located at camera position,
			// and the lookat position is the position plus the orientation vector
			case VM_LOOKAT:
				m_eyepos = *m_pcpos->GetTransformMatrix() * glm::fvec4(0, 0, 0, 1);
				m_targpos = m_eyepos + facing;
				break;

			// In POLAR m_Mode, the lookat position is the camera position...
			// the eye location is derived by multiplying the orientation by the distance
			default:
			case VM_POLAR:
				m_targpos = *m_pcpos->GetTransformMatrix() * glm::fvec4(0, 0, 0, 1);
				m_eyepos = m_targpos - (facing * m_orbitdist);
				break;
		}

		glm::fvec3 up, right;
		m_pcpos->GetLocalUpVector(&up);
		m_pcpos->GetLocalRightVector(&right);

		// glm's lookAt is busted. Using it results in incorrect placement of the eyepoint (opposite facing)
		m_view = glm::lookAt(m_eyepos, m_targpos, up);

		switch (m_projmode)
		{
			default:
			case PM_PERSPECTIVE:
				m_proj = glm::perspectiveFov(glm::radians(m_fov), m_dim.x, m_dim.y, m_nearclip, m_farclip);
				break;

			case PM_ORTHOGRAPHIC:
			{
				glm::fvec2 halfdim = (m_dim + m_orbitdist) / 2.0f;
				m_proj = glm::ortho(m_eyepos.x - halfdim.x, m_eyepos.x + halfdim.x, m_eyepos.y - halfdim.y, m_eyepos.y + halfdim.y, m_nearclip, m_farclip);
				break;
			}
		}

		m_Flags.Clear(CAMFLAG_REBUILDMATRICES);
	}
}


bool CameraImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	return false;
}


void CameraImpl::Render(Object::RenderFlags flags)
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


void CameraImpl::SetOrthoDimensions(float dimx, float dimy)
{
	if (m_dim != glm::fvec2(dimx, dimy))
	{
		m_dim.x = dimx;
		m_dim.y = dimy;

		if (m_projmode == Camera::ProjectionMode::PM_ORTHOGRAPHIC)
			m_Flags.Set(CAMFLAG_REBUILDMATRICES);
	}
}


void CameraImpl::SetOrthoDimensionsVec(const glm::fvec2 *dim)
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


bool CameraImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const
{
	return false;
}


void CameraImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();

	switch (fcc)
	{
		case 'C:VM':
			m_viewmode = (ViewMode)pprop->AsInt();
			if (pprop->GetType() != props::IProperty::PT_ENUM)
			{
				((props::IProperty *)pprop)->SetEnumProvider(this);
				((props::IProperty *)pprop)->SetEnumVal(m_viewmode);
			}
			break;

		case 'C:PM':
			m_projmode = (ProjectionMode)pprop->AsInt();
			if (pprop->GetType() != props::IProperty::PT_ENUM)
			{
				((props::IProperty *)pprop)->SetEnumProvider(this);
				((props::IProperty *)pprop)->SetEnumVal(m_projmode);
			}
			break;

		default:
			break;
	}

	m_Flags.Set(CAMFLAG_REBUILDMATRICES);
}


std::vector<tstring> ViewModeNames ={_T("Polar"), _T("Look At")};
std::vector<tstring> ProjModeNames ={_T("Perspective"), _T("Orthographic")};

size_t CameraImpl::GetNumValues(const props::IProperty *pprop) const
{
	if (pprop)
	{
		switch (pprop->GetID())
		{
			case 'C:VM':
				return ViewModeNames.size();
				break;

			case 'C:PM':
				return ProjModeNames.size();
				break;
		}
	}

	return 0;
}


const TCHAR *CameraImpl::GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const
{
	if (pprop)
	{
		switch (pprop->GetID())
		{
			case 'C:VM':
				if (buf && (bufsize >= ((ViewModeNames[ordinal].length() + 1) * sizeof(TCHAR))))
					_tcscpy_s(buf, bufsize, ViewModeNames[ordinal].c_str());
				return ViewModeNames[ordinal].c_str();
				break;

			case 'C:PM':
				if (buf && (bufsize >= ((ProjModeNames[ordinal].length() + 1) * sizeof(TCHAR))))
					_tcscpy_s(buf, bufsize, ProjModeNames[ordinal].c_str());
				return ProjModeNames[ordinal].c_str();
				break;
		}
	}

	return nullptr;
}
