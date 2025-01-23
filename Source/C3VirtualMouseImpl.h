// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDeviceImpl.h>


#define MOUSE_MOVEMULT		32


namespace c3
{

	class VirtualMouseImpl : public InputDeviceImpl
	{

	public:

		VirtualMouseImpl(System *sys, LPDIRECTINPUTDEVICE8 pdid);

		virtual ~VirtualMouseImpl();

		virtual DeviceType GetType() const { return InputDevice::DeviceType::MOUSE; };

		virtual size_t GetNumAxes() const { return 1; }

		virtual bool Update(float elapsed_seconds = 0.0f);

	protected:

	};

};