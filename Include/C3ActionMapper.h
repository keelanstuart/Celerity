// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDevice.h>


namespace c3
{

	class ActionMapper
	{

	public:

		// When an action has been triggered, a callback of this type, supplied to RegisterAction, will be executed
		typedef bool (__cdecl *ACTION_CALLBACK_FUNC)(InputDevice *from_device, size_t user, InputDevice::VirtualButton button);

		virtual size_t RegisterAction(const TCHAR *name, InputDevice::VirtualButton default_button, ACTION_CALLBACK_FUNC func) = NULL;

		virtual bool AssignActionButton(size_t action_index, const TCHAR *device_name, InputDevice::VirtualButton default_button) = NULL;

		virtual bool UnregisterAction(size_t action_index) = NULL;

		virtual size_t GetNumActions() const = NULL;

		virtual size_t FindActionIndex(const TCHAR *name) const = NULL;

		virtual const TCHAR *GetActionName(size_t action_index) const = NULL;

		virtual bool LoadActions(const TCHAR *filename) = NULL;

		virtual bool SaveActions(const TCHAR *filename) const = NULL;

		virtual void Update() = NULL;

	};

};