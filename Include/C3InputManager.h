// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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

		static void SetDeviceConnectionCallback(DEVICECONNECTION_CALLBACK_FUNC func, void *userdata);

		virtual size_t GetNumDevices() = NULL;

		virtual InputDevice *GetDevice(size_t idx) = NULL;

		virtual void AssignUser(const InputDevice *pdevice, size_t user) = NULL;

		virtual bool GetAssignedUser(const InputDevice *pdevice, size_t &user) const = NULL;

		virtual size_t GetNumUsers() const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY, float time = 0.0f) const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const = NULL;

		virtual void SetMousePos(int32_t x, int32_t y) = NULL;

		virtual void GetMousePos(int32_t &x, int32_t &y) = NULL;

		virtual void AcquireAll() = NULL;

		virtual void UnacquireAll() = NULL;

		// Removes and re-enumerates all devices - this should be handled when a WM_DEVICECHANGE message has been received
		// If you are directly getting devices, rather than using the input manager, you will need to re-get your devices after this
		virtual void Reset() = NULL;

	};

};