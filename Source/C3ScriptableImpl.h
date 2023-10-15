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
		Object *m_pOwner;

		CTinyJS *m_JS;
		tstring m_Code;
		float m_UpdateRate;
		float m_UpdateTime;

		bool m_Continue;

		void ResetJS();

	public:

		ScriptableImpl();

		virtual ~ScriptableImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags);

		virtual void Render(Object::RenderFlags flags);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const;

		virtual void Execute(const TCHAR *pcmd, ...);

	};

	// Scriptable Component GUID {1F1D4B43-8B2A-44DE-BAB5-D24D8AC32BEC}
	DEFINE_COMPONENTTYPE(Scriptable, ScriptableImpl, GUID({0x1f1d4b43, 0x8b2a, 0x44de, {0xba, 0xb5, 0xd2, 0x4d, 0x8a, 0xc3, 0x2b, 0xec}}), "Scriptable", "Allows the Object to be controlled by scripts", CF_PUSHFRONT);


	class Script
	{

	public:
		tstring m_Code;

	};

	// Script Resource GUID {E65D1068-54E4-4CB4-B080-4A527B154E93}
	DEFINE_RESOURCETYPE(Script, 0, GUID({0xe65d1068, 0x54e4, 0x4cb4, {0xb0, 0x80, 0x4a, 0x52, 0x7b, 0x15, 0x4e, 0x93}}), "Script", "Scripts", "c3js", "c3js");

};