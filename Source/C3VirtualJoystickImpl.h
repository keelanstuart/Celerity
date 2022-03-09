// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDeviceImpl.h>



namespace c3
{

	class VirtualJoystickImpl : public InputDeviceImpl
	{

	public:

		VirtualJoystickImpl(System *sys, LPDIRECTINPUTDEVICE8 pdid);
		virtual ~VirtualJoystickImpl();

		// Typically, just polls the device and modifes the state/delta arrays... but could do ai...?
		// Returns FALSE if there was no change in any button states
		virtual bool Update(float elapsed_seconds = 0.0f);

		virtual DeviceType GetType() const { return InputDevice::DeviceType::JOYSTICK; }

		//virtual void PlayForceFeedbackEffect(const TCHAR* fullpath, int32_t dir_offset = 0) = NULL;

	protected:

	};

};