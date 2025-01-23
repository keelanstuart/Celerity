// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Screen.h>

namespace c3
{

	// The ScreenManager maintains a stack of Screens - program state objects that typically correspond to model user interfaces,
	// whether that is your main menu, primary game loop, pause menu, etc.

	class ScreenManager
	{

	public:

#define SCRFLAG_UPDATEOVER		0x00000001
#define SCRFLAG_DRAWOVER		0x00000002

		using TScreenFlags = props::TFlags32;

		/// Adds your screen to the ScreenManager's registry
		virtual bool RegisterScreen(const Screen *pscreen) = NULL;

		/// Removes your Screen from the ScreenManager's registry
		virtual bool UnregisterScreen(const Screen *pscreen) = NULL;

		/// Returns the number of registered Screens
		virtual size_t RegisteredScreenCount() = NULL;

		/// Gets the Screen at the given index
		virtual Screen *GetRegisteredScreen(size_t idx) = NULL;

		/// Updates the screens by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		/// Called to render the screen stack; returns true if Postrender should be called, false if not
		virtual void Render() = NULL;

		/// Pushes a screen type that was registered
		virtual bool PushScreen(const TCHAR *screen_name, TScreenFlags flags = 0) = NULL;

		///  Pops the top-level screen, returns false if no screen is left on the stack (and you should probably terminate)
		virtual bool PopScreen() = NULL;

	};

};