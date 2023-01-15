// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	class InputDevice
	{

	public:

		// The maximum number of buttons that may be managed by a virtual joystick
		enum { MAXBUTTONS = 127 };

		// the maximum value that any button state may have
		enum { BUTTONVAL_MAX = 127 };

		enum { BUTTONVAL_MIN = -127 };

		// enumated indices into the button state/delta array
		typedef enum
		{
			DEBUGBUTTON = 0,

			HELP,

			// the start button
			START,

			// the select button
			SELECT,

			// the command to quit... or maybe go back a menu
			QUIT,

			// device directional control #1 axes (x, y, z axis support)
			AXIS1_NEGY,
			AXIS1_POSY,
			AXIS1_NEGX,
			AXIS1_POSX,
			AXIS1_NEGZ,
			AXIS1_POSZ,

			// device directional control #2 axes (x, y, z axis support)
			AXIS2_NEGY,
			AXIS2_POSY,
			AXIS2_NEGX,
			AXIS2_POSX,
			AXIS2_NEGZ,
			AXIS2_POSZ,

			// Twelve buttons currently supported
			BUTTON1,
			BUTTON2,
			BUTTON3,
			BUTTON4,
			BUTTON5,
			BUTTON6,
			BUTTON7,
			BUTTON8,
			BUTTON9,
			BUTTON10,
			BUTTON11,
			BUTTON12,

			// support for two throttles
			THROTTLE1,
			THROTTLE2,

			LSHIFT,
			RSHIFT,

			LCTRL,
			RCTRL,

			// characters (keyboard emulation)
			LETTER_A,
			LETTER_B,
			LETTER_C,
			LETTER_D,
			LETTER_E,
			LETTER_F,
			LETTER_G,
			LETTER_H,
			LETTER_I,
			LETTER_J,
			LETTER_K,
			LETTER_L,
			LETTER_M,
			LETTER_N,
			LETTER_O,
			LETTER_P,
			LETTER_Q,
			LETTER_R,
			LETTER_S,
			LETTER_T,
			LETTER_U,
			LETTER_V,
			LETTER_W,
			LETTER_X,
			LETTER_Y,
			LETTER_Z,

			TILDE,
			TAB,

			// Delete key
			DELETEKEY,

			// Backspace
			NAV_BACK,
			NAV_FORWARD,

			// numbers (keyboard emulation)
			NUM0,
			NUM1,
			NUM2,
			NUM3,
			NUM4,
			NUM5,
			NUM6,
			NUM7,
			NUM8,
			NUM9,

			SAVE,
			LOAD,

			NUMBUTTONS,

			ANY = NUMBUTTONS
		} VirtualButton;

		typedef enum 
		{
			NONE,

			JOYSTICK,
			MOUSE,
			KEYBOARD,

			UNKNOWN

		} DeviceType;

		// Returns the name of the device
		virtual const TCHAR *GetName() const = NULL;

		// Returns the device's UID - probably just the UID for a device type (i.e., all xbox 360 controllers would have the same GUID)
		virtual uint32_t GetUID() const = NULL;

		// gets the type of the device
		virtual DeviceType GetType() const = NULL;

		// gets the number of directional axes on the device
		virtual size_t GetNumAxes() const = NULL;

		// gets the number of buttons on the device
		virtual size_t GetNumButtons() const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual int ButtonPressed(InputDevice::VirtualButton button, float time = 0.0f) const = NULL;

		// Has the button been pressed?  A non-zero value is TRUE...
		virtual float ButtonPressedProportional(InputDevice::VirtualButton button) const = NULL;

		// What was the change in the button's state?
		virtual int ButtonChange(InputDevice::VirtualButton button) const = NULL;

		// Was the button just released?
		virtual bool ButtonReleased(InputDevice::VirtualButton button) const = NULL;

		virtual void SetProportionRange(float minprop, float maxprop) = NULL;

		//virtual void PlayForceFeedbackEffect(const TCHAR* fullpath, int32_t dir_offset = 0) = NULL;

		// Returns true if the device is currently plugged in
		virtual bool PluggedIn() const = NULL;

		// Sets the active state of the device; an active device has it's input states included, an inactive one deos not
		virtual void Activate(bool isactive = true) = NULL;

		// Returns true if the device is active and it's input states are being monitored
		virtual bool IsActive() const = NULL;

	};

};