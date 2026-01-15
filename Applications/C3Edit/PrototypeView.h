// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include "ViewTree.h"
#include <optional>

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
	void ChangeSelection(int num);

protected:
	CViewTree m_wndPrototypeView;
	CImageList m_PrototypeViewImages;
	std::optional<TVITEM> m_DragItem;
	CImageList *m_DragImage;
	HTREEITEM m_hEditItem;

	HTREEITEM FindChildItem(HTREEITEM hroot, const TCHAR *itemname);
	HTREEITEM MakeProtoGroup(HTREEITEM hroot, const TCHAR *group);
	HTREEITEM MakePrototype(HTREEITEM hroot, const TCHAR *group);
	HTREEITEM FindItemByPrototype(HTREEITEM hroot, const c3::Prototype *pproto);
	void FillPrototypeView();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void UpdateItem(const c3::Prototype *pproto);

	using PrototypeActionFunc = std::function<void(c3::Prototype *)>;
	void ActOnSelection(PrototypeActionFunc func, bool recursive);

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
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};

