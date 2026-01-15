// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputManager.h>
#include <C3InputDeviceImpl.h>
#include <deque>



namespace c3
{
	class InputManagerImpl : public InputManager
	{

	public:
		InputManagerImpl(System* sys);
		virtual ~InputManagerImpl();

		bool Initialize(HWND app_hwnd, HINSTANCE app_inst);

		void Shutdown();

		virtual void Update(float elapsed_seconds = 0.0f);

		virtual void SetMousePos(int32_t x, int32_t y);

		virtual void GetMousePos(int32_t &x, int32_t &y) const;

		virtual void EnableMouse(bool enabled = true);

		virtual bool MouseEnabled() const;

		virtual bool CaptureMouse(bool capture);

		virtual bool MouseCaptured() const;

		virtual CursorID RegisterCursor(const Texture2D *ptex, std::optional<glm::ivec2> hotspot, const TCHAR *name);

		virtual CursorID RegisterCursor(const TCHAR *filename, std::optional<glm::ivec2> hotspot, const TCHAR *name);

		virtual void UnregisterCursor(CursorID cursor_id);

		virtual size_t GetNumCursors() const;

		virtual const TCHAR *GetCursorName(CursorID id) const;

		virtual bool SetCursor(CursorID cursor_id);

		virtual CursorID GetCursor() const;

		virtual void SetCursorTransform(std::optional<glm::fmat4x4> mat);

		virtual void DrawMouseCursor(Renderer *prend);

		virtual void SetPickRay(const glm::fvec3 &pos, const glm::fvec3 &dir, UserID user = USER_DEFAULT);

		virtual bool GetPickRay(glm::fvec3 &pos, glm::fvec3 &dir, UserID user = USER_DEFAULT);

		virtual void AssignUser(const InputDevice *pdevice, UserID user);

		virtual bool GetAssignedUser(const InputDevice *pdevice, size_t &user) const;

		virtual size_t GetNumUsers() const;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY, float time = 0.0f) const;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const;

		virtual void AcquireAll();
		virtual void UnacquireAll();

		virtual void Reset();

		virtual size_t GetNumDevices() const;

		virtual InputDevice* GetDevice(size_t idx) const;

		virtual bool FindDevice(const TCHAR *name, size_t &idx) const;

		void PlayForceFeedbackEffect(const TCHAR* filename, int32_t dir_offset = 0);

	protected:
		System *m_pSys;

		// A list of devices and the user number they've been assigned
		typedef std::deque<std::pair<InputDeviceImpl *, size_t>> TDeviceArray;
		TDeviceArray m_Devices;
		size_t m_NumUsers;

		LPDIRECTINPUT8 m_pDI;

		static BOOL FAR PASCAL EnumJoysticksCallback(const DIDEVICEINSTANCE* did_instance, void* context);

		static bool FindFirstMultiAxisPath(tstring &out_path);	// SpaceMouse

		// We need these to determine our level of device cooperation and for DI to know who to deal with
		HWND m_hwnd;
		HINSTANCE m_hinst;

		float m_PlugCheckTime;

		using PickRayData = struct
		{
			glm::fvec3 pos, dir;
		};
		using UserPickRayDataMap = std::map<UserID, PickRayData>;
		UserPickRayDataMap m_PickData;

		using CursorInfo = struct
		{
			tstring name;
			const Texture2D *ptex;
			Material *pmtl;
			glm::ivec2 hotspot;
		};

		using CursorRegistry = std::map<CursorID, CursorInfo>;

		bool m_MouseEnabled;
		bool m_MouseCaptured;
		glm::ivec2 m_MousePos;
		glm::fmat4x4 m_MouseTransform;
		CursorRegistry m_Cursors;
		CursorRegistry::const_iterator m_MouseCursor;
		CursorID m_LastCursorID;
		RenderMethod *m_CursorRM;
		size_t m_CursorRMTech;

		const Object *m_PickingCamera;

	public:
		static DEVICECONNECTION_CALLBACK_FUNC s_DevConnCB;
		static void *s_DevConnUserData;
	};
};