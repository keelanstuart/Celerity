// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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
		static System *Create(props::TFlags64 flags);

		virtual void SetOwner(HWND owner) = NULL;

		virtual void Release() = NULL;

		virtual ResourceManager *GetResourceManager() = NULL;

		virtual Renderer *GetRenderer() = NULL;

		virtual Factory *GetFactory() = NULL;

		virtual PluginManager *GetPluginManager() = NULL;

		virtual pool::IThreadPool *GetThreadPool() = NULL;

		virtual FileMapper *GetFileMapper() = NULL;

		virtual Configuration *CreateConfiguration(const TCHAR *filename) = NULL;

		virtual Log *GetLog() = NULL;

		virtual void SetMousePos(int32_t x, int32_t y) = NULL;

		virtual void GetMousePos(int32_t &x, int32_t &y) = NULL;

		virtual size_t GetCurrentFrameNumber() = NULL;

		virtual void SetCurrentFrameNumber(size_t framenum) = NULL;

		virtual float GetCurrentTime() = NULL;

		virtual float GetElapsedTime() = NULL;

		virtual void UpdateTime() = NULL;

	};

};