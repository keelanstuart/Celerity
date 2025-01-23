// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "CodeEditCtrl.h"


#pragma once


class CScriptingToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


class CScriptingWnd : public CDockablePane
{
// Construction
public:
	CScriptingWnd() noexcept;

	void EditScriptResource(c3::Resource *pres);
	bool IsEditingScriptResource(c3::Resource *pres, int &tabidx);
	bool ImmediateScriptEmpty();
	bool ResourceScriptChanged();

	void GetImmediateScript(CString &s);
	void GetResourceScript(CString &s);

	void UpdateResourceScript();
	void AssignImmediateScript();

// Attributes
protected:
	c3::Resource *m_pRes;

	CMFCTabCtrl m_wndTabs;
	CCodeEditCtrl m_wndCodeEditorRes;
	CCodeEditCtrl m_wndCodeEditorImm;

	void UpdateFonts();

// Implementation
public:
	virtual ~CScriptingWnd();

protected:
	void AdjustLayout();

	virtual BOOL OnShowControlBarMenu(CPoint point);
	virtual HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	virtual BOOL OnEraseBkgnd(CDC *pDC);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
};

