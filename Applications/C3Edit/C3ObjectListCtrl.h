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
};


