// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


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
	else
		ZeroMemory(m_ButtonState, sizeof(m_ButtonState));

	ZeroMemory(m_ButtonDelta, sizeof(m_ButtonDelta));

	auto Dampen = [](int val) -> int
	{
		float pct = (float)val / (float)InputDevice::BUTTONVAL_MAX;
		pct *= pct;
		return (int)(pct * (float)InputDevice::BUTTONVAL_MAX);
	};

	auto SetButton = [&](size_t butidx, int val)
	{
		if (val != m_ButtonState[butidx])
		{
			// Signed delta: positive on press, negative on release
			m_ButtonDelta[butidx] = val - m_ButtonState[butidx];
		}
		else
		{
			m_ButtonDelta[butidx] = 0;
		}

		m_ButtonState[butidx] = val;
	};

	constexpr LONG deadzone = (InputDevice::BUTTONVAL_MAX / 16);

	int32_t iminprop = (int32_t)(255.0f * m_MinProportion);
	if (abs(state.lX) > deadzone)
	{
		if ((state.lX > 0) && (state.lX > iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_POSX, Dampen(abs(state.lX)));	// right motion
		else if ((state.lX < 0) && (state.lX < -iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_NEGX, Dampen(abs(state.lX)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS1_NEGX, 0);
			SetButton(InputDevice::VirtualButton::AXIS1_POSX, 0);
		}
	}

	if (abs(state.lY) > deadzone)
	{
		if ((state.lY > 0) && (state.lY > iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_NEGY, Dampen(abs(state.lY)));	// right motion
		else if ((state.lY < 0) && (state.lY < -iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_POSY, Dampen(abs(state.lY)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS1_NEGY, 0);
			SetButton(InputDevice::VirtualButton::AXIS1_POSY, 0);
		}
	}

	if (abs(state.lZ) > deadzone)
	{
		if ((state.lZ > 0) && (state.lZ > iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_POSZ, Dampen(abs(state.lZ)));	// right motion
		else if ((state.lZ < 0) && (state.lZ < -iminprop))
			SetButton(InputDevice::VirtualButton::AXIS1_NEGZ, Dampen(abs(state.lZ)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS1_NEGZ, 0);
			SetButton(InputDevice::VirtualButton::AXIS1_POSZ, 0);
		}
	}

	if (abs(state.lRx) > deadzone)
	{
		if ((state.lRx > 0) && (state.lRx > iminprop))
			SetButton(InputDevice::VirtualButton::AXIS2_POSX, Dampen(abs(state.lRx)));	// right motion
		else if ((state.lRx < 0) && (state.lRx < -iminprop))
			SetButton(InputDevice::VirtualButton::AXIS2_NEGX, Dampen(abs(state.lRx)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS2_NEGX, 0);
			SetButton(InputDevice::VirtualButton::AXIS2_POSX, 0);
		}
	}

	if (abs(state.lRy) > deadzone)
	{
		if ((state.lRy > 0) && (state.lRy > iminprop))
			SetButton(InputDevice::VirtualButton::AXIS2_POSY, Dampen(abs(state.lRy)));	// right motion
		else if ((state.lRy < 0) && (state.lRy < -iminprop))
			SetButton(InputDevice::VirtualButton::AXIS2_NEGY, Dampen(abs(state.lRy)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS2_NEGY, 0);
			SetButton(InputDevice::VirtualButton::AXIS2_POSY, 0);
		}
	}

	if (abs(state.lRz) > deadzone)
	{
		if ((state.lRz > iminprop) && (state.lRz > 0))
			SetButton(InputDevice::VirtualButton::AXIS2_POSZ, Dampen(abs(state.lRz)));	// right motion
		else if ((state.lRz < -iminprop) && (state.lRz < 0))
			SetButton(InputDevice::VirtualButton::AXIS2_NEGZ, Dampen(abs(state.lRz)));	// left motion
		else
		{
			SetButton(InputDevice::VirtualButton::AXIS2_NEGZ, 0);
			SetButton(InputDevice::VirtualButton::AXIS2_POSZ, 0);
		}
	}

	SetButton(InputDevice::VirtualButton::THROTTLE1, state.rglSlider[0]);
	SetButton(InputDevice::VirtualButton::THROTTLE2, state.rglSlider[1]);

	switch (state.rgdwPOV[0])
	{
		case 0:
			SetButton(InputDevice::VirtualButton::POV_POSY, InputDevice::BUTTONVAL_MAX);
			break;
	
		case 18000:
			SetButton(InputDevice::VirtualButton::POV_NEGY, InputDevice::BUTTONVAL_MAX);
			break;
	
		case 9000:
			SetButton(InputDevice::VirtualButton::POV_POSX, InputDevice::BUTTONVAL_MAX);
			break;
	
		case 27000:
			SetButton(InputDevice::VirtualButton::POV_NEGX, InputDevice::BUTTONVAL_MAX);
			break;
	}

	constexpr size_t DI_BUTTON1 = 21;

	for (uint32_t i = InputDevice::BUTTON1, j = 0; i <= InputDevice::BUTTON12; i++, j++)
	{
		if (state.rgbButtons[j])
			SetButton(i, InputDevice::BUTTONVAL_MAX);
		else
			SetButton(i, 0);
	}

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

