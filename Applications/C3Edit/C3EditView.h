// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include "renderdoc_app.h"


class C3EditView : public CView
{
protected: // create from serialization only
	C3EditView() noexcept;
	DECLARE_DYNCREATE(C3EditView)

// Attributes
public:
	C3EditDoc* GetDocument() const;

protected:
	typedef std::deque<c3::Object *> TObjectArray;
	TObjectArray m_Selected;
	bool m_ShowDebug;

// Operations
public:
	void ClearSelection();
	bool IsSelected(const c3::Object *obj) const;
	void AddToSelection(const c3::Object *obj);
	void RemoveFromSelection(const c3::Object *obj);
	size_t GetNumSelected();
	c3::Object *GetSelection(size_t index) const;

	void ComputePickRay(POINT screenpos, glm::fvec3 &pickpos, glm::fvec3 &pickvec) const;
	c3::Object *Pick(POINT p) const;

	void UpdateStatusMessage(c3::Object *pobj = nullptr);
	void UpdateObjectList();

	void AdjustYawPitch(float yawadj, float pitchadj, bool redraw = true);

	void SetAppropriateMouseCursor(UINT32 nFlags);

	void CenterViewOnSelection();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~C3EditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	static size_t m_ResourcesRefCt;

#define BLURTARGS	4

	c3::FrameBuffer *m_GBuf;
	c3::FrameBuffer *m_LCBuf;
	c3::FrameBuffer *m_AuxBuf;
	c3::FrameBuffer *m_SSBuf;
	std::vector<c3::Texture2D *> m_ColorTarg;
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

	int32_t m_uBlurTex;
	int32_t m_uBlurScale;
	c3::MatrixStack *m_SelectionXforms;

	static RENDERDOC_API_1_4_0 *m_pRenderDoc;
	bool m_RenderDocCaptureFrame;

	c3::Object *m_pHoverObj;

	CPoint m_MousePos;
	UINT_PTR m_DrawTimer;
	UINT_PTR m_PickTimer;
	glm::fvec3 m_BasePickPos;
	glm::fvec3 m_BasePickVec;
	c3::MatrixStack *m_PickMats;

	float m_WindowsUIScale;

	c3::Object *m_pUICam;

// Generated message map functions
protected:

	void InitializeGraphics();
	void CreateSurfaces();
	void DestroySurfaces();
	void UpdateShaderSurfaces();

	void HandleInput(c3::Positionable* pcampos);

	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

public:
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateEditTriggerRenderDocCapture(CCmdUI *pCmdUI);
	afx_msg void OnEditTriggerRenderDocCapture();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDuplicate(CCmdUI *pCmdUI);
	afx_msg void OnEditDuplicate();
	afx_msg void OnUpdateEditGroup(CCmdUI *pCmdUI);
	afx_msg void OnEditGroup();
	afx_msg void OnUpdateEditUngroup(CCmdUI *pCmdUI);
	afx_msg void OnEditUngroup();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnGraphDeleteNode();
    afx_msg void OnUpdateGraphDeleteNode(CCmdUI *pCmdUI);
	afx_msg void OnEditCenterCamera();
	afx_msg void OnUpdateEditCenterCamera(CCmdUI *pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditCameraSettings();
	afx_msg void OnUpdateEditCameraSettings(CCmdUI *pCmdUI);
    afx_msg void OnUpdateViewHoverInformation(CCmdUI *pCmdUI);
    afx_msg void OnViewHoverInformation();
};

#ifndef _DEBUG  // debug version in C3EditView.cpp
inline C3EditDoc* C3EditView::GetDocument() const
   { return reinterpret_cast<C3EditDoc*>(m_pDocument); }
#endif

