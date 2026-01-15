// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


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


props::TFlags64 &InteractableImpl::Flags()
{
	return m_Flags;
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


// we want to pass messages like this up to the highest level parent that has a function
// by the given name... so, for example, if the button itself has an enter/exit handler, call it
// so you can override behaviors... but, if say you wanted to handle click messages at the root
// menu object so all messages go through one place, send it there.
void ExecuteDeepest(Object *pobj, const TCHAR *funcname, const TCHAR *args, ...)
{
	Scriptable *ps = nullptr;
	do
	{
		ps = (Scriptable *)(pobj->FindComponent(Scriptable::Type()));
		if (!ps || !ps->FunctionExists(funcname))
		{
			ps = nullptr;
			pobj = pobj->GetParent();
		}
	}
	while (pobj && !ps);
	if (!ps)
		return;

	static tstring execbuf;

	va_list marker;
	va_start(marker, args);

	int sz = _vsctprintf(args, marker);
	if (sz > (int)execbuf.capacity())
	{
		execbuf.reserve((sz * 2) + 1);
	}
	execbuf.resize(sz + 1);

	_vsntprintf_s((TCHAR *)(execbuf.data()), execbuf.capacity(), execbuf.capacity(), args, marker);

	ps->Execute(execbuf.c_str());
}

void InteractableImpl::Update(float elapsed_time)
{
	// Don't allow any input actions if one of our parents doesn't accept input or isn't being drawn
	Object *pobj = m_pOwner;
	while (pobj)
	{
		if (!pobj->Flags().IsSet(OF_ACCEPTINPUT | OF_DRAW))
			return;

		pobj = pobj->GetParent();
	}

	System *psys = m_pOwner->GetSystem();
	InputManager *pim = psys->GetInputManager();

	props::IProperty *pup = m_pOwner->GetProperties()->GetPropertyById('USER');
	InputManager::UserID uid = pup ? pup->AsInt() : InputManager::USER_DEFAULT;

	int32_t mx, my;
	pim->GetMousePos(mx, my);

	glm::fvec3 pickpos, pickdir;
	pim->GetPickRay(pickpos, pickdir);

	// see if our pick ray (ostensibly the mouse) intersects with whatever we've got
	bool picked = m_pOwner->Intersect(&pickpos, &pickdir, nullptr, nullptr, nullptr, nullptr, OF_DRAW, -1);
	if (picked)
	{
		// if picked and newly-so, call the mouse_enter script function
		if (!m_Flags.IsSet(IF_PICKED))
		{
			ExecuteDeepest(m_pOwner, _T("mouse_enter"), _T("mouse_enter();"));
		}

		m_Flags.Set(IF_PICKED);
	}
	else
	{
		// if picked and newly-so, call the mouse_enter script function
		if (m_Flags.IsSet(IF_PICKED))
		{
			ExecuteDeepest(m_pOwner, _T("mouse_exit"), _T("mouse_exit();"));
		}

		m_Flags.Clear(IF_PICKED | IF_ACTIVATED);
	}

	if (m_Flags.IsSet(IF_PICKED))
	{
		const TCHAR *name = m_pOwner->GetName();

		auto EvalClick = [&](InputDevice::VirtualButton button, const TCHAR *buttonname)
		{
			bool genclick = false;

			if (pim->ButtonPressed(button))
			{
				m_Flags.Set(IF_ACTIVATED);
			}
			else if (pim->ButtonReleased(button))
			{
				if (m_Flags.IsSet(IF_ACTIVATED))
					genclick = true;

				m_Flags.Clear(IF_ACTIVATED);
			}

			if (genclick)
			{
				ExecuteDeepest(m_pOwner, _T("mouse_click"), _T("mouse_click(\"%s\", \"%s\", %d, %d);"), name, buttonname, mx, my);
			}
		};

		EvalClick(InputDevice::VirtualButton::BUTTON1, _T("left"));
		EvalClick(InputDevice::VirtualButton::BUTTON2, _T("right"));
		EvalClick(InputDevice::VirtualButton::BUTTON3, _T("middle"));
		EvalClick(InputDevice::VirtualButton::AXIS2_NEGZ, _T("wheelup"));
		EvalClick(InputDevice::VirtualButton::AXIS2_POSZ, _T("wheeldown"));
	}
}


bool InteractableImpl::Prerender(RenderFlags flags, int draworder)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (flags.IsSet(RF_SHADOW))
		return false;

	if (flags.IsSet(RF_LIGHT))
		return false;

	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void InteractableImpl::Render(RenderFlags flags, const glm::fmat4x4 *pmat)
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


bool InteractableImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance, glm::fvec3 *pNormal, bool force) const
{
	bool ret = false;

	// TODO: check collisions however you want here

	return ret;
}
