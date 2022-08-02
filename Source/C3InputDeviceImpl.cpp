// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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
	m_Name += _T(" (");
	m_Name += m_DIDInst.tszInstanceName;
	m_Name += _T(")");

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


const TCHAR *InputDeviceImpl::GetName() const
{
	return m_Name.c_str();
}


uint32_t InputDeviceImpl::GetUID() const
{
	return m_UID;
}


int InputDeviceImpl::ButtonPressed(InputDevice::VirtualButton button, float time) const
{
	if (button == ANY)
	{
		for (uint32_t i = 0; i < NUMBUTTONS; i++)
			if (m_ButtonState[i] && (m_ButtonTime[i] >= time))
				return TRUE;

		return FALSE;
	}

	if (m_ButtonState[button] && (m_ButtonTime[button] >= time))
		return m_ButtonState[button];

	return FALSE;
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
	for(size_t i = 0; i < NUMBUTTONS; i++)
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
