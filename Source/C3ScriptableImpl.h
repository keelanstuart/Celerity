// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Scriptable.h>
#include <TinyJS.h>

namespace c3
{

	class ScriptableImpl : public Scriptable, props::IPropertyChangeListener
	{

	protected:
		CTinyJS m_JS;
		tstring m_Code;
		float m_UpdateRate;
		float m_UpdateTime;
		System *m_pSys;

	public:

		ScriptableImpl();

		virtual ~ScriptableImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(Object *pobject, float elapsed_time = 0.0f);

		virtual bool Prerender(Object *pobject, Object::RenderFlags flags);

		virtual void Render(Object *pobject, Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance) const;

	};

	DEFINE_COMPONENTTYPE(Scriptable, ScriptableImpl, GUID({0x1f1d4b43, 0x8b2a, 0x44de, {0xba, 0xb5, 0xd2, 0x4d, 0x8a, 0xc3, 0x2b, 0xec}}), "Scriptable", "Allows the Object to be controlled by scripts");

};