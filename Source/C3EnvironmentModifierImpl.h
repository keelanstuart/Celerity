// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3EnvironmentModifier.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class EnvironmentModifierImpl : public EnvironmentModifier, props::IPropertyChangeListener
	{

	protected:

		Object *m_pOwner;

		Positionable *m_pPos;

		props::TFlags64 m_Flags;

		bool m_bCameraInside;

	public:

		EnvironmentModifierImpl();

		virtual ~EnvironmentModifierImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags);

		virtual void Render(Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

	};

	// {C8ED7967-2E22-46D2-BA1B-9FD0D1E7CAE2}
	DEFINE_COMPONENTTYPE(EnvironmentModifier, EnvironmentModifierImpl, GUID({ 0xc8ed7967, 0x2e22, 0x46d2, { 0xba, 0x1b, 0x9f, 0xd0, 0xd1, 0xe7, 0xca, 0xe2 } }), "EnvironmentModifier", "EnvironmentModifier affects some, or all, of the environment settings, such as lighting/shadowing, fogging, audio, etc. (requires Positionable) Recursive; last one in hierarchy wins that the eyepoint is inside.", 0);

};