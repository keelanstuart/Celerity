// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include <C3InputManagerImpl.h>
#include <C3VirtualKeyboardImpl.h>
#include <C3VirtualMouseImpl.h>
#include <C3VirtualJoystickImpl.h>
#include <C3SpaceMouseImpl.h>
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

	m_MouseEnabled = false;
	m_MouseCaptured = false;
	m_MousePos = { 0, 0 };
	m_MouseTransform = glm::identity<glm::fmat4x4>();
	m_MouseCursor = m_Cursors.cend();
	m_LastCursorID = -1;
	m_PickingCamera = nullptr;

	Resource *pres = m_pSys->GetResourceManager()->GetResource(_T("ui.c3rm"), RESF_DEMANDLOAD);
	if (pres && (pres->GetStatus() == Resource::RS_LOADED))
	{
		m_CursorRM = dynamic_cast<RenderMethod *>((RenderMethod *)pres->GetData());
		if (m_CursorRM)
			m_CursorRM->FindTechnique(_T("cursor"), m_CursorRMTech);
	}
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


void InputManagerImpl::GetMousePos(int32_t& x, int32_t& y) const
{
	x = m_MousePos.x;
	y = m_MousePos.y;
}


void InputManagerImpl::EnableMouse(bool enabled)
{
	m_MouseEnabled = enabled;
}


bool InputManagerImpl::MouseEnabled() const
{
	return m_MouseEnabled;
}


bool InputManagerImpl::CaptureMouse(bool capture)
{
	if (capture != m_MouseCaptured)
	{
		if (capture)
		{
			if (GetCapture() == NULL)
				SetCapture(m_pSys->GetOwner());
			else
				capture = false;
		}
		else
		{
			if (GetCapture() == m_pSys->GetOwner())
				ReleaseCapture();
		}

		m_MouseCaptured = capture;
	}

	return m_MouseCaptured;
}


bool InputManagerImpl::MouseCaptured() const
{
	return m_MouseCaptured;
}


InputManager::CursorID InputManagerImpl::RegisterCursor(const Texture2D *ptex, std::optional<glm::ivec2> hotspot, const TCHAR *name)
{
	if (!ptex)
		return CURSOR_INVALID;

	// Get the current cursor before we modify the tree so we can restore it later
	CursorID cur = (m_MouseCursor != m_Cursors.cend()) ? (m_MouseCursor->first) : CURSOR_INVALID;

	CursorID ret = m_LastCursorID + 1;
	auto [it, inserted] = m_Cursors.emplace(ret, CursorInfo{});
	if (inserted)
	{
		it->second.ptex = ptex;
		it->second.pmtl = m_pSys->GetRenderer()->GetMaterialManager()->CreateMaterial();
		if (it->second.pmtl)
		{
			it->second.pmtl->SetTexture(Material::TCT_DIFFUSE, ptex);
			it->second.pmtl->SetCullMode(Renderer::CM_DISABLED);
		}

		if (name)
			it->second.name = name;
		else
			it->second.name = std::format(_T("Cursor{0}"), ret);

		if (hotspot.has_value())
			it->second.hotspot = *hotspot;
		else
			it->second.hotspot = { 0, 0 };
	}

	m_MouseCursor = m_Cursors.find(cur);

	return ret;
}


InputManager::CursorID InputManagerImpl::RegisterCursor(const TCHAR *filename, std::optional<glm::ivec2> hotspot, const TCHAR *name)
{
	Resource *pres = m_pSys->GetResourceManager()->GetResource(filename, RESF_DEMANDLOAD);
	if (pres && (pres->GetStatus() == Resource::RS_LOADED))
	{
		Texture2D *ptex = dynamic_cast<Texture2D *>((Texture2D *)pres->GetData());
		if (ptex)
			return RegisterCursor(ptex, hotspot, name);
	}

	return CURSOR_INVALID;
}


void InputManagerImpl::UnregisterCursor(InputManager::CursorID cursor_id)
{
	// Get the current cursor before we modify the tree so we can restore it later
	CursorID cur = (m_MouseCursor != m_Cursors.cend()) ? (m_MouseCursor->first) : -1;

	m_Cursors.erase(cursor_id);

	if (cur != cursor_id)
		m_MouseCursor = m_Cursors.find(cur);
	else
		m_MouseCursor = m_Cursors.cend();
}


size_t InputManagerImpl::GetNumCursors() const
{
	return m_Cursors.size();
}


const TCHAR *InputManagerImpl::GetCursorName(CursorID id) const
{
	CursorRegistry::const_iterator it = m_Cursors.find(id);

	return (it != m_Cursors.cend()) ? it->second.name.c_str() : nullptr;
}


bool InputManagerImpl::SetCursor(CursorID cursor_id)
{
	m_MouseCursor = m_Cursors.find(cursor_id);

	return (m_MouseCursor == m_Cursors.cend()) ? false : true;
}


InputManager::CursorID InputManagerImpl::GetCursor() const
{
	if (m_MouseCursor == m_Cursors.cend())
		return -1;

	return m_MouseCursor->first;
}


void InputManagerImpl::SetCursorTransform(std::optional<glm::fmat4x4> mat)
{
	m_MouseTransform = mat.has_value() ? *mat : glm::identity<glm::fmat4x4>();
}


void InputManagerImpl::DrawMouseCursor(Renderer *prend)
{
	if (m_MouseCursor == m_Cursors.cend())
		return;

	if (!m_MouseEnabled)
		return;

	RECT r;
	GetClientRect(m_pSys->GetOwner(), &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;

	const float sx = (float)((Texture2D *)(m_MouseCursor->second.ptex))->Width();
	const float sy = (float)((Texture2D *)(m_MouseCursor->second.ptex))->Height();

	glm::fvec3 mpos;

	// window (0,0 bottom-right) -> GUI (0,0 center, +Y up)
	mpos.x = (float)(w * 0.5f - m_MousePos.x) - (float)m_MouseCursor->second.hotspot.x;
	mpos.y = (float)(m_MousePos.y - h * 0.5f) + (float)m_MouseCursor->second.hotspot.y;
	mpos.z = 0.0f;

	glm::fmat4x4 mat = glm::translate(mpos) * (m_MouseTransform * glm::scale(glm::fvec3(sx, sy, 1.0f)));

	prend->UseMaterial(m_MouseCursor->second.pmtl);
	prend->UseRenderMethod(m_CursorRM, m_CursorRMTech);

	Resource *pres = m_pSys->GetResourceManager()->GetResource(_T("[guirect.model]"));
	Model *pmod = (Model *)pres->GetData();

	pmod->Draw(&mat, false);
}


void InputManagerImpl::SetPickRay(const glm::fvec3 &pos, const glm::fvec3 &dir, UserID user)
{
	if (user == USER_ANY)
		user = USER_DEFAULT;

	PickRayData prd;
	prd.pos = pos;
	prd.dir = dir;

	m_PickData.insert_or_assign(user, prd);
}


bool InputManagerImpl::GetPickRay(glm::fvec3 &pos, glm::fvec3 &dir, UserID user)
{
	UserPickRayDataMap::const_iterator it;
	if (user == USER_ANY)
	{
		it = m_PickData.cbegin();
	}
	else
	{
		it = m_PickData.find(user);
	}

	if (it != m_PickData.cend())
	{
		pos = it->second.pos;
		dir = it->second.dir;
		return true;
	}

	return false;
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

bool InputManagerImpl::FindFirstMultiAxisPath(tstring &out_path)
{
	GUID hidGuid; HidD_GetHidGuid(&hidGuid);
	HDEVINFO devs = SetupDiGetClassDevs(&hidGuid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (devs == INVALID_HANDLE_VALUE)
		return false;

	SP_DEVICE_INTERFACE_DATA ifdata{ sizeof(SP_DEVICE_INTERFACE_DATA) };
	DWORD index = 0;
	bool found = false;

	while (SetupDiEnumDeviceInterfaces(devs, nullptr, &hidGuid, index++, &ifdata))
	{
		DWORD needed = 0;
		SetupDiGetDeviceInterfaceDetail(devs, &ifdata, nullptr, 0, &needed, nullptr);

		std::vector<BYTE> buf(needed);

		auto detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf.data();
		detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(devs, &ifdata, detail, needed, nullptr, nullptr))
			continue;

		HANDLE h = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);

		if (h == INVALID_HANDLE_VALUE)
			continue;

		PHIDP_PREPARSED_DATA pp = nullptr;
		HIDP_CAPS caps{};

		if (HidD_GetPreparsedData(h, &pp))
		{
			if ((HidP_GetCaps(pp, &caps) == HIDP_STATUS_SUCCESS) &&
				(caps.UsagePage == HID_USAGE_PAGE_GENERIC) &&
				(caps.Usage == 0x08) /* Multi-axis Controller */)
			{
				out_path = detail->DevicePath;
				found = true;
				HidD_FreePreparsedData(pp);
				CloseHandle(h);
				break;
			}
			HidD_FreePreparsedData(pp);
		}

		CloseHandle(h);
	}

	SetupDiDestroyDeviceInfoList(devs);

	return found;
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

#if 0
	// WHY?
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
#endif

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

	tstring out_path;
	if (FindFirstMultiAxisPath(out_path))
	{
		SpaceMouseImpl *pspacemouse = new SpaceMouseImpl(m_pSys);
		m_Devices.push_back(TDeviceArray::value_type(pspacemouse, USER_DEFAULT));
		if (s_DevConnCB)
			s_DevConnCB(pspacemouse, true, s_DevConnUserData);
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


size_t InputManagerImpl::GetNumDevices() const
{
	return m_Devices.size();
}



InputDevice *InputManagerImpl::GetDevice(size_t idx) const
{
	if (idx < m_Devices.size())
	{
		return m_Devices[idx].first;
	}

	return nullptr;
}


bool InputManagerImpl::FindDevice(const TCHAR *name, size_t &idx) const
{
	for (size_t i = 0, maxi = m_Devices.size(); i < maxi; i++)
	{
		if (!_tcsicmp(m_Devices[i].first->GetName(), name))
		{
			idx = i;
			return true;
		}
	}

	return false;
}
