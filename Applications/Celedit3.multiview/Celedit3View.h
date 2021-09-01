
// Celedit3View.h : interface of the CCeledit3View class
//

#pragma once

class CCeledit3CntrItem;

class CCeledit3View : public CView
{
protected: // create from serialization only
	CCeledit3View() noexcept;
	DECLARE_DYNCREATE(CCeledit3View)

// Attributes
public:
	CCeledit3Doc* GetDocument() const;
	// m_pSelection holds the selection to the current CCeledit3CntrItem.
	// For many applications, such a member variable isn't adequate to
	//  represent a selection, such as a multiple selection or a selection
	//  of objects that are not CCeledit3CntrItem objects.  This selection
	//  mechanism is provided just to help you get started

	// TODO: replace this selection mechanism with one appropriate to your app
	CCeledit3CntrItem* m_pSelection;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support

// Implementation
public:
	virtual ~CCeledit3View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	c3::Renderer *m_Rend;
	glm::fvec4 m_ClearColor;

// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnInsertObject();
	afx_msg void OnCancelEditCntr();
	afx_msg void OnFilePrint();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in Celedit3View.cpp
inline CCeledit3Doc* CCeledit3View::GetDocument() const
   { return reinterpret_cast<CCeledit3Doc*>(m_pDocument); }
#endif

