// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	/// As with all versions of Celerity, there is a System container that creates and maintains all instances of
	/// various sub-components... everything from rendering, audio, and resource management to object creation, time, and logging, etc.

	/// To get started with Celerity, the first thing you do is call c3::System::Create()

	class C3_API System
	{

	public:
		static System *Create(HWND owner, props::TFlags64 flags);

		virtual void Release() = NULL;

		virtual void SetOwner(HWND owner) = NULL;

		virtual HWND GetOwner() const = NULL;

		virtual bool IsSplashWnd(HWND h) const = NULL;

		virtual pool::IThreadPool *GetThreadPool() = NULL;

		virtual PluginManager *GetPluginManager() = NULL;

		virtual ResourceManager *GetResourceManager() = NULL;

		virtual Renderer *GetRenderer() = NULL;

		virtual InputManager *GetInputManager() = NULL;

		virtual ActionMapper *GetActionMapper() = NULL;

		virtual Factory *GetFactory() = NULL;

		virtual FileMapper *GetFileMapper() = NULL;

		virtual Configuration *CreateConfiguration(const TCHAR *filename) = NULL;

		virtual Log *GetLog() = NULL;

		virtual GlobalObjectRegistry *GetGlobalObjectRegistry() = NULL;

		virtual float GetCurrentTime() = NULL;

		virtual float GetElapsedTime() = NULL;

		virtual void UpdateTime() = NULL;

	};

};