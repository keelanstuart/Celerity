#include <optional>

#pragma once


// C3ObjectListCtrl

class C3ObjectListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(C3ObjectListCtrl)

public:
	C3ObjectListCtrl();
	virtual ~C3ObjectListCtrl();

protected:
	std::optional<CPoint> m_HoverPos;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void DrawItem(LPDRAWITEMSTRUCT pdi);
	afx_msg void OnPaint();
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT pmi);

	virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDuplicate(CCmdUI *pCmdUI);
	afx_msg void OnEditDuplicate();
	afx_msg void OnUpdateEditAssignRoot(CCmdUI *pCmdUI);
	afx_msg void OnEditAssignRoot();
	afx_msg void OnUpdateEditExport(CCmdUI *pCmdUI);
	afx_msg void OnEditExport();
	afx_msg void OnUpdateEditGroup(CCmdUI *pCmdUI);
	afx_msg void OnEditGroup();
	afx_msg void OnUpdateEditUngroup(CCmdUI *pCmdUI);
	afx_msg void OnEditUngroup();
	afx_msg void OnUpdateEditCenterCamera(CCmdUI *pCmdUI);
	afx_msg void OnEditCenterCamera();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI *pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);

	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
};


