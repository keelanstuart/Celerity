// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart

// SpaceMouse (3Dconnexion) raw HID input device


#include "pch.h"
#include <C3SpaceMouseImpl.h>

using namespace c3;


static inline void SafeClose(HANDLE &h)
{
	if (h && h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
	}
}

SpaceMouseImpl::SpaceMouseImpl(System *sys) : InputDeviceImpl(sys, nullptr)
{
	ZeroMemory(&m_ov, sizeof(m_ov));
	m_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_hHaveReportEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	m_Name = _T("SpaceMouse");

	// Try opening the HID device immediately; if not present, we'll mark detached
	if (OpenSpaceMouse())
	{
		m_bAttached = true;

		// Kick off the background reader
		m_hThread = CreateThread(nullptr, 0, &SpaceMouseImpl::ReaderThreadThunk, this, 0, nullptr);
	}
	else
	{
		m_bAttached = false;
	}
}

SpaceMouseImpl::~SpaceMouseImpl()
{
	if (m_hStopEvent)
		SetEvent(m_hStopEvent);

	if (m_hThread)
		WaitForSingleObject(m_hThread, 2000);

	CloseSpaceMouse();

	SafeClose(m_hThread);
	SafeClose(m_hStopEvent);
	SafeClose(m_hHaveReportEvent);
}

void SpaceMouseImpl::SetDeadzone(int dz)
{
	m_deadzone = std::max<int8_t>(InputDevice::BUTTONVAL_MIN, std::min<int8_t>(dz, InputDevice::BUTTONVAL_MAX));
}

bool SpaceMouseImpl::Update(float elapsed_seconds)
{
	// Take the latest HID state and map into your virtual buttons, like VirtualJoystickImpl
	ApplyStateToButtons(m_latest.load(std::memory_order_relaxed));

	// Let the base handle any book-keeping it normally does
	__super::Update(elapsed_seconds);
	return true;
}

// ------------------ HID: open/close/enumeration ------------------

bool SpaceMouseImpl::OpenSpaceMouse()
{
	GUID hidGuid; HidD_GetHidGuid(&hidGuid);
	HDEVINFO devs = SetupDiGetClassDevs(&hidGuid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (devs == INVALID_HANDLE_VALUE)
		return false;

	SP_DEVICE_INTERFACE_DATA ifData{ sizeof(SP_DEVICE_INTERFACE_DATA) };
	DWORD index = 0;
	bool found = false;

	while (SetupDiEnumDeviceInterfaces(devs, nullptr, &hidGuid, index++, &ifData))
	{
		DWORD needed = 0;

		SetupDiGetDeviceInterfaceDetail(devs, &ifData, nullptr, 0, &needed, nullptr);

		std::vector<BYTE> buf(needed);
		auto detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf.data();
		detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(devs, &ifData, detail, needed, nullptr, nullptr))
			continue;

		HANDLE h = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED, nullptr);

		if (h == INVALID_HANDLE_VALUE)
			continue;

		// Check this is a Multi-axis Controller (Usage Page 0x01, Usage 0x08)
		PHIDP_PREPARSED_DATA pp = nullptr;
		HIDP_CAPS caps{};
		if (HidD_GetPreparsedData(h, &pp))
		{
			if ((HidP_GetCaps(pp, &caps) == HIDP_STATUS_SUCCESS) &&
				(caps.UsagePage == HID_USAGE_PAGE_GENERIC) &&
				(caps.Usage == 0x08) /* Multi-axis Controller */)
			{
				m_hDevice = h;
				m_pp = pp;
				m_caps = caps;

				found = true;
				break;
			}
			HidD_FreePreparsedData(pp);
		}
		CloseHandle(h);
	}

	SetupDiDestroyDeviceInfoList(devs);

	if (!found)
		return false;

	// Prepare caps
	USHORT nVals = m_caps.NumberInputValueCaps;
	USHORT nBtns = m_caps.NumberInputButtonCaps;

	m_valCaps.resize(nVals);
	if (nVals)
		HidP_GetValueCaps(HidP_Input, m_valCaps.data(), &nVals, m_pp);

	m_btnCaps.resize(nBtns);
	if (nBtns)
		HidP_GetButtonCaps(HidP_Input, m_btnCaps.data(), &nBtns, m_pp);

	// Report buffer & overlapped event
	m_report.resize(m_caps.InputReportByteLength);
	m_ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	// Mark attached
	m_bAttached = (m_hDevice != INVALID_HANDLE_VALUE);
	return m_bAttached;
}

void SpaceMouseImpl::CloseSpaceMouse()
{
	if (m_pp)
	{
		HidD_FreePreparsedData(m_pp);
		m_pp = nullptr;
	}

	SafeClose(m_ov.hEvent);
	SafeClose(m_hDevice);
}

// ------------------ Reader thread ------------------

DWORD WINAPI SpaceMouseImpl::ReaderThreadThunk(LPVOID p)
{
	return static_cast<SpaceMouseImpl *>(p)->ReaderThread();
}

DWORD SpaceMouseImpl::ReaderThread()
{
	// Usages we care about
	static const USAGE wanted[6] =
	{
		HID_USAGE_GENERIC_X, HID_USAGE_GENERIC_Y, HID_USAGE_GENERIC_Z,
		HID_USAGE_GENERIC_RX, HID_USAGE_GENERIC_RY, HID_USAGE_GENERIC_RZ
	};

	HANDLE waits[2] = { m_hStopEvent, m_ov.hEvent };

	while (WaitForSingleObject(m_hStopEvent, 0) == WAIT_TIMEOUT)
	{
		// Kick off an async read if one isn't in flight
		ResetEvent(m_ov.hEvent);
		DWORD bytes = 0;
		BOOL ok = ReadFile(m_hDevice, m_report.data(), (DWORD)m_report.size(), &bytes, &m_ov);

		if (!ok && (GetLastError() != ERROR_IO_PENDING))
		{
			// Device likely yanked; mark detached and bail
			m_bAttached = false;
			break;
		}

		// Wait until either we're asked to stop or read completed
		DWORD w = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
		if (w == WAIT_OBJECT_0) { // stop
			CancelIo(m_hDevice);
			break;
		}

		if (!GetOverlappedResult(m_hDevice, &m_ov, &bytes, FALSE) || (bytes == 0))
			continue;

		// Parse values
		float axes[6] = { 0, 0, 0, 0, 0, 0 };
		int   ax_i = 0;

		// Track last seen logical limits (fall back to symmetric if not present)
		LONG lo = -350, hi = 350;

		for (const auto &vc : m_valCaps)
		{
			// Scan for our six usages
			for (USAGE u : wanted)
			{
				if ((vc.UsagePage == HID_USAGE_PAGE_GENERIC) &&
					((u > vc.Range.UsageMin) && (u < vc.Range.UsageMax)))
				{
					ULONG value = 0;
					if (HidP_GetUsageValue(HidP_Input, vc.UsagePage, vc.LinkUsage,
						u, &value, m_pp, (PCHAR)m_report.data(), (ULONG)m_report.size()) == HIDP_STATUS_SUCCESS)
					{
						lo = vc.LogicalMin;
						hi = vc.LogicalMax;
						if (ax_i < 6)
							axes[ax_i++] = Normalize((LONG)value, lo, hi);
					}
				}
			}
		}

		// Buttons
		uint32_t mask = 0;
		{
			USAGE pressed[32]; ULONG cnt = 32;
			if (!m_btnCaps.empty()) {
				cnt = 32;
				HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_BUTTON, 0,
					pressed, &cnt, m_pp, (PCHAR)m_report.data(), (ULONG)m_report.size());
				for (ULONG i = 0; i < cnt; ++i) {
					USAGE b = pressed[i];
					if (b >= 1 && b <= 32) mask |= (1u << (b - 1));
				}
			}
		}

		HidState s;
		s.tx = axes[0]; s.ty = axes[1]; s.tz = axes[2];
		s.rx = axes[3]; s.ry = axes[4]; s.rz = axes[5];
		s.buttons = mask;
		m_latest.store(s, std::memory_order_relaxed);

		// notify (optional)
		SetEvent(m_hHaveReportEvent);
	}

	return 0;
}

// ------------------ Helpers ------------------

float SpaceMouseImpl::Normalize(LONG v, LONG lo, LONG hi)
{
	if (hi == lo)
		return 0.0f;

	const double mid = (double)lo + (double)(hi - lo) / 2.0;

	return (float)((v - mid) / ((hi - lo) / 2.0)); // -> [-1..1]
}

static inline int DampenMagnitudeTo127(int rawAbs /*0..127*/)
{
	// match the feel from VirtualJoystickImpl

	float pct = (float)rawAbs / (float)c3::InputDevice::BUTTONVAL_MAX;
	pct *= pct;

	return (int)(pct * (float)c3::InputDevice::BUTTONVAL_MAX);
}

int SpaceMouseImpl::ToButtonRange(float n)
{
	// map [-1..1] -> 0..127 magnitude (sign handled by which virtual button we drive)
	float mag = std::min(1.0f, std::max(0.0f, std::fabs(n)));

	int v = (int)(mag * (float)InputDevice::BUTTONVAL_MAX + 0.5f);

	return DampenMagnitudeTo127(v);
}

void SpaceMouseImpl::ApplyStateToButtons(const HidState &s)
{
	// Zero deltas first pass
	ZeroMemory(m_ButtonDelta, sizeof(m_ButtonDelta));

	// Helper to set a “virtual button” magnitude and delta (copied pattern)
	auto SetButton = [&](size_t idx, int val)
	{
		if (val != m_ButtonState[idx])
			m_ButtonDelta[idx] = val - m_ButtonState[idx];

		m_ButtonState[idx] = val;
	};

	// Deadzone & min proportion (same semantics as VirtualJoystickImpl)
	const int iminprop = (int)(255.0f * m_MinProportion);

	auto HandleAxis = [&](float n, InputDevice::VirtualButton negBtn, InputDevice::VirtualButton posBtn)
	{
		const int val = ToButtonRange(n);
		const bool pos = (n > 0.0f);
		const bool neg = (n < 0.0f);
		if (val > std::max(m_deadzone, iminprop))
		{
			if (pos)
			{
				SetButton(posBtn, val);
				SetButton(negBtn, 0);
			}
			else if (neg)
			{
				SetButton(negBtn, val);
				SetButton(posBtn, 0);
			}
		}
		else
		{
			SetButton(negBtn, 0);
			SetButton(posBtn, 0);
		}
	};

	// Map translation -> AXIS1_*, rotation -> AXIS2_* (mirrors joystick layout)
	HandleAxis(s.ty, InputDevice::AXIS1_NEGY, InputDevice::AXIS1_POSY);
	HandleAxis(s.tx, InputDevice::AXIS1_NEGX, InputDevice::AXIS1_POSX);
	HandleAxis(s.tz, InputDevice::AXIS1_NEGZ, InputDevice::AXIS1_POSZ);

	HandleAxis(s.ry, InputDevice::AXIS2_NEGY, InputDevice::AXIS2_POSY);
	HandleAxis(s.rx, InputDevice::AXIS2_NEGX, InputDevice::AXIS2_POSX);
	HandleAxis(s.rz, InputDevice::AXIS2_NEGZ, InputDevice::AXIS2_POSZ);

	// Map first 12 physical buttons to your BUTTON1..BUTTON12
	for (size_t i = 0; i < 12; i++)
	{
		const bool pressed = (s.buttons & (1u << i)) != 0;
		SetButton((size_t)InputDevice::BUTTON1 + i, pressed ? InputDevice::BUTTONVAL_MAX : 0);
	}

	// No POV / sliders for SpaceMouse, so we leave them at 0
}
