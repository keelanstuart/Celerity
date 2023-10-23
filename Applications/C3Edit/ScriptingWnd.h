// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


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

	void SetResourceScript(c3::Resource *pres);

// Attributes
protected:
	CMFCTabCtrl m_wndTabs;
	CCodeEditCtrl m_wndCodeEditorRes;
	CCodeEditCtrl m_wndCodeEditorImm;
	CScriptingToolBar m_wndToolBar;

	void UpdateFonts();

// Implementation
public:
	virtual ~CScriptingWnd();

protected:
	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnNewScript();
	afx_msg void OnUpdateNewScript(CCmdUI *pCmdUI);
	afx_msg void OnLoadScript();
	afx_msg void OnUpdateLoadScript(CCmdUI *pCmdUI);
	afx_msg void OnSaveScript();
	afx_msg void OnUpdateSaveScript(CCmdUI *pCmdUI);
	afx_msg void OnRunScript();
	afx_msg void OnUpdateRunScript(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
};

