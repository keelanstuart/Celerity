// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// C3App:
// See C3App.cpp for the implementation of this class
//


class C3App : public CWinApp
{
public:
	c3::System *m_C3;
	c3::Configuration *m_Config;
	tstring m_AppDataRoot;
	tstring m_StartScript;

public:
	C3App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern C3App theApp;
