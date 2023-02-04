
#pragma once

class CDebugOutputEdit : public CEdit
{
protected:
	HBRUSH CtlColor(CDC *pDC, UINT);
	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd() noexcept;

	void UpdateFonts();

// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;

	CDebugOutputEdit m_wndOutputDebug;

protected:
	void FillDebugWindow();

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

