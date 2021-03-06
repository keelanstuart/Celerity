
// C3Dlg.h : header file
//

#pragma once


// C3Dlg dialog
class C3Dlg : public CDialog
{
protected:
	c3::Renderer *m_Rend;
	c3::FrameBuffer *m_FB;
	c3::Texture2D *m_ColorTarg[2];
	c3::DepthBuffer *m_DepthTarg;

	c3::Factory *m_Factory;
	c3::Object *m_RootObj;
	c3::Object *m_Camera;

	LARGE_INTEGER m_PerfFreq;
	LARGE_INTEGER m_PerfTime;

// Construction
public:
	C3Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_C3APP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void Cleanup();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnFinalRelease();
	virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL DestroyWindow();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};
