// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputManager.h>
#include <C3InputDeviceImpl.h>
#include <deque>



namespace c3
{
	class InputManagerImpl : public InputManager
	{

	public:
		InputManagerImpl(System* sys);
		virtual ~InputManagerImpl();

		bool Initialize(HWND app_hwnd, HINSTANCE app_inst);

		void Shutdown();

		virtual void Update(float elapsed_seconds = 0.0f);

		virtual void SetMousePos(int32_t x, int32_t y);

		virtual void GetMousePos(int32_t &x, int32_t &y);

		virtual void AssignUser(const InputDevice *pdevice, size_t user);

		virtual bool GetAssignedUser(const InputDevice *pdevice, size_t &user) const;

		virtual size_t GetNumUsers() const;

		//InputDevice *AddVirtualDevice(size_t user, uint32_t devicetype, const TCHAR* device_name = NULL);

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY, float time = 0.0f) const;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, size_t user = USER_ANY) const;

		virtual void AcquireAll();
		virtual void UnacquireAll();

		virtual size_t GetNumDevices();

		virtual InputDevice* GetDevice(size_t idx);

		void PlayForceFeedbackEffect(const TCHAR* filename, int32_t dir_offset = 0);

	protected:
		System *m_pSys;

		// A list of devices and the user number they've been assigned
		typedef std::deque<std::pair<InputDeviceImpl *, size_t>> TDeviceArray;
		TDeviceArray m_Devices;
		size_t m_NumUsers;

		LPDIRECTINPUT8 m_pDI;

		static bool CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* did_instance, void* context);

		// We need these to determine our level of device cooperation and for DI to know who to deal with
		HWND m_hwnd;
		HINSTANCE m_hinst;

		float m_PlugCheckTime;

		struct
		{
			int32_t x, y;
		} m_MousePos;

	};
};