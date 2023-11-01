// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDevice.h>


namespace c3
{

	class C3_API InputManager
	{

	public:
		enum { USER_ANY = -1, USER_DEFAULT };

		typedef bool (__cdecl *DEVICECONNECTION_CALLBACK_FUNC)(InputDevice *device, bool conn, void *userdata);

		// Sets the function that will be called when a device is connected or disconnected
		static void SetDeviceConnectionCallback(DEVICECONNECTION_CALLBACK_FUNC func, void *userdata);

		// Returns the number of input devices that have been identified
		virtual size_t GetNumDevices() = NULL;

		// Gets a Device by index
		virtual InputDevice *GetDevice(size_t idx) = NULL;

		// The User paradigm can be useful for local multiplayer where 3 physical devices (mouse / keyboard pair and a joypad)
		// supply input data for 2 players. AssignUser allows you to set that
		virtual void AssignUser(const InputDevice *pdevice, size_t user) = NULL;

		// Returns the user for the given Device
		virtual bool GetAssignedUser(const InputDevice *pdevice, size_t &user) const = NULL;

		// Returns the number of individual users
		virtual size_t GetNumUsers() const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY, float time = 0.0f) const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		// Sets the mouse position (primarily for UI) in desktop space
		virtual void SetMousePos(int32_t x, int32_t y) = NULL;

		// Gets the mouse position (primarily for UI) in desktop space
		virtual void GetMousePos(int32_t &x, int32_t &y) = NULL;

		// Acquires all connected Devices; this may be necessary when restoring focus to your application
		virtual void AcquireAll() = NULL;

		// Unacquires all connected devices; be polite and let others use exclusive devices when your application loses focus
		virtual void UnacquireAll() = NULL;

		// Removes and re-enumerates all devices - this should be handled when a WM_DEVICECHANGE message has been received
		// If you are directly getting devices, rather than using the input manager, you will need to re-get your devices after this
		virtual void Reset() = NULL;

	};

};