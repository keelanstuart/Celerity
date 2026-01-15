// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{
	class System;

	class Log
	{

	public:

		using RedirectFunction = std::function<void(const TCHAR *)>;

		/// Returns the System used to create this Log
		virtual System *GetSystem() = NULL;

		/// Opens a text file in append mode
		virtual bool SetLogFile(const TCHAR *filename) = NULL;

		/// Use SetRedirectFunction to pipe your log output to a debug window, etc.
		virtual bool SetRedirectFunction(RedirectFunction rf) = NULL;

		/// Essentially closes an open output file, deletes it, then reopens it
		virtual void Reset() = NULL;

		/// Does a formatted print and sends the result to the log file, the redirect function, and _RPT0->_CRT_WARN
		virtual void Print(const TCHAR *format, ...) = NULL;

	};

};