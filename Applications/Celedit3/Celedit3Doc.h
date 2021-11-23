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

// Celedit3Doc.h : interface of the CCeledit3Doc class
//


#pragma once


class CCeledit3Doc : public CDocument
{
protected: // create from serialization only
	CCeledit3Doc() noexcept;
	DECLARE_DYNCREATE(CCeledit3Doc)

// Attributes
public:
	typedef enum
	{
		CAM_FREE = 0,
		CAM_LEFT,
		CAM_RIGHT,
		CAM_TOP,
		CAM_BOTTOM,
		CAM_FRONT,
		CAM_BACK,

		CAM_NUMCAMS
	} CAMTYPE;

	c3::Object *m_RootObj;
	c3::Object *m_Camera[CAM_NUMCAMS];
	c3::Object *m_Brush;

	tstring m_Name;
	tstring m_Description;
	tstring m_Author;
	tstring m_Website;
	tstring m_Copyright;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CCeledit3Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
