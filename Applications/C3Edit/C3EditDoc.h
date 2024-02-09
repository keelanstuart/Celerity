// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once


class C3EditDoc : public CDocument
{
protected: // create from serialization only
	C3EditDoc() noexcept;
	DECLARE_DYNCREATE(C3EditDoc)

// Attributes
public:

	typedef struct sPerViewInfo
	{
		c3::Object *m_Camera;
		float yaw, pitch;
		c3::Object *m_GUICamera;
	} SPerViewInfo;

	c3::Object *m_RootObj;
	c3::Object *m_GUIRootObj;

	typedef std::map<HWND, SPerViewInfo> TWndMappedObject;
	TWndMappedObject m_PerViewInfo;
	c3::Object *m_Brush;
	float m_TimeWarp;
	bool m_Paused;

	tstring m_Name;
	tstring m_Description;
	tstring m_Author;
	tstring m_Website;
	tstring m_Copyright;

	glm::fvec4 m_ClearColor;
	glm::fvec4 m_ShadowColor;
	glm::fvec4 m_FogColor;
	float m_FogDensity;

// Operations
public:
	SPerViewInfo *GetPerViewInfo(HWND h);

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~C3EditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void ResetViews();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
};
