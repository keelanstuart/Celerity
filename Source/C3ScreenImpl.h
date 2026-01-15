// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3Screen.h>
#include <C3ObjectImpl.h>
#include <C3GlobalObjectRegistryImpl.h>


namespace c3
{

	class ScreenImpl : public Screen, props::IPropertyChangeListener
	{

	protected:

		Object *m_pOwner;

		props::TFlags64 m_Flags;

		GlobalObjectRegistryImpl m_Reg;

		float m_RunTime;

	public:
		ScreenImpl();

		virtual ~ScreenImpl();

		virtual GlobalObjectRegistry *GetObjectRegistry();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 &Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(RenderFlags flags, int draworder = 0);

		virtual void Render(RenderFlags rendflags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance, glm::fvec3 *pNormal, bool force) const;

	};

	DEFINE_COMPONENTTYPE(Screen, ScreenImpl, GUID({ 0x76a0cb6b, 0x7702, 0x49d2, { 0x8f, 0xd4, 0xc8, 0xf4, 0x23, 0xff, 0xa5, 0xdd } }), "Screen", "Captures top-level things", 0);

};