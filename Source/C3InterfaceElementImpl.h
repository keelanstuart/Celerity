// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InterfaceElement.h>

namespace c3
{

	class InterfaceElementImpl : public InterfaceElement, props::IPropertyChangeListener
	{

	protected:
		Object *m_pOwner;

		props::TFlags64 m_Flags;

	public:

		InterfaceElementImpl();

		virtual ~InterfaceElementImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags);

		virtual void Render(Object::RenderFlags rendflags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

	};

	DEFINE_COMPONENTTYPE(InterfaceElement, InterfaceElementImpl, GUID({0x2a1a3303, 0xb99f, 0x4f3b, {0xb6, 0x88, 0x5d, 0x13, 0xf4, 0xae, 0x77, 0xd0}}), "InterfaceElement", "InterfaceElement contains combined text and image rendering and event response scripting (requires Scriptable)", 0);

};