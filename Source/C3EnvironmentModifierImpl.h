// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3EnvironmentModifier.h>
#include <C3PositionableImpl.h>

namespace c3
{

	class EnvironmentModifierImpl : public EnvironmentModifier, props::IPropertyChangeListener
	{

	protected:
		Positionable *m_pPos;

		props::TFlags64 m_Flags;

	public:

		EnvironmentModifierImpl();

		virtual ~EnvironmentModifierImpl();

		virtual void Release();

		virtual ComponentType *GetType();

		virtual props::TFlags64 Flags();

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, props::TFlags64 rendflags);

		virtual void Render(Object *pobject, props::TFlags64 rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const;

	};

	// {C8ED7967-2E22-46D2-BA1B-9FD0D1E7CAE2}
	DEFINE_COMPONENTTYPE(EnvironmentModifier, EnvironmentModifierImpl, GUID({ 0xc8ed7967, 0x2e22, 0x46d2, { 0xba, 0x1b, 0x9f, 0xd0, 0xd1, 0xe7, 0xca, 0xe2 } }), "EnvironmentModifier", "EnvironmentModifier affects some, or all, of the environment settings, such as lighting/shadowing, fogging, audio, etc. (requires Positionable) Recursive; last one in hierarchy wins that the eyepoint is inside.");

};