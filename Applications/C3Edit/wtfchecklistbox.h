#pragma once

class CWtfCheckListBox : public CListBox
{
	DECLARE_DYNAMIC(CWtfCheckListBox)

	// Constructors
public:
	CWtfCheckListBox();
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// Attributes
	void SetCheckStyle(UINT nStyle);
	UINT GetCheckStyle();
	void SetCheck(int nIndex, int nCheck);
	int GetCheck(int nIndex);
	void Enable(int nIndex, BOOL bEnabled = TRUE);
	BOOL IsEnabled(int nIndex);

	virtual CRect OnGetCheckPosition(CRect rectItem, CRect rectCheckBox);

	// Overridables (must override draw, measure and compare for owner draw)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	// Implementation
protected:
	void PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int PreCompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void PreDeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	bool PreDrawItemThemed(CDC* pDC, DRAWITEMSTRUCT &drawItem, int nCheck, int cyItem);
	void PreDrawItemNonThemed(CDC* pDC, DRAWITEMSTRUCT &drawItem, int nCheck, int cyItem);
	void PreDrawItemHelper(LPDRAWITEMSTRUCT lpDrawItemStruct);

	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);

	void SetSelectionCheck( int nCheck );

#ifdef _DEBUG
	virtual void PreSubclassWindow();
#endif

	int CalcMinimumItemHeight();
	void InvalidateCheck(int nIndex);
	void InvalidateItem(int nIndex);
	int CheckFromPoint(CPoint point, BOOL& bInCheck);

	int m_cyText;
	UINT m_nStyle;

	// Message map functions
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBAddString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBFindString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBFindStringExact(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBGetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBInsertString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSelectString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSetItemHeight(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
