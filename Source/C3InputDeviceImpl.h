// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDevice.h>

#define DIRECTINPUT_VERSION         DIRECTINPUT_HEADER_VERSION
#include <dinput.h>


#define MAXEFFCOUNT		5

struct riffdata
{
	LPDIRECTINPUTDEVICE8 pDIDevice;
	uint32_t effcount;
	LPDIRECTINPUTEFFECT eff[MAXEFFCOUNT];
};



// **************************************************************************************************
// **************************************************************************************************
// **************************************************************************************************

namespace c3
{
	class InputDeviceImpl : public InputDevice
	{

	public:

		InputDeviceImpl(System *psys, LPDIRECTINPUTDEVICE8 pdid);

		virtual ~InputDeviceImpl();

		virtual bool Update(float elapsed_seconds = 0.0f);

		virtual const TCHAR *GetName() const;

		virtual uint32_t GetUID() const;

		virtual DeviceType GetType() const = NULL;

		virtual size_t GetNumAxes() const;

		virtual size_t GetNumButtons() const;

		virtual int ButtonPressed(InputDevice::VirtualButton button, float time = 0.0f) const;

		virtual float ButtonPressedProportional(InputDevice::VirtualButton button) const;

		virtual int ButtonChange(InputDevice::VirtualButton button) const;

		virtual bool ButtonReleased(InputDevice::VirtualButton button) const;

		virtual void SetProportionRange(float minprop, float maxprop);

		//virtual void PlayForceFeedbackEffect(const TCHAR* fullpath, int32_t dir_offset = 0) = NULL;

		virtual bool PluggedIn() const;

		virtual void Activate(bool isactive = true);

		virtual bool IsActive() const;

		virtual void Acquire();

		virtual void Unacquire();

		virtual bool IsDIDevice(LPDIRECTINPUTDEVICE8 pdid);

	protected:

		System *m_pSys;

		tstring m_Name;

		uint32_t m_UID;

		// This will be set if the device has been accquired
		bool m_bActive;

		// the state of a virtual button
		int m_ButtonState[MAXBUTTONS];

		// the difference between the current state of the button and the last state of the button
		int m_ButtonDelta[MAXBUTTONS];

		// how long this button has been pressed in seconds
		float m_ButtonTime[MAXBUTTONS];

		// The DI stuff
		LPDIRECTINPUTDEVICE8 m_pDIDevice;
		DIDEVICEINSTANCE m_DIDInst;
		DIDEVCAPS m_DIDCaps;

		float m_MinProportion;
		float m_MaxProportion;

		riffdata ffcbdata;

		// True if the device is unplugged
		bool m_bAttached;
	};

};