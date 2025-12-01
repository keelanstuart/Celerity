// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


// SpaceMouse (3Dconnexion) raw HID input device
//
// Depends on: setupapi.lib, hid.lib
// build: add /DUNICODE and link setupapi.lib hid.lib

#pragma once

#include <C3.h>
#include <C3InputDeviceImpl.h>

#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidclass.h>
#include <atomic>
#include <thread>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

namespace c3
{
	class SpaceMouseImpl : public InputDeviceImpl
	{

	public:
		SpaceMouseImpl(System *sys);
		virtual ~SpaceMouseImpl();

		// Polls latest cached HID state -> updates m_ButtonState / m_ButtonDelta
		virtual bool Update(float elapsed_seconds = 0.0f) override;

		virtual DeviceType GetType() const override
		{
			return InputDevice::DeviceType::JOYSTICK;
		}

		virtual size_t GetNumAxes() const override
		{
			return 6;
		}

		virtual size_t GetNumButtons() const override
		{
			return 12;
		}

		// Tweak deadzone at runtime, [-127..127]
		void SetDeadzone(int dz);

	private:

		// HID plumbing
		HANDLE m_hDevice = INVALID_HANDLE_VALUE;
		HANDLE m_hStopEvent = nullptr;
		HANDLE m_hHaveReportEvent = nullptr; // signaled when a new report is parsed
		HANDLE m_hThread = nullptr;

		OVERLAPPED m_ov{};
		PHIDP_PREPARSED_DATA m_pp = nullptr;
		HIDP_CAPS m_caps{};
		std::vector<HIDP_VALUE_CAPS> m_valCaps;
		std::vector<HIDP_BUTTON_CAPS> m_btnCaps;
		std::vector<BYTE> m_report;          // input report buffer

		bool OpenSpaceMouse();
		void CloseSpaceMouse();
		static DWORD WINAPI ReaderThreadThunk(LPVOID p);
		DWORD ReaderThread();

		// Shared “latest state” from reader thread
		struct HidState
		{
			// normalized to [-1..1]
			float tx = 0, ty = 0, tz = 0;    // X,Y,Z
			float rx = 0, ry = 0, rz = 0;    // Rx,Ry,Rz
			uint32_t buttons = 0;            // bitmask (first 32)
		};
		std::atomic<HidState> m_latest{ HidState{} };

		// Mapping / helpers
		int m_deadzone = InputDevice::BUTTONVAL_MAX / 16; // match VirtualJoystickImpl default feel

		void ApplyStateToButtons(const HidState &s);
	};

};
