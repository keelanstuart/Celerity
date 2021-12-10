
// C3Edit.h : main header file for the C3Edit application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// C3EditApp:
// See C3Edit.cpp for the implementation of this class
//

class C3EditApp : public CWinAppEx
{
public:

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

	C3EditApp() noexcept;

protected:
	CMultiDocTemplate* m_pDocTemplate;
public:


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	c3::System *m_C3;

	tstring m_AppDataRoot;

	c3::Configuration *m_Config;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern C3EditApp theApp;
