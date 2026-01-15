// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"
#include <C3VirtualKeyboardImpl.h>

using namespace c3;

VirtualKeyboardImpl::VirtualKeyboardImpl(System *psys, LPDIRECTINPUTDEVICE8 pdid) : InputDeviceImpl(psys, pdid)
{
	HRESULT hr = m_pDIDevice->SetDataFormat(&c_dfDIKeyboard);

	SetDefaultKeyMapping();
}


VirtualKeyboardImpl::~VirtualKeyboardImpl()
{
}


bool VirtualKeyboardImpl::Update(float elapsed_seconds)
{
	HRESULT hr = m_pDIDevice->Poll();
	m_bAttached = SUCCEEDED(hr);

	uint8_t raw_states[HW_KB_KEYS];
	ZeroMemory(&raw_states, HW_KB_KEYS);

	// Get the input's device state, and put the state in dims
	if (m_bAttached)
		hr = m_pDIDevice->GetDeviceState(sizeof(raw_states), &raw_states);

	bool button_downnow[InputDevice::MAXBUTTONS];
	memset(button_downnow, 0, sizeof(bool) * InputDevice::MAXBUTTONS);

	// go through each key in the raw keyboard data...
	for (size_t i = 0; i < HW_KB_KEYS; i++)
	{
		// get the "virtual" key that the real key maps to...
		int mapto = real_to_virtual[i];

		// if the mapping is valid, then update our virtual keys...
		if ((mapto >= 0) && raw_states[i])
			button_downnow[mapto] |= true;
	}

	for (size_t j = 0; j < InputDevice::MAXBUTTONS; j++)
	{
		int tmpstate = button_downnow[j] ? InputDevice::BUTTONVAL_MAX : 0;
		m_ButtonDelta[j] = tmpstate - m_ButtonState[j];
		m_ButtonState[j] = tmpstate;
	}

	return __super::Update(elapsed_seconds);
}

void VirtualKeyboardImpl::SetDefaultKeyMapping()
{
	// Reset all our mappings
	for (size_t i = 0; i < HW_KB_KEYS; i++)
		real_to_virtual[i] = -1;
	
	// map our debug key to the ctrl keys
	real_to_virtual[DIK_LCONTROL]	= InputDevice::VirtualButton::LCTRL;
	real_to_virtual[DIK_RCONTROL]	= InputDevice::VirtualButton::RCTRL;

	// delete key
	real_to_virtual[DIK_DELETE]			= InputDevice::VirtualButton::DELETEKEY;
	real_to_virtual[DIK_NUMPADPERIOD]	= InputDevice::VirtualButton::DELETEKEY;

	real_to_virtual[DIK_BACK]		= InputDevice::VirtualButton::NAV_BACK;

	// map enter to start, and space to select
	real_to_virtual[DIK_RETURN]		= InputDevice::VirtualButton::START;
	real_to_virtual[DIK_SPACE]		= InputDevice::VirtualButton::SELECT;

	real_to_virtual[DIK_ESCAPE]		= InputDevice::VirtualButton::QUIT;

	// the first directional set is the arrow key set...
	real_to_virtual[DIK_UP]			= InputDevice::VirtualButton::AXIS1_POSY;
	real_to_virtual[DIK_DOWN]		= InputDevice::VirtualButton::AXIS1_NEGY;
	real_to_virtual[DIK_LEFT]		= InputDevice::VirtualButton::AXIS1_NEGX;
	real_to_virtual[DIK_RIGHT]		= InputDevice::VirtualButton::AXIS1_POSX;

	// the second directional set is the number pad...
	real_to_virtual[DIK_NUMPAD8]	= InputDevice::VirtualButton::AXIS2_POSY;
	real_to_virtual[DIK_NUMPAD2]	= InputDevice::VirtualButton::AXIS2_NEGY;
	real_to_virtual[DIK_NUMPAD4]	= InputDevice::VirtualButton::AXIS2_NEGX;
	real_to_virtual[DIK_NUMPAD6]	= InputDevice::VirtualButton::AXIS2_POSX;

	// duplicate the dir1 left/right on the number pad for easier controls...
	real_to_virtual[DIK_NUMPAD7]	= InputDevice::VirtualButton::AXIS1_NEGX;
	real_to_virtual[DIK_NUMPAD9]	= InputDevice::VirtualButton::AXIS1_POSX;

	real_to_virtual[DIK_SPACE]		= InputDevice::VirtualButton::BUTTON3;
	real_to_virtual[DIK_LSHIFT]		= InputDevice::VirtualButton::LSHIFT;
	real_to_virtual[DIK_RSHIFT]		= InputDevice::VirtualButton::RSHIFT;

	// characters (keyboard emulation)
	real_to_virtual[DIK_A]			= InputDevice::VirtualButton::LETTER_A;
	real_to_virtual[DIK_B]			= InputDevice::VirtualButton::LETTER_B;
	real_to_virtual[DIK_C]			= InputDevice::VirtualButton::LETTER_C;
	real_to_virtual[DIK_D]			= InputDevice::VirtualButton::LETTER_D;
	real_to_virtual[DIK_E]			= InputDevice::VirtualButton::LETTER_E;
	real_to_virtual[DIK_F]			= InputDevice::VirtualButton::LETTER_F;
	real_to_virtual[DIK_G]			= InputDevice::VirtualButton::LETTER_G;
	real_to_virtual[DIK_H]			= InputDevice::VirtualButton::LETTER_H;
	real_to_virtual[DIK_I]			= InputDevice::VirtualButton::LETTER_I;
	real_to_virtual[DIK_J]			= InputDevice::VirtualButton::LETTER_J;
	real_to_virtual[DIK_K]			= InputDevice::VirtualButton::LETTER_K;
	real_to_virtual[DIK_L]			= InputDevice::VirtualButton::LETTER_L;
	real_to_virtual[DIK_M]			= InputDevice::VirtualButton::LETTER_M;
	real_to_virtual[DIK_N]			= InputDevice::VirtualButton::LETTER_N;
	real_to_virtual[DIK_O]			= InputDevice::VirtualButton::LETTER_O;
	real_to_virtual[DIK_P]			= InputDevice::VirtualButton::LETTER_P;
	real_to_virtual[DIK_Q]			= InputDevice::VirtualButton::LETTER_Q;
	real_to_virtual[DIK_R]			= InputDevice::VirtualButton::LETTER_R;
	real_to_virtual[DIK_S]			= InputDevice::VirtualButton::LETTER_S;
	real_to_virtual[DIK_T]			= InputDevice::VirtualButton::LETTER_T;
	real_to_virtual[DIK_U]			= InputDevice::VirtualButton::LETTER_U;
	real_to_virtual[DIK_V]			= InputDevice::VirtualButton::LETTER_V;
	real_to_virtual[DIK_W]			= InputDevice::VirtualButton::LETTER_W;
	real_to_virtual[DIK_X]			= InputDevice::VirtualButton::LETTER_X;
	real_to_virtual[DIK_Y]			= InputDevice::VirtualButton::LETTER_Y;
	real_to_virtual[DIK_Z]			= InputDevice::VirtualButton::LETTER_Z;

	real_to_virtual[DIK_TAB]		= InputDevice::VirtualButton::TAB;
	real_to_virtual[DIK_GRAVE]		= InputDevice::VirtualButton::TILDE;

	// numbers (keyboard emulation)
	real_to_virtual[DIK_0]			= InputDevice::VirtualButton::NUM0;
	real_to_virtual[DIK_1]			= InputDevice::VirtualButton::NUM1;
	real_to_virtual[DIK_2]			= InputDevice::VirtualButton::NUM2;
	real_to_virtual[DIK_3]			= InputDevice::VirtualButton::NUM3;
	real_to_virtual[DIK_4]			= InputDevice::VirtualButton::NUM4;
	real_to_virtual[DIK_5]			= InputDevice::VirtualButton::NUM5;
	real_to_virtual[DIK_6]			= InputDevice::VirtualButton::NUM6;
	real_to_virtual[DIK_7]			= InputDevice::VirtualButton::NUM7;
	real_to_virtual[DIK_8]			= InputDevice::VirtualButton::NUM8;
	real_to_virtual[DIK_9]			= InputDevice::VirtualButton::NUM9;

	real_to_virtual[DIK_F1]			= InputDevice::VirtualButton::HELP;
	real_to_virtual[DIK_F2]			= InputDevice::VirtualButton::LOAD;
	real_to_virtual[DIK_F3]			= InputDevice::VirtualButton::SAVE;
	real_to_virtual[DIK_F11]		= InputDevice::VirtualButton::DEBUGBUTTON;
}
