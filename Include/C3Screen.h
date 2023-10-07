// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3Screen.h>
#include <PowerProps.h>

namespace c3
{

	// Implement the Screen interface and register an instance of it with the ScreenManager to do whatever you want in modal fashion

	class Screen
	{

	public:

		virtual const TCHAR *GetName() const = NULL;

		/// Initialize your Screen
		virtual bool Initialize() = NULL;

		/// Called to update the state of your Screen with the number of seconds elapsed since the last update
		virtual void Update(float elapsed_seconds = 0.0f) = NULL;

		/// Called when your Screen should draw itself
		virtual void Render() = NULL;

	};

};
