// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDevice.h>


namespace c3
{

	class ActionMapper
	{

	public:

		using TriggerType = enum ETriggerType
		{
			DOWN_CONTINUOUS = 0,	// keeps generating the event as long as the button is down
			DOWN_DELTA,				// generates the event only when the button is first depressed
			UP_DELTA,				// generates the event only when the button is released
		};

		// a callback you provide when your action is triggered. gives you the device, user number, which button, and that button's value
		typedef bool (__cdecl *ACTION_CALLBACK_FUNC)(InputDevice *from_device, size_t user, InputDevice::VirtualButton button, float value, void *userdata);

		// registers an action with a name and your callback
		virtual size_t RegisterAction(const TCHAR *name, TriggerType trigger, float delay, ACTION_CALLBACK_FUNC func, void *userdata) = NULL;

		// unregisters a previously registered action
		virtual bool UnregisterAction(size_t index) = NULL;

		// returns the number of currently registered actions
		virtual size_t GetNumActions() const = NULL;

		// find the index of an action given its name
		virtual size_t FindActionIndex(const TCHAR *name) const = NULL;

		// returns the name of the action at the give index, or nullptr if the given index is invalid
		virtual const TCHAR *GetActionName(size_t index) const = NULL;

		// associates a device's button with a particular action
		virtual bool MakeAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button) = NULL;

		// removes an association 
		virtual bool BreakAssociation(size_t actionidx, uint32_t devid, InputDevice::VirtualButton button) = NULL;

	};

};