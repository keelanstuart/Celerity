// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3Log.h>

namespace c3
{

	class SystemImpl;

	class LogImpl : public Log
	{

	protected:
		SystemImpl *m_pSys;

		FILE *m_File;
		tstring m_Filename;

		REDIRECT_FUNCTION m_pcbRedirect;
		void *m_pUserData;

	public:

		LogImpl(SystemImpl *psys);
		virtual ~LogImpl();

		virtual System *GetSystem();

		virtual bool SetLogFile(const TCHAR *filename);
		virtual bool SetRedirectFunction(REDIRECT_FUNCTION pcb, void *userdata);

		virtual void Reset();
		virtual void Print(const TCHAR *format, ...);

	};

};