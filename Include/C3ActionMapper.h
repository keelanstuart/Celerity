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

		// 
		typedef bool (__cdecl *ACTION_CALLBACK_FUNC)(InputDevice *from_device, size_t user, InputDevice::VirtualButton button);

		virtual size_t RegisterAction(const TCHAR *name, ACTION_CALLBACK_FUNC func) = NULL;

		virtual bool UnregisterAction(size_t index) = NULL;

		virtual size_t GetNumActions() const = NULL;

		virtual size_t FindActionIndex(const TCHAR *name) const = NULL;

		virtual const TCHAR *GetActionName(size_t index) const = NULL;

	};

};