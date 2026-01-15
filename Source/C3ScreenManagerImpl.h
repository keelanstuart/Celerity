// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3ScreenManager.h>
#include <PowerProps.h>
#include <C3Utility.h>

namespace c3
{

	// The ScreenManager maintains a stack of Screens - program state objects that typically correspond to model user interfaces,
	// whether that is your main menu, primary game loop, pause menu, etc.

	class ScreenManagerImpl : public ScreenManager
	{

	protected:

		System *m_pSys;

		using TScreenStack = std::deque<std::pair<Object *, TScreenFlags>>;
		TScreenStack m_ScreenStack;

		TObjectArray m_Cleanup;

		size_t m_StartUpdateIndex;
		size_t m_StartRenderIndex;

		void ComputeStartIndices();

	public:

		ScreenManagerImpl(System *psys);

		virtual ~ScreenManagerImpl();

		// Updates the screens by the given amount of elapsed time (in seconds)
		virtual void Update(float elapsed_time = 0.0f);

		// Called to render the screen stack; returns true if Postrender should be called, false if not
		virtual void Render(ScreenFunc render_func);

		// Pushes a screen type that was registered
		virtual bool PushScreen(const TCHAR *screen_name, const TCHAR *script_filename = nullptr, TScreenFlags flags = 0);

		// Pops the top-level screen
		virtual bool PopScreen();

		// Gets the Screen Object at the top of the Screen Stack - optionally, go down the stack with offset
		virtual Object *GetActiveScreen(size_t offset = 0) const;

	};

};