
// C3Dlg.h : header file
//

#pragma once

#include "renderdoc_app.h"
#include <PowerProps.h>

#define BLURTARGS	4


// C3Dlg dialog
class C3Dlg : public CDialog
{
public:

protected:
	c3::Renderer *m_Rend;
	c3::InputManager *m_Input;
	c3::FrameBuffer *m_GBuf;
	c3::FrameBuffer *m_LCBuf;
	c3::FrameBuffer *m_SSBuf;
	std::vector<c3::Texture2D *> m_ColorTarg;
	c3::DepthBuffer *m_DepthTarg;
	c3::DepthBuffer *m_ShadowTarg;
	c3::Texture2D *m_BTex[BLURTARGS];
	c3::FrameBuffer *m_BBuf[BLURTARGS];
	c3::ShaderComponent *m_VS_resolve;
	c3::ShaderComponent *m_FS_resolve;
	c3::ShaderProgram *m_SP_resolve;
	c3::ShaderComponent *m_VS_blur;
	c3::ShaderComponent *m_FS_blur;
	c3::ShaderProgram *m_SP_blur;
	c3::ShaderComponent *m_VS_combine;
	c3::ShaderComponent *m_FS_combine;
	c3::ShaderProgram *m_SP_combine;
	bool m_bFirstDraw;

	UINT_PTR m_DrawTimerId;

	int32_t m_uBlurTex, m_uBlurScale;

	c3::Factory *m_Factory;
	c3::Object *m_RootObj;
	c3::Object *m_CameraRoot, *m_CameraArm, *m_Camera;
	std::deque<c3::Object *> m_Light;
	std::deque<glm::fvec3> m_LightMove;

#define MAX_USERS	2

	struct SControls
	{
		SControls() { memset(&move, 0, sizeof(SMove)); memset(&look, 0, sizeof(SLook)); }
		struct SMove
		{
			float forward;
			float backward;
			float left;
			float right;
			float up;
			float down;
			float run;
			float jump;
		} move;

		struct SLook
		{
			float up;
			float down;
			float left;
			float right;
		} look;

	} m_Controls[MAX_USERS];

	c3::Object *m_pControllable[MAX_USERS];

	enum EViewMode
	{
		VM_FOLLOW_POSDIR = 0,
		VM_FOLLOW_POS,
		VM_FREE,

		VM_NUMMODES
	};
	size_t m_ViewMode;

	LARGE_INTEGER m_PerfFreq;
	LARGE_INTEGER m_PerfTime;

	RENDERDOC_API_1_4_0 *m_pRDoc;
	bool m_bCapturedFirstFrame;

	bool m_bMouseCursorEnabled;

	static bool __cdecl DeviceConnected(c3::InputDevice *device, bool conn, void *userdata);

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

	void RegisterAction(const TCHAR *name, c3::ActionMapper::ETriggerType tt, float delay);


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
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnDevChange(WPARAM wparam, LPARAM lparam);
};
