// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	class C3_API Scriptable : public Component
	{

	public:

		static const ComponentType *Type();

		virtual void Execute(const TCHAR *pcmd, ...) = NULL;

		virtual bool FunctionExists(const TCHAR *funcname) = NULL;

	};

};