// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// Celedit3View.h : interface of the CCeledit3View class
//

#pragma once


class CCeledit3View : public CView
{
protected: // create from serialization only
	CCeledit3View() noexcept;
	DECLARE_DYNCREATE(CCeledit3View)

// Attributes
public:
	CCeledit3Doc* GetDocument() const;

protected:
	typedef std::deque<const c3::Object *> TObjectArray;
	TObjectArray m_Selected;


// Operations
public:
	void ClearSelection();
	void AddToSelection(const c3::Object *obj);
	void RemoveFromSelection(const c3::Object *obj);
	size_t GetNumSelected();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CCeledit3View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CPoint m_MousePos;
	UINT_PTR m_DrawTimer;

	c3::Renderer *m_Rend;
	glm::fvec4 m_ClearColor;

	c3::Camera *m_pCam;
	c3::Positionable *m_pCamPos;
	float m_CamPitch, m_CamYaw;

	c3::FrameBuffer *m_FB;
	c3::Texture2D *m_ColorTarg[3];
	c3::DepthBuffer *m_DepthTarg;

	c3::ShaderComponent *m_VS_copyback;
	c3::ShaderComponent *m_FS_copyback;
	c3::ShaderProgram *m_SP_copyback;


// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
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
};

#ifndef _DEBUG  // debug version in Celedit3View.cpp
inline CCeledit3Doc* CCeledit3View::GetDocument() const
   { return reinterpret_cast<CCeledit3Doc*>(m_pDocument); }
#endif

