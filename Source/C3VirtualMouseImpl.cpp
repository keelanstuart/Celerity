// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"
#include <C3VirtualMouseImpl.h>


using namespace c3;


VirtualMouseImpl::VirtualMouseImpl(System *psys, LPDIRECTINPUTDEVICE8 pdid) : InputDeviceImpl(psys, pdid)
{
	// Set the data format to the default one for mice...
	if (m_pDIDevice)
		m_pDIDevice->SetDataFormat(&c_dfDIMouse2);
}



VirtualMouseImpl::~VirtualMouseImpl()
{
}



// Typically, just polls the device and modifes the state/delta arrays... but could do ai...?
// Returns FALSE if there was no change in any button states
bool VirtualMouseImpl::Update(float elapsed_seconds)
{
	HRESULT hr = m_pDIDevice->Poll();
	m_bAttached = true;//SUCCEEDED(hr);

	DIMOUSESTATE2 state;      // DirectInput mouse state structure
	ZeroMemory(&state, sizeof(DIMOUSESTATE2));

	// Get the input's device state
	if (m_bAttached)
		hr = m_pDIDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &state);

	// reset all of our motion deltas...
	m_ButtonState[InputDevice::VirtualButton::AXIS2_POSX] >>= 1;
	m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGX] >>= 1;
	m_ButtonState[InputDevice::VirtualButton::AXIS2_POSY] >>= 1;
	m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGY] >>= 1;

	m_ButtonDelta[InputDevice::VirtualButton::AXIS2_POSX] = 0;
	m_ButtonDelta[InputDevice::VirtualButton::AXIS2_NEGX] = 0;
	m_ButtonDelta[InputDevice::VirtualButton::AXIS2_POSY] = 0;
	m_ButtonDelta[InputDevice::VirtualButton::AXIS2_NEGY] = 0;

	// Did we move horizontally?
	if (state.lX)
	{
		if (state.lX > 0)
		{
			m_ButtonState[InputDevice::VirtualButton::AXIS2_POSX] = abs(state.lX) * MOUSE_MOVEMULT;	// right motion
			m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGX] = 0;
		}
		else
		{
			m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGX] = abs(state.lX) * MOUSE_MOVEMULT;		// left motion
			m_ButtonState[InputDevice::VirtualButton::AXIS2_POSX] = 0;
		}
	}

	// Did we move vertically?
	if (state.lY)
	{
		if (state.lY > 0)
		{
			m_ButtonState[InputDevice::VirtualButton::AXIS2_POSY] = abs(state.lY) * MOUSE_MOVEMULT;		// down motion
			m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGY] = 0;
		}
		else
		{
			m_ButtonState[InputDevice::VirtualButton::AXIS2_NEGY] = abs(state.lY) * MOUSE_MOVEMULT;		// up motion
			m_ButtonState[InputDevice::VirtualButton::AXIS2_POSY] = 0;
		}
	}

	m_ButtonState[InputDevice::VirtualButton::THROTTLE1] = 0;
	m_ButtonDelta[InputDevice::VirtualButton::THROTTLE1] = 0;

	// Did we roll our mouse wheel?
	if (state.lZ)
	{
		m_ButtonState[InputDevice::VirtualButton::THROTTLE1] = -state.lZ;
		m_ButtonDelta[InputDevice::VirtualButton::THROTTLE1] = -state.lZ;
	}

	for (uint32_t i = 0; i < 8; i++)
	{
		if (state.rgbButtons[i] & 0x80)	// if the high-order bit is set, the button is being pressed.
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

	return __super::Update(elapsed_seconds);
}
