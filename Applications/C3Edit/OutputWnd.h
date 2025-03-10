// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#pragma once

class CDebugOutputEdit : public CEdit
{
protected:
	HBRUSH CtlColor(CDC *pDC, UINT);
	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd() noexcept;

	void UpdateFonts();

// Attributes
protected:

	CDebugOutputEdit m_wndOutputDebug;
	DWORD m_hUIThread;

protected:
	void FillDebugWindow();

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL DestroyWindow();
    afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

