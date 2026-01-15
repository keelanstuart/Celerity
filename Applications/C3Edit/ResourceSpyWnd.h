// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "C3ResourceSpyListCtrl.h"

#pragma once


class CResourcesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


class CResourceSpyWnd : public CDockablePane
{
// Construction
public:
	CResourceSpyWnd() noexcept;

// Attributes
protected:

	C3ResourceSpyListCtrl m_ResList;
	uint64_t m_Frame;
	
// Implementation
public:
	virtual ~CResourceSpyWnd();

protected:
	void AdjustLayout();

	virtual BOOL OnShowControlBarMenu(CPoint point);
	virtual HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	virtual BOOL OnEraseBkgnd(CDC *pDC);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnNotifyListData(NMHDR *phdr, LRESULT *pres);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

};

