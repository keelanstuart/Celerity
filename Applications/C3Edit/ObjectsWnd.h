// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "C3ObjectListCtrl.h"
#include <optional>

#pragma once

class CObjectWnd : public CDockablePane
{
public:
	CObjectWnd() noexcept;
	virtual ~CObjectWnd();

	void AdjustLayout();
	//void OnChangeVisualStyle();

protected:
	C3ObjectListCtrl m_ObjList;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	size_t GetVisibleItemCount(const c3::Object *proot);
	const c3::Object *GetItemByIndex(const c3::Object *proot, size_t index, size_t *depth = nullptr);

	void UpdateContents();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void GetObjectListItemData();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);

};

