
// C3Dlg.h : header file
//

#pragma once

#include "renderdoc_app.h"
#include <PowerProps.h>

// C3Dlg dialog
class C3Dlg : public CDialog
{
public:

#define MOVE_FORWARD	0x0001
#define MOVE_BACKWARD	0x0002
#define MOVE_LEFT		0x0004
#define MOVE_RIGHT		0x0008
#define MOVE_UP			0x0010
#define MOVE_DOWN		0x0020
#define MOVE_RUN		0x0040

	props::TFlags64 m_fMovement;

protected:
	c3::Renderer *m_Rend;
	c3::FrameBuffer *m_GBuf;
	c3::FrameBuffer *m_LCBuf;
	c3::FrameBuffer *m_SSBuf;
	std::vector<c3::Texture2D *> m_ColorTarg;
	c3::DepthBuffer *m_DepthTarg;
	c3::DepthBuffer *m_ShadowTarg;
	c3::ShaderComponent *m_VS_copyback;
	c3::ShaderComponent *m_FS_copyback;
	c3::ShaderProgram *m_SP_copyback;

	UINT_PTR m_DrawTimerId;

	int32_t m_ulSunDir, m_ulSunColor, m_ulAmbientColor;
	glm::fvec3 m_SunDir, m_SunColor, m_AmbientColor;

	c3::Factory *m_Factory;
	c3::Object *m_RootObj;
	c3::Object *m_Camera;
	std::deque<c3::Object *> m_Light;
	std::deque<glm::fvec3> m_LightMove;

	bool m_MoveF, m_MoveL, m_MoveR, m_MoveB, m_Run, m_MoveU, m_MoveD;

	LARGE_INTEGER m_PerfFreq;
	LARGE_INTEGER m_PerfTime;

	RENDERDOC_API_1_4_0 *m_pRDoc;
	bool m_bCapturedFirstFrame;

	bool m_bMouseCursorEnabled;

public:

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

	void SetMouseEnabled(bool b);

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
	virtual BOOL DestroyWindow();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
