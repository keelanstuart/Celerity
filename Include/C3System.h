// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class C3_API System
	{

	public:
		static System *Create(props::TFlags64 flags);

		virtual void Release() = NULL;

		virtual Renderer *GetRenderer() = NULL;

		virtual Log *GetLog() = NULL;

	};

};