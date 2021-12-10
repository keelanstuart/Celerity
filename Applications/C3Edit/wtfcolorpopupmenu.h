// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#pragma once

#include <afxpopupmenu.h>
#include "wtfcolorbar.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CPane;
class CMFCPropertyGridCtrl;
class CMFCRibbonColorButton;

/*============================================================================*/
// CWTFColorPopupMenu window

class CWTFColorPopupMenu : public CMFCPopupMenu
{
	friend class CWTFColorButton;
	friend class CWTFPropertyGridColorProperty;

	DECLARE_DYNAMIC(CWTFColorPopupMenu)

// Construction
public:
	CWTFColorPopupMenu(const CArray<COLORREF, COLORREF>& colors, COLORREF color, LPCTSTR lpszAutoColor, LPCTSTR lpszOtherColor, LPCTSTR lpszDocColors, CList<COLORREF,COLORREF>& lstDocColors,
		int nColumns, int nHorzDockRows, int nVertDockColumns, COLORREF colorAutomatic, UINT uiCommandID, BOOL bStdColorDlg = FALSE) :
	m_wndColorBar(colors, color, lpszAutoColor, lpszOtherColor, lpszDocColors, lstDocColors, nColumns, nHorzDockRows, nVertDockColumns, colorAutomatic, uiCommandID, NULL)
	{
		m_bEnabledInCustomizeMode = FALSE;
		m_wndColorBar.m_bStdColorDlg = bStdColorDlg;
	}

	CWTFColorPopupMenu(CWTFColorButton* pParentBtn, const CArray<COLORREF, COLORREF>& colors, COLORREF color, LPCTSTR lpszAutoColor, LPCTSTR lpszOtherColor,
		LPCTSTR lpszDocColors, CList<COLORREF,COLORREF>& lstDocColors, int nColumns, COLORREF colorAutomatic) :
	m_wndColorBar(colors, color, lpszAutoColor, lpszOtherColor, lpszDocColors, lstDocColors, nColumns, -1, -1, colorAutomatic, (UINT)-1, pParentBtn)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

	CWTFColorPopupMenu(CMFCRibbonColorButton* pParentBtn, const CArray<COLORREF, COLORREF>& colors, COLORREF color, LPCTSTR lpszAutoColor, LPCTSTR lpszOtherColor,
		LPCTSTR lpszDocColors, CList<COLORREF,COLORREF>& lstDocColors, int nColumns, COLORREF colorAutomatic, UINT nID) :
	m_wndColorBar(colors, color, lpszAutoColor, lpszOtherColor, lpszDocColors, lstDocColors, nColumns, colorAutomatic, nID, pParentBtn)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

	virtual ~CWTFColorPopupMenu();

// Attributes
protected:
	CWTFColorBar m_wndColorBar;
	BOOL      m_bEnabledInCustomizeMode;

public:
	void SetPropList(CWTFPropertyGridCtrl* pWndList) { m_wndColorBar.SetPropList(pWndList); }

// Overrides
	virtual CMFCPopupMenuBar* GetMenuBar() { return &m_wndColorBar; }
	virtual CPane* CreateTearOffBar(CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
