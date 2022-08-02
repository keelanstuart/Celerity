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

#include <dbt.h>


using namespace c3;

struct SVDEnumJoyScratch
{
	LPDIRECTINPUT8 pdi;
	LPDIRECTINPUTDEVICE8 *pdid;
	InputManagerImpl *_this;
};


// ****************************************************************************************
// ****************************************************************************************
// ****************************************************************************************

InputManagerImpl::InputManagerImpl(System *sys)
{
	m_pSys = sys;
	m_pDI = nullptr;

	m_PlugCheckTime = 0.0f;
}


InputManagerImpl::~InputManagerImpl()
{
	Shutdown();
}

static size_t devcount = 1;

BOOL FAR PASCAL InputManagerImpl::EnumJoysticksCallback(const DIDEVICEINSTANCE *did_instance, void *context)
{
	LPDIRECTINPUTDEVICE8 *did = ((SVDEnumJoyScratch *)context)->pdid;
	LPDIRECTINPUT8 di = ((SVDEnumJoyScratch *)context)->pdi;
	InputManagerImpl *_this = ((SVDEnumJoyScratch *)context)->_this;

	// See if the device is plugged in or not...
	HRESULT hr = di->GetDeviceStatus(did_instance->guidInstance);

	// if it's not attached, then it's not a valid device after all...
	if (hr == DI_NOTATTACHED)
		return DIENUM_CONTINUE;

	// don't re-add devices we already know about
#if 0
	for (auto d : _this->m_Devices)
	{
		InputDeviceImpl *pd = d.first;
		if (pd->IsDIDevice(*did))
			return DIENUM_CONTINUE;
	}
#endif

	// Obtain an interface to the enumerated joystick.
	hr = di->CreateDevice(did_instance->guidInstance, did, NULL);

	if (SUCCEEDED(hr))
	{
		VirtualJoystickImpl *pjoy = new VirtualJoystickImpl(_this->m_pSys, *did);
		if (pjoy)
		{
			_this->m_Devices.push_back(TDeviceArray::value_type(pjoy, devcount++));
			if (s_DevConnCB)
				s_DevConnCB(pjoy, true, s_DevConnUserData);
		}
	}

	return DIENUM_CONTINUE;
}


bool InputManagerImpl::Initialize(HWND app_hwnd, HINSTANCE app_inst)
{
	m_hwnd = app_hwnd;
	m_hinst = app_inst;

	// Initialize DirectInput
	HRESULT hr = DirectInput8Create(m_hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDI, NULL);

	if (FAILED(hr))
		return false;

	Reset();

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

InputManager::DEVICECONNECTION_CALLBACK_FUNC InputManagerImpl::s_DevConnCB = nullptr;
void *InputManagerImpl::s_DevConnUserData = nullptr;

void InputManager::SetDeviceConnectionCallback(InputManager::DEVICECONNECTION_CALLBACK_FUNC func, void *userdata)
{
	InputManagerImpl::s_DevConnCB = func;
	InputManagerImpl::s_DevConnUserData = userdata;
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

#if 0
	for (auto pdev : m_Devices)
	{
		if (!pdev.first->Update(elapsed_seconds))
		{
			if (s_DevConnCB)
				s_DevConnCB(pdev.first, false, s_DevConnUserData);
		}
	}

	for (TDeviceArray::iterator it = m_Devices.begin(); it != m_Devices.end(); it++)
	{
		if (!it->first)
		{
			m_Devices.erase(it);
			it = m_Devices.begin();
		}
	}
#else
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
#endif
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

void InputManagerImpl::Reset()
{
	for (auto pdev : m_Devices)
	{
		if (s_DevConnCB)
			s_DevConnCB(pdev.first, false, s_DevConnUserData);

		delete pdev.first;
	}

	m_Devices.clear();

	InputDevice *pdev_keyboard = nullptr;
	InputDevice *pdev_mouse = nullptr;

	for (auto pdev : m_Devices)
	{
		switch (pdev.first->GetType())
		{
			case InputDevice::DeviceType::KEYBOARD:
				pdev_keyboard = pdev.first;
				break;

			case InputDevice::DeviceType::MOUSE:
				pdev_mouse = pdev.first;
				break;
		}
	}

	LPDIRECTINPUTDEVICE8 pdid;

	if (!pdev_keyboard && SUCCEEDED(m_pDI->CreateDevice(GUID_SysKeyboard, &pdid, NULL)))
	{
		VirtualKeyboardImpl *pkeyboard = new VirtualKeyboardImpl(m_pSys, pdid);
		if (pkeyboard)
		{
			m_Devices.push_back(TDeviceArray::value_type(pkeyboard, USER_DEFAULT));
			if (s_DevConnCB)
				s_DevConnCB(pkeyboard, true, s_DevConnUserData);
		}
	}

	if (!pdev_mouse && SUCCEEDED(m_pDI->CreateDevice(GUID_SysMouse, &pdid, NULL)))
	{
		VirtualMouseImpl *pmouse = new VirtualMouseImpl(m_pSys, pdid);
		if (pmouse)
		{
			m_Devices.push_back(TDeviceArray::value_type(pmouse, USER_DEFAULT));
			if (s_DevConnCB)
				s_DevConnCB(pmouse, true, s_DevConnUserData);
		}
	}

	SVDEnumJoyScratch scratch = {m_pDI, &pdid, this};

	HRESULT hr;
	devcount = 1;
	hr = m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &scratch, DIEDFL_ATTACHEDONLY);
	//hr = m_pDI->EnumDevices(DI8DEVCLASS_DEVICE, EnumJoysticksCallback, &scratch, DIEDFL_ATTACHEDONLY);
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
