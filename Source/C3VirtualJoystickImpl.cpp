// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include <C3VirtualJoystickImpl.h>

using namespace c3;


BOOL FAR PASCAL EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, void *context)
{
	DIPROPRANGE diprg; 

	diprg.diph.dwSize       = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow        = DIPH_BYID;
	diprg.diph.dwObj        = pdidoi->dwType;		// Specify the enumerated axis
	diprg.lMin              = InputDevice::BUTTONVAL_MIN;
	diprg.lMax              = InputDevice::BUTTONVAL_MAX;

	// Set the range for the axis
	if (FAILED(((LPDIRECTINPUTDEVICE8)context)->SetProperty(DIPROP_RANGE, &diprg.diph)))
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
}

VirtualJoystickImpl::VirtualJoystickImpl(System *psys, LPDIRECTINPUTDEVICE8 pdid) : InputDeviceImpl(psys, pdid)
{
	m_MinProportion = 0.2f;

	// Set the data format to the default one for keyboards...
	m_pDIDevice->SetDataFormat(&c_dfDIJoystick2);

	DIPROPDWORD DIPropAutoCenter;

	DIPropAutoCenter.diph.dwSize       = sizeof(DIPropAutoCenter);
	DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIPropAutoCenter.diph.dwObj        = 0;
	DIPropAutoCenter.diph.dwHow        = DIPH_DEVICE;
	DIPropAutoCenter.dwData            = DIPROPAUTOCENTER_OFF;

	HRESULT hr = m_pDIDevice->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);
	if (FAILED(hr))
	{
		// Handle the failure as appropriate
	}

	// Set up the min/max ranges for all our axes
	m_pDIDevice->EnumObjects(EnumAxesCallback, m_pDIDevice, DIDFT_AXIS);
}


VirtualJoystickImpl::~VirtualJoystickImpl()
{
}


bool VirtualJoystickImpl::Update(float elapsed_seconds)
{
	HRESULT hr = m_pDIDevice->Poll();
	m_bAttached = SUCCEEDED(hr);

	DIJOYSTATE2 state;
	memset(&state, 0, sizeof(DIJOYSTATE2));
	if (m_bAttached)
		hr = m_pDIDevice->GetDeviceState(sizeof(DIJOYSTATE2), &state);

	ZeroMemory(m_ButtonState, sizeof(m_ButtonState));

	if (m_bAttached)
	{
		int32_t iminprop = (int32_t)(255.0f * m_MinProportion);
		if (abs(state.lX) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lX > 0) && (state.lX > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_POSX] = abs(state.lX);	// right motion

			if ((state.lX < 0) && (state.lX < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_NEGX] = abs(state.lX);	// left motion
		}

		if (abs(state.lY) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lY > 0) && (state.lY > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_NEGY] = abs(state.lY);	// right motion

			if ((state.lY < 0) && (state.lY < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_POSY] = abs(state.lY);		// left motion
		}

		if (abs(state.lZ) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lZ > 0) && (state.lZ > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_POSZ] = abs(state.lZ);	// right motion

			if ((state.lZ < 0) && (state.lZ < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS1_NEGZ] = abs(state.lZ);	// left motion
		}

		if (abs(state.lRx) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lRx > 0) && (state.lRx > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_POSX] = abs(state.lRx);	// right motion

			if ((state.lRx < 0) && (state.lRx < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGX] = abs(state.lRx);	// left motion
		}

		if (abs(state.lRy) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lRy > 0) && (state.lRy > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_POSY] = abs(state.lRy);	// right motion

			if ((state.lRy < 0) && (state.lRy < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGY] = abs(state.lRy);	// left motion
		}

		if (abs(state.lRz) > (InputDevice::BUTTONVAL_MAX / 16))
		{
			if ((state.lRz > 0) && (state.lRz > iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_POSZ] = abs(state.lRz);	// right motion

			if ((state.lRz < 0) && (state.lRz < -iminprop))
				m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGZ] = abs(state.lRz);	// left motion
		}

		m_ButtonState[InputDevice::VirtualButton::THROTTLE1] = state.rglSlider[0];
		m_ButtonState[InputDevice::VirtualButton::THROTTLE2] = state.rglSlider[1];
	}

	switch (state.rgdwPOV[0])
	{
		case 0:
			m_ButtonState[InputDevice::VirtualButton::POV_POSY] = InputDevice::BUTTONVAL_MAX;
			break;

		case 18000:
			m_ButtonState[InputDevice::VirtualButton::POV_NEGY] = InputDevice::BUTTONVAL_MAX;
			break;

		case 9000:
			m_ButtonState[InputDevice::VirtualButton::POV_POSX] = InputDevice::BUTTONVAL_MAX;
			break;

		case 27000:
			m_ButtonState[InputDevice::VirtualButton::POV_NEGX] = InputDevice::BUTTONVAL_MAX;
			break;
	}

	for (uint32_t i = 0; i < InputDevice::MAXBUTTONS; i++)
	{
		if (state.rgbButtons[i] >= (InputDevice::BUTTONVAL_MAX / 16))	// if the high-order bit is set, the button is being pressed.
		{
			if (!m_ButtonState[InputDevice::VirtualButton::BUTTON1 + i])
				m_ButtonDelta[InputDevice::VirtualButton::BUTTON1 + i] = InputDevice::BUTTONVAL_MAX;
			else
				m_ButtonDelta[InputDevice::VirtualButton::BUTTON1 + i] = 0;

			m_ButtonState[InputDevice::VirtualButton::BUTTON1 + i] = InputDevice::BUTTONVAL_MAX;
		}
		else
		{
			if (m_ButtonState[InputDevice::VirtualButton::BUTTON1 + i])
				m_ButtonDelta[InputDevice::VirtualButton::BUTTON1 + i] = InputDevice::BUTTONVAL_MAX;
			else
				m_ButtonDelta[InputDevice::VirtualButton::BUTTON1 + i] = 0;

			m_ButtonState[InputDevice::VirtualButton::BUTTON1 + i] = 0;
		}
	}

	m_ButtonState[InputDevice::VirtualButton::LSHIFT] = m_ButtonState[InputDevice::VirtualButton::BUTTON9];
	m_ButtonDelta[InputDevice::VirtualButton::RSHIFT] = m_ButtonDelta[InputDevice::VirtualButton::BUTTON10];

	__super::Update(elapsed_seconds);

	return true;
}

size_t VirtualJoystickImpl::GetNumAxes() const
{
	return m_DIDCaps.dwAxes;
}

size_t VirtualJoystickImpl::GetNumButtons() const
{
	return m_DIDCaps.dwButtons;
}

