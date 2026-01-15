// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


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

		using TScreenFlags = props::TFlags32;

		// Set this flag to allow screens not at the top of the stack to be updated
		// Note: not setting this flag at some level of the stack will stop any lower updates
		static constexpr uint32_t SCRFLAG_UPDATEOVER	= 0x00000001;

		// Set this flag to allow screens not at the top of the stack to be drawn
		// Note: not setting this flag at some level of the stack will stop any lower screen from drawing
		static constexpr uint32_t SCRFLAG_DRAWOVER		= 0x00000002;


		/// Updates the screens by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f) = NULL;

		using ScreenFunc = std::function<void(GlobalObjectRegistry *)>;

		/// Called to render the screen stack; returns true if Postrender should be called, false if not
		virtual void Render(ScreenFunc render_func) = NULL;

		/// Pushes a screen type that was registered
		virtual bool PushScreen(const TCHAR *screen_name, const TCHAR *script_filename = nullptr, TScreenFlags flags = 0) = NULL;

		///  Pops the top-level screen, returns false if no screen is left on the stack (and you should probably terminate)
		virtual bool PopScreen() = NULL;

		// Gets the Screen at the top of the stack - provide an offset to get Screens further down
		virtual Object *GetActiveScreen(size_t offset = 0) const = NULL;
	};

};