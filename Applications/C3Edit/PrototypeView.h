// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Prototypees Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#pragma once

#include "ViewTree.h"

class CPrototypeToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPrototypeView : public CDockablePane
{
public:
	CPrototypeView() noexcept;
	virtual ~CPrototypeView();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	CPrototypeToolBar m_wndToolBar;
	CViewTree m_wndPrototypeView;
	CImageList m_PrototypeViewImages;
	UINT m_nCurrSort;

	HTREEITEM FindChildItem(HTREEITEM hroot, const TCHAR *itemname);
	HTREEITEM MakeProtoGroup(HTREEITEM hroot, const TCHAR *group);
	HTREEITEM MakePrototype(HTREEITEM hroot, const TCHAR *group);
	void FillPrototypeView();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPrototypeSearch();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);
};

