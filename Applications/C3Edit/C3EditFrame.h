// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once
#include "PrototypeView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "ScriptingWnd.h"
#include "ResourceSpyWnd.h"
#include "ObjectsWnd.h"
#include "Resource.h"
#include "C3Edit.h"

class COutlookBar : public CMFCOutlookBar
{
	virtual BOOL AllowShowOnPaneMenu() const { return TRUE; }
	virtual void GetPaneName(CString& strName) const { BOOL bNameValid = strName.LoadString(IDS_OUTLOOKBAR); ASSERT(bNameValid); if (!bNameValid) strName.Empty(); }
};

class C3EditFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	C3EditFrame() noexcept;
	DECLARE_DYNCREATE(C3EditFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// Implementation
public:
	virtual ~C3EditFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SetActiveTool(C3EditApp::ToolType toolid);

	void IncludeAxis(props::TFlags64 axis);
	void ExcludeAxis(props::TFlags64 axis);
	void SetAxes(props::TFlags64 axes);
	props::TFlags64 GetAxes();
	BOOL AxisActive(props::TFlags64 axis);

	void SetActiveProperties(props::IPropertySet *props, bool readonly = false, const TCHAR *title = nullptr);
	void SetActivePrototype(c3::Prototype *pproto, bool readonly = false, const TCHAR *title = nullptr);
	void SetActiveObject(c3::Object *pobj, bool readonly = false, const TCHAR *title = nullptr);
	void RefreshActiveProperties();
	void UpdateObjectList();

protected:
	CMFCToolBarButton *GetToolButtonByID(CMFCToolBar *toolbar, DWORD butid);

public:  // control bar embedded members
	COutputWnd        m_wndOutput;
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndAxesToolBar;
	CMFCToolBar       m_wnd3DToolBar;
	CMFCToolBar       m_wndScriptingToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CPrototypeView    m_wndProtoView;
	CPropertiesWnd    m_wndProperties;
	CScriptingWnd     m_wndScripting;
	CResourceSpyWnd	  m_wndResourceSpy;
	CObjectWnd        m_wndObjects;
	CMFCShellTreeCtrl m_wndTree;
	CMFCCaptionBar    m_wndCaptionBar;

	float m_Scale;

// Generated message map functions
protected:
	afx_msg void OnUpdateAxisScreenrel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAxisX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAxisY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAxisZ(CCmdUI* pCmdUI);

	afx_msg void OnAxisScreenrel();
	afx_msg void OnAxisX();
	afx_msg void OnAxisY();
	afx_msg void OnAxisZ();

	afx_msg void OnUpdateToolSelect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolTranslate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolRotate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolScale(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolUniscale(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolWand(CCmdUI* pCmdUI);

	afx_msg void OnToolSelect();
	afx_msg void OnToolTranslate();
	afx_msg void OnToolRotate();
	afx_msg void OnToolScale();
	afx_msg void OnToolUniscale();
	afx_msg void OnToolWand();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileClose();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnViewFileView();
	afx_msg void OnUpdateViewFileView(CCmdUI* pCmdUI);
	afx_msg void OnViewProtoView();
	afx_msg void OnUpdateViewProtoView(CCmdUI* pCmdUI);
	afx_msg void OnViewOutputWindow();
	afx_msg void OnUpdateViewOutputWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewPropertiesWindow();
	afx_msg void OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
	afx_msg void OnViewScriptingWindow();
	afx_msg void OnUpdateViewScriptingWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewResourceSpyWindow();
	afx_msg void OnUpdateViewResourceSpyWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewObjectsWindow();
	afx_msg void OnUpdateViewObjectsWindow(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	afx_msg void OnUpdateAssignScript(CCmdUI *pCmdUI);
	afx_msg void OnAssignScript();
	afx_msg void OnUpdateRunScript(CCmdUI *pCmdUI);
	afx_msg void OnRunScript();
	afx_msg void OnUpdateUpdateScript(CCmdUI *pCmdUI);
	afx_msg void OnUpdateScript();

	afx_msg void OnUpdateReloadResource(CCmdUI *pCmdUI);
	afx_msg void OnReloadResource();

	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	BOOL CreateCaptionBar();
public:
	afx_msg void OnMove(int x, int y);
	afx_msg void OnClose();
    afx_msg void OnEditResourcePathConfiguration();
    afx_msg void OnUpdateAppAdvanceTime(CCmdUI *pCmdUI);
    afx_msg void OnAppAdvanceTime();
    afx_msg void OnUpdateAppEditorDraw(CCmdUI *pCmdUI);
    afx_msg void OnAppEditorDraw();
    afx_msg void OnDockDebug();
    afx_msg void OnUpdateDockDebug(CCmdUI *pCmdUI);
    afx_msg void OnDockObjects();
    afx_msg void OnUpdateDockObjects(CCmdUI *pCmdUI);
    afx_msg void OnDockProperties();
    afx_msg void OnUpdateDockProperties(CCmdUI *pCmdUI);
    afx_msg void OnDockPrototypes();
    afx_msg void OnUpdateDockPrototypes(CCmdUI *pCmdUI);
    afx_msg void OnDockResources();
    afx_msg void OnUpdateDockResources(CCmdUI *pCmdUI);
    afx_msg void OnDockScripts();
    afx_msg void OnUpdateDockScripts(CCmdUI *pCmdUI);
	afx_msg void OnHelpSdkDocs();
	afx_msg void OnUpdateHelpSdkDocs(CCmdUI *pCmdUI);
	afx_msg void OnHelpGenComponent();
	afx_msg void OnUpdateHelpGenComponent(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditPackfileManagement(CCmdUI *pCmdUI);
	afx_msg void OnEditPackfileManagement();
};


