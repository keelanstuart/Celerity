// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDeviceImpl.h>


namespace c3
{

	class VirtualKeyboardImpl : public InputDeviceImpl
	{

	public:

		VirtualKeyboardImpl(System *sys, LPDIRECTINPUTDEVICE8 pdid);
		virtual ~VirtualKeyboardImpl();

		virtual DeviceType GetType() const { return InputDevice::DeviceType::KEYBOARD; };

		virtual bool Update(float elapsed_seconds = 0.0f);

		void SetDefaultKeyMapping();

	protected:
		int real_to_virtual[InputDevice::MAXBUTTONS];

	};

};