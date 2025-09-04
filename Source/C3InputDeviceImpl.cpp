// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include <C3InputDeviceImpl.h>
#include <C3ResourceManager.h>
#include <C3FileMapper.h>
#include <C3CRC.h>


using namespace c3;

InputDeviceImpl::InputDeviceImpl(System *sys, LPDIRECTINPUTDEVICE8 pdid)
{
	m_pSys = sys;
	m_pDIDevice = pdid;

	// reset the button states
	memset(m_ButtonState, 0, sizeof(int32_t) * MAXBUTTONS);

	// reset the button deltas
	memset(m_ButtonDelta, 0, sizeof(int32_t) * MAXBUTTONS);

	// reset the button times
	memset(m_ButtonTime, 0xFF, sizeof(int32_t) * MAXBUTTONS);

	m_bActive = true;

	m_MinProportion = 0.0f;
	m_MaxProportion = 1.0f;

	memset(&ffcbdata, 0, sizeof(riffdata));

	if (!pdid)
		return;

	memset(&m_DIDInst, 0, sizeof(DIDEVICEINSTANCE));
	m_DIDInst.dwSize = sizeof(DIDEVICEINSTANCE);
	m_pDIDevice->GetDeviceInfo(&m_DIDInst);

	m_Name = m_DIDInst.tszProductName;
//	m_Name += _T(" (");
//	m_Name += m_DIDInst.tszInstanceName;
//	m_Name += _T(")");

	m_UID = c3::Crc32::CalculateString(m_Name.c_str());

	memset(&m_DIDCaps, 0, sizeof(DIDEVCAPS));
	m_DIDCaps.dwSize = sizeof(DIDEVCAPS);
	pdid->GetCapabilities(&m_DIDCaps);

	// Set the cooperative level...
	HRESULT hr = m_pDIDevice->SetCooperativeLevel(sys->GetOwner(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
}


InputDeviceImpl::~InputDeviceImpl()
{
	if (m_pDIDevice)
	{
		m_pDIDevice->Release();
		m_pDIDevice = NULL;
	}
}


System *InputDeviceImpl::GetSystem() const
{
	return m_pSys;
}


const TCHAR *InputDeviceImpl::GetName() const
{
	return m_Name.c_str();
}


uint32_t InputDeviceImpl::GetUID() const
{
	return m_UID;
}


bool InputDeviceImpl::ButtonPressed(InputDevice::VirtualButton button, float time)
{
	if (button == ANY)
	{
		for (uint32_t i = 0; i < NUMBUTTONS; i++)
		{
			if (m_ButtonState[i] && (m_ButtonTime[i] >= time))
			{
				m_ButtonTime[i] = 0.0f;
				return true;
			}
		}

		return false;
	}

	if (m_ButtonState[button] && (m_ButtonTime[button] >= time))
	{
		m_ButtonTime[button] = 0.0f;
		return true;
	}

	return false;
}

float InputDeviceImpl::ButtonPressedProportional(InputDevice::VirtualButton button) const
{
	float propval = ((float)m_ButtonState[button]) / ((float)BUTTONVAL_MAX);

	if (propval <= m_MinProportion)
		propval = 0.0f;

	if (propval >= m_MaxProportion)
		propval = 1.0f;

	return  propval;
}

int32_t InputDeviceImpl::ButtonChange(InputDevice::VirtualButton button) const
{
	return m_ButtonDelta[button];
}

bool InputDeviceImpl::ButtonReleased(InputDevice::VirtualButton button) const
{
	if (!m_ButtonState[button] && m_ButtonDelta[button])
	{
		return true;
	}

	return false;
}


bool InputDeviceImpl::Update(float elapsed_seconds)
{
	for (size_t i = 0; i < NUMBUTTONS; i++)
	{
		if (!m_ButtonState[i])
		{
			m_ButtonTime[i] = 0.0f;
			continue;
		}

		m_ButtonTime[i] += elapsed_seconds;
	}

	return m_bAttached;
}

#if 0
bool CALLBACK EffectsEnumCallback(LPCDIFILEEFFECT lpDiFileEf, LPVOID pvRef)
{
	if (((riffdata *)pvRef)->effcount < MAXEFFCOUNT)
	{
		HRESULT hr;

		// Create the file effect
		hr = ((riffdata *)pvRef)->pDIDevice->CreateEffect(lpDiFileEf->GuidEffect, lpDiFileEf->lpDiEffect,
			&(((riffdata *)pvRef)->eff[((riffdata *)pvRef)->effcount]), NULL);

		if (SUCCEEDED(hr))
			((riffdata *)pvRef)->effcount++;

		return DIENUM_CONTINUE;
	}

	return DIENUM_STOP;
}


void InputDeviceImpl::PlayForceFeedbackEffect(const TCHAR *fullpath, int32_t dir_offset)
{
	if (diDevCaps.dwFlags & DIDC_FORCEFEEDBACK)
	{
		{
			for (uint32_t i = 0; i < ffcbdata.effcount; i++)
			{
				ffcbdata.eff[i]->Unload();
			}

			memset(&ffcbdata, 0, sizeof(riffdata));
			ffcbdata.pDIDevice = pDIDevice;
			pDIDevice->EnumEffectsInFile(fullpath, EffectsEnumCallback, &ffcbdata, DIFEF_MODIFYIFNEEDED);

			if (ffcbdata.effcount)
			{
				pDIDevice->SendForceFeedbackCommand(DISFFC_STOPALL);

				for (uint32_t i = 0; i < ffcbdata.effcount; i++)
				{
					if (dir_offset)
					{
						DIEFFECT tmp;
						memset(&tmp, 0, sizeof(DIEFFECT));
						tmp.dwSize = sizeof(DIEFFECT);

						ffcbdata.eff[i]->GetParameters(&tmp, DIEP_DIRECTION);

						if (tmp.dwFlags | DIEFF_POLAR)
						{
							for (uint32_t j = 0; j < tmp.cAxes; j++)
								tmp.rglDirection[j] += (dir_offset * 100);
							ffcbdata.eff[i]->SetParameters(&tmp, DIEP_DIRECTION);
						}
					}

					ffcbdata.eff[i]->Start(1, 0);
				}
			}
		}
	}
}
#endif

void InputDeviceImpl::Acquire()
{
	if (m_pDIDevice)
	{
		m_pDIDevice->Acquire();
	}
}

void InputDeviceImpl::Unacquire()
{
	if (m_pDIDevice)
	{
		m_pDIDevice->Unacquire();
	}
}

bool InputDeviceImpl::IsActive() const
{
	return m_bActive;
}

void InputDeviceImpl::Activate(bool b)
{
	m_bActive = b;
}

void InputDeviceImpl::SetProportionRange(float minprop, float maxprop)
{
	m_MinProportion = minprop;
	m_MaxProportion = maxprop;
}

bool InputDeviceImpl::PluggedIn() const
{
	return m_bAttached;
}


bool InputDeviceImpl::IsDIDevice(LPDIRECTINPUTDEVICE8 pdid)
{
	return (pdid == m_pDIDevice) ? true : false;
}


size_t InputDeviceImpl::GetNumAxes() const
{
	return m_DIDCaps.dwAxes;
}


size_t InputDeviceImpl::GetNumButtons() const
{
	return m_DIDCaps.dwButtons;
}


const TCHAR *ButtonName[InputDevice::NUMBUTTONS] =
{
	_T("debug"),			//DEBUGBUTTON = 0,
	_T("help"),				//HELP,
	_T("start"),			//START,
	_T("select"),			//SELECT,
	_T("quit"),				//QUIT,
	_T("axis 1 -y"),		//AXIS1_NEGY,
	_T("axis 1 +y"),		//AXIS1_POSY,
	_T("axis 1 -x"),		//AXIS1_NEGX,
	_T("axis 1 +x"),		//AXIS1_POSX,
	_T("axis 1 -z"),		//AXIS1_NEGZ,
	_T("axis 1 +z"),		//AXIS1_POSZ,
	_T("axis 2 -y"),		//AXIS2_NEGY,
	_T("axis 2 +y"),		//AXIS2_POSY,
	_T("axis 2 -x"),		//AXIS2_NEGX,
	_T("axis 2 +x"),		//AXIS2_POSX,
	_T("axis 2 -z"),		//AXIS2_NEGZ,
	_T("axis 2 +z"),		//AXIS2_POSZ,
	_T("pov +y"),			//POV_POSY,
	_T("pov -y"),			//POV_NEGY,
	_T("pov +x"),			//POV_POSX,
	_T("pov -x"),			//POV_NEGX,
	_T("button 1"),			//BUTTON1,
	_T("button 2"),			//BUTTON2,
	_T("button 3"),			//BUTTON3,
	_T("button 4"),			//BUTTON4,
	_T("button 5"),			//BUTTON5,
	_T("button 6"),			//BUTTON6,
	_T("button 7"),			//BUTTON7,
	_T("button 8"),			//BUTTON8,
	_T("button 9"),			//BUTTON9,
	_T("button 10"),		//BUTTON10,
	_T("button 11"),		//BUTTON11,
	_T("button 12"),		//BUTTON12,
	_T("throttle 1"),		//THROTTLE1,
	_T("throttle 2"),		//THROTTLE2,
	_T("l shift"),			//LSHIFT,
	_T("r shift"),			//RSHIFT,
	_T("l ctrl"),			//LCTRL,
	_T("r ctrl"),			//RCTRL,
	_T("a"),				//LETTER_A,
	_T("b"),				//LETTER_B,
	_T("c"),				//LETTER_C,
	_T("d"),				//LETTER_D,
	_T("e"),				//LETTER_E,
	_T("f"),				//LETTER_F,
	_T("g"),				//LETTER_G,
	_T("h"),				//LETTER_H,
	_T("i"),				//LETTER_I,
	_T("j"),				//LETTER_J,
	_T("k"),				//LETTER_K,
	_T("l"),				//LETTER_L,
	_T("m"),				//LETTER_M,
	_T("n"),				//LETTER_N,
	_T("o"),				//LETTER_O,
	_T("p"),				//LETTER_P,
	_T("q"),				//LETTER_Q,
	_T("r"),				//LETTER_R,
	_T("s"),				//LETTER_S,
	_T("t"),				//LETTER_T,
	_T("u"),				//LETTER_U,
	_T("v"),				//LETTER_V,
	_T("w"),				//LETTER_W,
	_T("x"),				//LETTER_X,
	_T("y"),				//LETTER_Y,
	_T("z"),				//LETTER_Z,
	_T("~"),				//TILDE,
	_T("tab"),				//TAB,
	_T("delete"),			//DELETEKEY,
	_T("back"),				//NAV_BACK,
	_T("forward"),			//NAV_FORWARD,
	_T("0"),				//NUM0,
	_T("1"),				//NUM1,
	_T("2"),				//NUM2,
	_T("3"),				//NUM3,
	_T("4"),				//NUM4,
	_T("5"),				//NUM5,
	_T("6"),				//NUM6,
	_T("7"),				//NUM7,
	_T("8"),				//NUM8,
	_T("9"),				//NUM9,
	_T("save"),				//SAVE,
	_T("load"),				//LOAD
};

const TCHAR *InputDevice::GetButtonName(VirtualButton b)
{
	return ButtonName[b];
}

bool InputDevice::GetButtonCodeByName(const TCHAR *name, InputDevice::VirtualButton &ret)
{
	for (size_t i = 0; i < InputDevice::NUMBUTTONS; i++)
	{
		if (!_tcsicmp(ButtonName[i], name))
		{
			ret = (VirtualButton)i;
			return true;
		}
	}

	return false;
}
