// TODO: add copyright

#pragma once

#include <C3.h>
#include <C3Interactable.h>

namespace c3
{

	class InteractableImpl : public Interactable, props::IPropertyChangeListener
	{
	protected:

		Object* m_pOwner;

#define IF_PICKED		0x0001
#define IF_TABSTOP		0x0002
#define IF_ACTIVATED	0x0004

		props::TFlags64 m_Flags;

	public:

		InteractableImpl();

		virtual ~InteractableImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder = 0);

		virtual void Render(Object::RenderFlags rendflags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance, glm::fvec3 *pNormal, bool force) const;

	};

	DEFINE_COMPONENTTYPE(Interactable, InteractableImpl, GUID({0x8681db8c,0xd675,0x4497,{0xbf,0x57,0x69,0x13,0xd9,0xf5,0x19,0x3b}}), "Interactable", "TODO: describe your component here", 0);

};
