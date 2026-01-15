// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3InputDevice.h>


namespace c3
{

	class C3_API InputManager
	{

	public:
		enum { USER_ANY = -1, USER_DEFAULT };
		using UserID = size_t;

		enum { CURSOR_INVALID = -1 };
		using CursorID = int;

		typedef bool (__cdecl *DEVICECONNECTION_CALLBACK_FUNC)(InputDevice *device, bool conn, void *userdata);

		// Sets the function that will be called when a device is connected or disconnected
		static void SetDeviceConnectionCallback(DEVICECONNECTION_CALLBACK_FUNC func, void *userdata);

		// Returns the number of input devices that have been identified
		virtual size_t GetNumDevices() const = NULL;

		// Gets a Device by index
		virtual InputDevice *GetDevice(size_t idx) const = NULL;

		// Finds a device by name. If found, returns true and sets idx, otherwise returns false
		virtual bool FindDevice(const TCHAR *name, size_t &idx) const = NULL;

		// The User paradigm can be useful for local multiplayer where 3 physical devices (mouse / keyboard pair and a joypad)
		// supply input data for 2 players. AssignUser allows you to set that
		virtual void AssignUser(const InputDevice *pdevice, UserID user) = NULL;

		// Returns the user for the given Device
		virtual bool GetAssignedUser(const InputDevice *pdevice, UserID &user) const = NULL;

		// Returns the number of individual users
		virtual size_t GetNumUsers() const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY, float time = 0.0f) const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const = NULL;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const = NULL;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button = InputDevice::VirtualButton::ANY, UserID user = USER_ANY) const = NULL;

		// Sets the mouse position (primarily for UI) in desktop space
		virtual void SetMousePos(int32_t x, int32_t y) = NULL;

		// Gets the mouse position (primarily for UI) in desktop space
		virtual void GetMousePos(int32_t &x, int32_t &y) const = NULL;

		// Enables or disables mouse cursor visibility
		virtual void EnableMouse(bool enabled = true) = NULL;

		// Gets the current mouse mode
		virtual bool MouseEnabled() const = NULL;

		// Sets the capture state of the mouse in the Celerity context window
		virtual bool CaptureMouse(bool capture = true) = NULL;

		// Tells you if the mouse is currently captured
		virtual bool MouseCaptured() const = NULL;

		// Registers a Texture as mouse cursor with an offset,
		// allows you to call SetMouseCursor to draw it at the current mouse coordinates when visible
		// the default hotspot is (0,0); the upper-left hand corner
		virtual CursorID RegisterCursor(const Texture2D *ptex, std::optional<glm::ivec2> hotspot, const TCHAR *name) = NULL;

		// Registers a mouse cursor from a filename that it loads as a Texture - with an offset,
		// allows you to call SetMouseCursor to draw it at the current mouse coordinates when visible
		// the default hotspot is (0,0); the upper-left hand corner
		virtual CursorID RegisterCursor(const TCHAR *filename, std::optional<glm::ivec2> hotspot, const TCHAR *name) = NULL;

		// Unregisters a mouse cursor; you can no longer call SetMouseCursor with this ID
		virtual void UnregisterCursor(CursorID cursor_id) = NULL;

		// Returns the number of registered mouse cursors
		virtual size_t GetNumCursors() const = NULL;

		// Returns the name of the mouse cursor with the given id
		virtual const TCHAR *GetCursorName(CursorID id) const = NULL;

		// Sets the current mouse cursor to the registered cursor with the given ID
		virtual bool SetCursor(CursorID cursor_id) = NULL;

		// Gets the currently active mouse cursor
		virtual CursorID GetCursor() const = NULL;

		// Transforms the mouse cursor about it's hotspot position
		// if no mat is supplied, the transform is reset to identity
		virtual void SetCursorTransform(std::optional<glm::fmat4x4> mat) = NULL;

		// Draws the mouse cursor
		virtual void DrawMouseCursor(Renderer *prend) = NULL;

		// Sets the global pick ray origin (in world space) and direction (normalized) - used by Interactable
		virtual void SetPickRay(const glm::fvec3 &pos, const glm::fvec3 &dir, UserID user = USER_DEFAULT) = NULL;

		// Gets the global pick ray origin (in world space) and direction (normalized) - used by Interactable
		virtual bool GetPickRay(glm::fvec3 &pos, glm::fvec3 &dir, UserID user = USER_DEFAULT) = NULL;

		// Acquires all connected Devices; this may be necessary when restoring focus to your application
		virtual void AcquireAll() = NULL;

		// Unacquires all connected devices; be polite and let others use exclusive devices when your application loses focus
		virtual void UnacquireAll() = NULL;

		// Removes and re-enumerates all devices - this should be handled when a WM_DEVICECHANGE message has been received
		// If you are directly getting devices, rather than using the input manager, you will need to re-get your devices after this
		virtual void Reset() = NULL;
	};

};