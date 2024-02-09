// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3ScreenManager.h>
#include <PowerProps.h>

namespace c3
{

	// The ScreenManager maintains a stack of Screens - program state objects that typically correspond to model user interfaces,
	// whether that is your main menu, primary game loop, pause menu, etc.

	class ScreenManagerImpl : public ScreenManager
	{

	protected:

		System *m_pSys;

		using TScreenRegistry = std::deque<Screen *>;
		TScreenRegistry m_ScreenReg;

		using TScreenStack = std::deque<std::pair<Screen *, TScreenFlags>>;
		TScreenStack m_ScreenStack;

	public:

		ScreenManagerImpl(System *psys);

		virtual ~ScreenManagerImpl();

		/// Adds your screen to the ScreenManager's registry
		virtual bool RegisterScreen(const Screen *pscreen);

		/// Removes your Screen from the ScreenManager's registry
		virtual bool UnregisterScreen(const Screen *pscreen);

		/// Returns the number of registered Screens
		virtual size_t RegisteredScreenCount();

		/// Gets the Screen at the given index
		virtual Screen *GetRegisteredScreen(size_t idx);

		/// Updates the screens by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f);

		/// Called to render the screen stack; returns true if Postrender should be called, false if not
		virtual void Render();

		/// Pushes a screen type that was registered
		virtual bool PushScreen(const TCHAR *screen_name, TScreenFlags flags = 0);

		///  Pops the top-level screen
		virtual bool PopScreen();

	};

};