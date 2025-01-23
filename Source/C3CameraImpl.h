// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Camera.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class CameraImpl : public Camera, props::IPropertyChangeListener, props::IProperty::IEnumProvider
	{

	protected:
		Object *m_pOwner;

		glm::fmat4x4 m_proj, m_view;

		ViewMode m_viewmode;

		ProjectionMode m_projmode;

		glm::fvec2 m_dim;

		float m_fov;

		float m_nearclip;

		float m_farclip;

		glm::fvec3 m_eyepos, m_targpos;

		float m_orbitdist;

		Positionable *m_pcpos;

		props::TFlags64 m_Flags;

	public:

		CameraImpl();

		virtual ~CameraImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat);

		virtual void SetViewMode(ViewMode mode);
		virtual ViewMode GetViewMode();

		virtual void SetProjectionMode(ProjectionMode mode);
		virtual ProjectionMode GetProjectionMode();

		virtual void SetPolarDistance(float distance);
		virtual float GetPolarDistance();

		virtual void SetOrthoDimensions(float dimx, float dimy);
		virtual void SetOrthoDimensionsVec(const glm::fvec2 *dim);
		virtual const glm::fvec2 *GetOrthoDimensions(glm::fvec2 *dim);
		virtual float GetOrthoWidth();
		virtual float GetOrthoHeight();

		virtual void SetFOV(float height);
		virtual float GetFOV();

		virtual const glm::fvec3 *GetEyePos(glm::fvec3 *pos);
		virtual const glm::fvec3 *GetTargetPos(glm::fvec3 *pos);

		virtual const glm::fmat4x4 *GetViewMatrix(glm::fmat4x4 *mat);
		virtual const glm::fmat4x4 *GetProjectionMatrix(glm::fmat4x4 *mat);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance) const;

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf = nullptr, size_t bufsize = 0) const;

	};

	DEFINE_COMPONENTTYPE(Camera, CameraImpl, GUID({0xfcc880c3, 0x54a2, 0x4168, { 0xbd, 0x4, 0xe9, 0x91, 0xfe, 0xee, 0x29, 0xef }}), "Camera", "Camera generates view and projection matrices for scene rendering (requires Positionable)", 0);

};
