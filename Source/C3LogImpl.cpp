// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3LogImpl.h>
#include <C3SystemImpl.h>


using namespace c3;


LogImpl::LogImpl(SystemImpl *psys)
{
	m_pSys = psys;

	m_File = nullptr;
	m_pUserData = nullptr;
	m_pcbRedirect = nullptr;
}


LogImpl::~LogImpl()
{
	if (m_File)
	{
		fflush(m_File);
		fclose(m_File);
		m_File = nullptr;
	}
}


System *LogImpl::GetSystem()
{
	return m_pSys;
}


bool LogImpl::SetLogFile(const TCHAR *filename)
{
	if (m_File)
	{
		fflush(m_File);
		fclose(m_File);
		m_File = NULL;
	}

	if (filename)
	{
		m_Filename = filename;
		_tfopen_s(&m_File, filename, _T("at"));
	}

	if (!m_File)
		return true;

	return false;
}


bool LogImpl::SetRedirectFunction(REDIRECT_FUNCTION pcb, void *userdata)
{
	m_pcbRedirect = pcb;
	m_pUserData = userdata;

	return true;
}


void LogImpl::Reset()
{
	if (m_File)
	{
		fflush(m_File);
		fclose(m_File);

		m_File = NULL;

		DeleteFile(m_Filename.c_str());

		_tfopen_s(&m_File, m_Filename.c_str(), _T("wt"));
	}
}


void LogImpl::Print(const TCHAR *format, ...)
{
#define PRINT_BUFSIZE	1024
	TCHAR buf[PRINT_BUFSIZE];	// Temporary buffer for output

	va_list marker;
	va_start(marker, format);
	_vsntprintf_s(buf, PRINT_BUFSIZE - 1, format, marker);

	// this writes the message to the log file...
	if (m_File)
	{
		_ftprintf(m_File, buf);
		fflush(m_File);
	}

	if (m_pcbRedirect)
		m_pcbRedirect(m_pUserData, buf);

#if defined(UNICODE)
	_RPTW0(_CRT_WARN, buf);
#else
	_RPT0(_CRT_WARN, buf);
#endif
}
