// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include "renderdoc_app.h"
#include <PowerProps.h>

// C3Dlg dialog
class C3Dlg : public CDialog
{
public:

protected:

#define BLURTARGS	4

	c3::FrameBuffer *m_GBuf;
	c3::FrameBuffer *m_LCBuf;
	c3::FrameBuffer *m_AuxBuf;
	c3::FrameBuffer *m_SSBuf;
	c3::FrameBuffer *m_EffectsBuf;
	c3::DepthBuffer *m_DepthTarg;
	c3::DepthBuffer *m_ShadowTarg;
	std::vector<c3::Texture2D *> m_BTex;
	std::vector<c3::FrameBuffer *> m_BBuf;
	c3::ShaderComponent *m_VS_resolve;
	c3::ShaderComponent *m_FS_resolve;
	c3::ShaderProgram *m_SP_resolve;
	c3::ShaderComponent *m_VS_blur;
	c3::ShaderComponent *m_FS_blur;
	c3::ShaderProgram *m_SP_blur;
	c3::ShaderComponent *m_VS_combine;
	c3::ShaderComponent *m_FS_combine;
	c3::ShaderProgram *m_SP_combine;
	c3::ShaderComponent *m_VS_bounds;
	c3::ShaderComponent *m_FS_bounds;
	c3::ShaderProgram *m_SP_bounds;

	int32_t m_ulSunDir;
	int32_t m_ulSunColor;
	int32_t m_ulAmbientColor;
	int32_t m_ulFogColor;
	int32_t m_ulFogDensity;
	int32_t m_ulFogStart;
	int32_t m_ulFogEnd;

	int32_t m_uBlurTex;
	int32_t m_uBlurScale;

	bool m_MotionBlur;
	size_t m_MotionUpdateIdx;

	bool m_bSurfacesCreated, m_bSurfacesReady;
	bool m_ShowDebug;

	CPoint m_MousePos;

	float m_WindowsUIScale;

	static bool __cdecl DeviceConnected(c3::InputDevice *device, bool conn, void *userdata);

public:

// Construction
public:
	C3Dlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~C3Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_C3APP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void Cleanup();

	void SetMouseEnabled(bool b);

	void RegisterAction(const TCHAR *name, c3::ActionMapper::ETriggerType tt, float delay);

	void DestroySurfaces();

	void CreateSurfaces();

	void InitializeGraphics();

// Implementation
protected:
	HICON m_hIcon;

	void UpdateShaderSurfaces();

	void ComputePickRay(const c3::Object *cam, POINT screenpos, glm::fvec3 &pickpos, glm::fvec3 &pickvec) const;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
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
	afx_msg LRESULT OnDevChange(WPARAM wparam, LPARAM lparam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMove(int x, int y);
};
