// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Camera.h>

namespace c3
{

	class CameraImpl : public Camera, props::IPropertyChangeListener
	{

	protected:
		C3MATRIX m_proj, m_view;

		props::IProperty *m_pviewmode;
		ViewMode m_viewmode;

		props::IProperty *m_pprojpmode;
		ProjectionMode m_projmode;

		props::IProperty *m_pdim;
		C3VEC2 m_dim;

		props::IProperty *m_pfov;
		float m_fov;

		C3VEC3 m_campos, m_targpos;

	public:

		CameraImpl();

		virtual ~CameraImpl();

		virtual void Release();

		virtual ComportmentType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IPropertySet *ppropset, const props::IProperty *pprop);

		virtual void SetViewMode(ViewMode mode);
		virtual ViewMode GetViewMode();

		virtual void SetProjectionMode(ProjectionMode mode);
		virtual ProjectionMode GetProjectionMode();

		virtual void SetPolarDistance(float distance);
		virtual float GetPolarDistance();

		virtual void SetOrthoDimensions(const C3VEC2 *dim);
		virtual const C3VEC2 *GetOrthoDimensions(C3VEC2 *dim);
		virtual float GetOrthoWidth();
		virtual float GetOrthoHeight();

		virtual void SetFOV(float height);
		virtual float GetFOV();

		virtual const C3VEC3 *GetEyePos(C3VEC3 *pos);
		virtual const C3VEC3 *GetTargetPos(C3VEC3 *pos);

		virtual const C3MATRIX *GetViewMatrix(C3MATRIX *mat);
		virtual const C3MATRIX *GetProjectionMatrix(C3MATRIX *mat);

	};

};
