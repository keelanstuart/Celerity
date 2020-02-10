// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

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

	typedef enum
	{
		TT_SELECT = 0,
		TT_TRANSLATE,
		TT_ROTATE,
		TT_SCALE,
		TT_UNISCALE,
		TT_WAND
	} ToolType;

	typedef enum
	{
		AT_X = 0x1,
		AT_Y = 0x2,
		AT_Z = 0x4,

		AT_SCREENREL = 0x8
	} AxisType;

	typedef enum
	{
		CT_PERSPECTIVE_FREE = 0,
		CT_ORTHO_FREE,
		CT_TOP,
		CT_BOTTOM,
		CT_LEFT,
		CT_RIGHT,
		CT_FRONT,
		CT_BACK,

		CT_NUMCAMERATYPES
	} CameraType;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	c3::System *m_C3;

	c3::Configuration *m_Config;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCeledit3App theApp;
