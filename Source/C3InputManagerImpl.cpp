// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"
#include <C3InputManagerImpl.h>
#include <C3VirtualKeyboardImpl.h>
#include <C3VirtualMouseImpl.h>
#include <C3VirtualJoystickImpl.h>
#include <C3ResourceManager.h>
#include <C3FileMapper.h>
#include <C3System.h>


using namespace c3;

struct SVDEnumJoyScratch
{
	LPDIRECTINPUT8 pdi;
	LPDIRECTINPUTDEVICE8 *pdid;
};


// ****************************************************************************************
// ****************************************************************************************
// ****************************************************************************************

InputManagerImpl::InputManagerImpl(System *sys)
{
	m_pSys = sys;
	m_pDI = NULL;

	m_PlugCheckTime = 0.0f;
}


InputManagerImpl::~InputManagerImpl()
{
	Shutdown();
}


bool InputManagerImpl::Initialize(HWND app_hwnd, HINSTANCE app_inst)
{
	m_hwnd = app_hwnd;
	m_hinst = app_inst;

	// Initialize DirectInput
	HRESULT hr = DirectInput8Create(m_hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDI, NULL);

	if (FAILED(hr))
		return false;

	LPDIRECTINPUTDEVICE8 pdid;

	if (SUCCEEDED(m_pDI->CreateDevice(GUID_SysKeyboard, &pdid, NULL)))
	{
		VirtualKeyboardImpl *pkeyboard = new VirtualKeyboardImpl(m_pSys, pdid);
		m_Devices.push_back(TDeviceArray::value_type(pkeyboard, USER_DEFAULT));
	}

	if (SUCCEEDED(m_pDI->CreateDevice(GUID_SysMouse, &pdid, NULL)))
	{
		VirtualMouseImpl *pmouse = new VirtualMouseImpl(m_pSys, pdid);
		m_Devices.push_back(TDeviceArray::value_type(pmouse, USER_DEFAULT));
	}

	return true;
}


void InputManagerImpl::Shutdown()
{
	for (auto pdev : m_Devices)
		delete pdev.first;

	m_Devices.clear();

	if (m_pDI)
	{
		m_pDI->Release();
		m_pDI = NULL;
	}
}


void InputManagerImpl::SetMousePos(int32_t x, int32_t y)
{
	m_MousePos.x = x;
	m_MousePos.y = y;
}


void InputManagerImpl::GetMousePos(int32_t& x, int32_t& y)
{
	x = m_MousePos.x;
	y = m_MousePos.y;
}


void InputManagerImpl::Update(float elapsed_seconds)
{
	if (!m_pDI)
		return;

	bool checkplugs = false;
	m_PlugCheckTime -= elapsed_seconds;
	if (m_PlugCheckTime <= 0.0f)
	{
		checkplugs = true;
		m_PlugCheckTime = 100.0f;
	}

	for (auto pdev : m_Devices)
	{
		pdev.first->Update(elapsed_seconds);
	}
}


bool CALLBACK InputManagerImpl::EnumJoysticksCallback(const DIDEVICEINSTANCE *did_instance, void *context)
{
	LPDIRECTINPUTDEVICE8 *did = ((SVDEnumJoyScratch *)context)->pdid;
	LPDIRECTINPUT8 di = ((SVDEnumJoyScratch *)context)->pdi;

	// See if the device is plugged in or not...
	HRESULT hr = di->GetDeviceStatus(did_instance->guidInstance);

	// if it's not attached, then it's not a valid device after all...
	if (hr == DI_NOTATTACHED)
        return DIENUM_CONTINUE;

    // Obtain an interface to the enumerated joystick.
    hr = di->CreateDevice(did_instance->guidInstance, did, NULL);

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if(FAILED(hr)) 
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}


void InputManagerImpl::AssignUser(const InputDevice *pdevice, size_t user)
{
	for (auto pds : m_Devices)
	{
		if (pdevice == pds.first)
		{
			pds.second = user;
			return;
		}
	}
}

bool InputManagerImpl::GetAssignedUser(const InputDevice *pdevice, size_t &user) const
{
	if (!pdevice)
		return false;

	for (const auto pds : m_Devices)
	{
		if (pdevice == pds.first)
		{
			user = pds.second;
			return true;
		}
	}

	return false;
}

size_t InputManagerImpl::GetNumUsers() const
{
	return m_NumUsers;
}

/*
c3::InputDevice *InputManagerImpl::AddDevice(int32_t user, uint32_t devicetype, const TCHAR *device_name)
{
	C2VirtualDevice *dev = NULL;

	LPDIRECTINPUTDEVICE8 pdid = NULL;

	switch(devicetype)
	{
		case VDEVICE_KEYBOARD:
			if (SUCCEEDED(m_pDI->CreateDevice(GUID_SysKeyboard, &pdid, NULL)))
				dev = new C2VirtualKeyboard(m_pSys, user);
			break;

		case VDEVICE_MOUSE:
			if (SUCCEEDED(m_pDI->CreateDevice(GUID_SysMouse, &pdid, NULL)))
				dev = new C2VirtualMouse(m_pSys, user);
			break;

		case VDEVICE_JOYSTICK:
		{
			SVDEnumJoyScratch scratch = {m_pDI, &pdid};

			HRESULT hr = m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &scratch, DIEDFL_ATTACHEDONLY);
			if (SUCCEEDED(hr) && pdid)
				dev = new VirtualJoystickImpl(m_pSys, user);
			break;
		}
	}

	// if the creation process was successful, then 
	if (dev)
	{
		devices.push_back(dev);

		dev->AssignDID(pdid);

		dev->Initialize(m_hwnd, m_hinst);
	}

	return dev;
}
*/


// Has the button been pressed?  A non-zero value is TRUE...
int InputManagerImpl::ButtonPressed(InputDevice::VirtualButton button, size_t user, float time) const
{
	int tmp;
	for (auto dev : m_Devices)
	{
		if ((user == USER_ANY) || (user == dev.second))
			if ((tmp = dev.first->ButtonPressed(button, time)) != 0)
				return tmp;
	}

	return 0;
}



// Has the button been pressed?  A non-zero value is TRUE...
float InputManagerImpl::ButtonPressedProportional(InputDevice::VirtualButton button, size_t user) const
{
	float tmp;
	for (auto dev : m_Devices)
	{
		if ((user == USER_ANY) || (user == dev.second))
			if ((tmp = dev.first->ButtonPressedProportional(button)) != 0.0f)
				return tmp;
	}

	return 0.0f;
}




// What was the change in the button's state?
int InputManagerImpl::ButtonChange(InputDevice::VirtualButton button, size_t user) const
{
	int tmp;
	for (auto dev : m_Devices)
	{
		if ((user == USER_ANY) || (user == dev.second))
			if ((tmp = dev.first->ButtonChange(button)) != 0)
				return tmp;
	}

	return 0;
}



// Was the button just released?
bool InputManagerImpl::ButtonReleased(InputDevice::VirtualButton button, size_t user) const
{
	for (auto dev : m_Devices)
	{
		if ((user == USER_ANY) || (user == dev.second))
			if (dev.first->ButtonReleased(button))
				return true;
	}

	return false;
}



void InputManagerImpl::AcquireAll()
{
	for (auto dev : m_Devices)
	{
		dev.first->Acquire();
	}
}



void InputManagerImpl::UnacquireAll()
{
	for (auto dev : m_Devices)
	{
		dev.first->Unacquire();
	}
}


void InputManagerImpl::PlayForceFeedbackEffect(const TCHAR *filename, int32_t dir_offset)
{
	TCHAR fullpath[MAX_PATH];
	if (m_pSys->GetFileMapper()->FindFile(filename, fullpath))
	{
		for (auto dev : m_Devices)
		{
			//dev.first->PlayForceFeedbackEffect(fullpath, dir_offset);
		}
	}
}


size_t InputManagerImpl::GetNumDevices()
{
	return m_Devices.size();
}



InputDevice *InputManagerImpl::GetDevice(size_t idx)
{
	if (idx < m_Devices.size())
	{
		return m_Devices[idx].first;
	}

	return nullptr;
}
