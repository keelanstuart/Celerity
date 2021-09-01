
// Celedit3.h : main header file for the Celedit3 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CCeledit3App:
// See Celedit3.cpp for the implementation of this class
//

class CCeledit3App : public CWinAppEx
{
public:
	CCeledit3App() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	c3::System *m_C3;


	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCeledit3App theApp;
