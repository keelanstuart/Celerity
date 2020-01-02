
// C3App.h : main header file for the PROJECT_NAME application
//

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
