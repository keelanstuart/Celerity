
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
	void AddToSelection(c3::Object *obj);
	void RemoveFromSelection(c3::Object *obj);
	size_t GetNumSelected();

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

	static int32_t m_ulSunDir;
	static int32_t m_ulSunColor;
	static int32_t m_ulAmbientColor;
	static glm::fvec3 m_SunDir;
	static glm::fvec3 m_SunColor;
	static glm::fvec3 m_AmbientColor;
	static int32_t m_uBlurTex;
	static int32_t m_uBlurScale;

	static RENDERDOC_API_1_4_0 *m_pRenderDoc;
	bool m_RenderDocCaptureFrame;

	CPoint m_MousePos;
	UINT_PTR m_DrawTimer;

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
	afx_msg void OnUpdateEditTriggerrenderdoccapture(CCmdUI *pCmdUI);
	afx_msg void OnEditTriggerrenderdoccapture();
};

#ifndef _DEBUG  // debug version in C3EditView.cpp
inline C3EditDoc* C3EditView::GetDocument() const
   { return reinterpret_cast<C3EditDoc*>(m_pDocument); }
#endif

