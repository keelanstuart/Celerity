// TODO: add copyright

#include "pch.h"

#include "C3InteractableImpl.h"

using namespace c3;


DECLARE_COMPONENTTYPE(Interactable, InteractableImpl);


InteractableImpl::InteractableImpl()
{
}


InteractableImpl::~InteractableImpl()
{
}


void InteractableImpl::Release()
{
	delete this;
}


props::TFlags64 InteractableImpl::Flags() const
{
	return 0;
}


bool InteractableImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet* propset = pobject->GetProperties();
	if (!propset)
		return false;

	// TODO: create your required properties here...

	return true;
}


void InteractableImpl::Update(float elapsed_time)
{
	Scriptable *ps = (Scriptable *)m_pOwner->FindComponent(Scriptable::Type());
	Positionable *pp = (Positionable *)m_pOwner->FindComponent(Positionable::Type());

	if (ps && pp)
	{
		System *psys = m_pOwner->GetSystem();
		InputManager *pim = psys->GetInputManager();

		props::IProperty *pup = m_pOwner->GetProperties()->GetPropertyById('USER');
		InputManager::UserID uid = pup ? pup->AsInt() : InputManager::USER_DEFAULT;

		int32_t mx, my;
		pim->GetMousePos(mx, my);

		glm::fvec3 pickpos, pickdir;
		pim->GetPickRay(pickpos, pickdir);

		// see if our pick ray (ostensibly the mouse) intersects with whatever we've got
		bool picked = m_pOwner->Intersect(&pickpos, &pickdir, nullptr, nullptr, nullptr, OF_DRAW, -1);
		if (picked)
		{
			// if picked and newly-so, call the mouse_enter script function
			if (!m_Flags.IsSet(IF_PICKED) && ps->FunctionExists(_T("mouse_enter")))
			{
				ps->Execute(_T("mouse_enter();"));
			}

			m_Flags.Set(IF_PICKED);
		}
		else
		{
			// if picked and newly-so, call the mouse_enter script function
			if (m_Flags.IsSet(IF_PICKED) && ps->FunctionExists(_T("mouse_exit")))
			{
				ps->Execute(_T("mouse_exit();"));
			}

			m_Flags.Clear(IF_PICKED | IF_ACTIVATED);
		}

		if (m_Flags.IsSet(IF_PICKED))
		{
			bool genclick = false;

			if (pim->ButtonPressed(InputDevice::VirtualButton::BUTTON1))
			{
				if (pim->ButtonChange(InputDevice::VirtualButton::BUTTON1))
					genclick = true;

				m_Flags.Set(IF_ACTIVATED);
			}
			else if (pim->ButtonReleased(InputDevice::VirtualButton::BUTTON1))
			{
				if (m_Flags.IsSet(IF_ACTIVATED))
					genclick = true;

				m_Flags.Clear(IF_ACTIVATED);
			}

			if (genclick)
				ps->Execute(_T("mouse_click(\"%s\", %d, %d);"), "left", mx, my);
		}
	}
}


bool InteractableImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void InteractableImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	if (flags.IsSet(RF_SHADOW))
		return; // TODO; just suggestions

	if (flags.IsSet(RF_LIGHT))
		return; // TODO; just suggestions

	if (flags.IsSet(RF_AUXILIARY))
		return; // TODO; just suggestions

	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();

	//TODO: use pr to draw thing(s)

}


void InteractableImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();

#if 0
	switch (fcc) // TODO: handle property changes by ID
	{
		default:
			break;
	}
#endif
}


bool InteractableImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance, bool force) const
{
	bool ret = false;

	// TODO: check collisions however you want here

	return ret;
}
