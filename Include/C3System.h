// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


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

		virtual void Release() = NULL;

		virtual Renderer *GetRenderer() = NULL;

		virtual Factory *GetFactory() = NULL;

		virtual pool::IThreadPool *GetThreadPool() = NULL;

		virtual Log *GetLog() = NULL;

	};

};