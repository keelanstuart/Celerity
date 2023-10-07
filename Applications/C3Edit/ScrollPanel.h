#pragma once

class CScrollPanel : public CWnd
{
	DECLARE_DYNAMIC(CScrollPanel)

// Constructors
public:
	CScrollPanel();
	virtual ~CScrollPanel();

public:
	static AFX_DATA const SIZE sizeDefault;
		// used to specify default calculated page and line sizes

	// in logical units - call one of the following Set routines
	void SetScaleToFitSize(SIZE sizeTotal);
	void SetScrollSizes(int nMapMode, SIZE sizeTotal,
				const SIZE& sizePage = sizeDefault,
				const SIZE& sizeLine = sizeDefault);

// Attributes
public:
	CPoint GetScrollPosition() const;       // upper corner of scrolling
	CSize GetTotalSize() const;             // logical size

	void CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const;

	// for device units
	CPoint GetDeviceScrollPosition() const;
	void GetDeviceScrollSizes(int& nMapMode, SIZE& sizeTotal,
			SIZE& sizePage, SIZE& sizeLine) const;

// Operations
public:
	void ScrollToPosition(POINT pt);    // set upper left position
	void FillOutsideRect(CDC* pDC, CBrush* pBrush);
	void ResizeParentToFit(BOOL bShrinkOnly = TRUE);
	BOOL DoMouseWheel(UINT fFlags, short zDelta, CPoint point);

// Implementation
protected:
	int m_nMapMode;				 // mapping mode for window creation
	CSize m_totalLog;           // total size in logical units (no rounding)
	CSize m_totalDev;           // total size in device units
	CSize m_pageDev;            // per page scroll size in device units
	CSize m_lineDev;            // per line scroll size in device units

	BOOL m_bCenter;             // Center output if larger than total size
	BOOL m_bInsideUpdate;       // internal state for OnSize callback
	void CenterOnPoint(CPoint ptCenter);
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking

protected:
	void OnDraw(CDC* pDC);      // pass on pure virtual

	void UpdateBars();          // adjust scrollbars etc
	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);
		// size with no bars
	void GetScrollBarSizes(CSize& sizeSb);
	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);

public:

	void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder) override;
	void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	virtual CSize GetWheelScrollDistance(CSize sizeDistance,
		BOOL bHorz, BOOL bVert);

	// scrolling implementation support for OLE
	BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);
	afx_msg LRESULT HandleMButtonDown(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
