
// C3EditView.h : interface of the C3EditView class
//

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

	void AdjustYawPitch(float yawadj, float pitchadj, bool redraw = true);

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

	static c3::FrameBuffer *m_GBuf;
	static c3::FrameBuffer *m_LCBuf;
	static c3::FrameBuffer *m_SSBuf;
	static std::vector<c3::Texture2D *> m_ColorTarg;
	static c3::DepthBuffer *m_DepthTarg;
	static c3::DepthBuffer *m_ShadowTarg;
	static std::vector<c3::Texture2D *> m_BTex;
	static std::vector<c3::FrameBuffer *> m_BBuf;
	static c3::ShaderComponent *m_VS_resolve;
	static c3::ShaderComponent *m_FS_resolve;
	static c3::ShaderProgram *m_SP_resolve;
	static c3::ShaderComponent *m_VS_blur;
	static c3::ShaderComponent *m_FS_blur;
	static c3::ShaderProgram *m_SP_blur;
	static c3::ShaderComponent *m_VS_combine;
	static c3::ShaderComponent *m_FS_combine;
	static c3::ShaderProgram *m_SP_combine;
	static c3::ShaderComponent *m_VS_bounds;
	static c3::ShaderComponent *m_FS_bounds;
	static c3::ShaderProgram *m_SP_bounds;

	static int32_t m_ulSunDir;
	static int32_t m_ulSunColor;
	static int32_t m_ulAmbientColor;
	static glm::fvec3 m_SunDir;
	static glm::fvec3 m_SunColor;
	static glm::fvec3 m_AmbientColor;
	static int32_t m_uBlurTex;
	static int32_t m_uBlurScale;
	static c3::MatrixStack *m_SelectionXforms;

	static RENDERDOC_API_1_4_0 *m_pRenderDoc;
	bool m_RenderDocCaptureFrame;

	c3::Object *m_pHoverObj;

	CPoint m_MousePos;
	UINT_PTR m_DrawTimer;
	UINT_PTR m_PickTimer;
	glm::fvec3 m_BasePickPos;
	glm::fvec3 m_BasePickVec;


// Generated message map functions
protected:
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
};

#ifndef _DEBUG  // debug version in C3EditView.cpp
inline C3EditDoc* C3EditView::GetDocument() const
   { return reinterpret_cast<C3EditDoc*>(m_pDocument); }
#endif

