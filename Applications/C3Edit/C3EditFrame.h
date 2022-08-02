
// C3EditFrame.h : interface of the C3EditFrame class
//

#pragma once
#include "PrototypeView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
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

protected:
	CMFCToolBarButton *GetToolButtonByID(CMFCToolBar *toolbar, DWORD butid);

protected:  // control bar embedded members
	COutputWnd        m_wndOutput;
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndAxesToolBar;
	CMFCToolBar       m_wnd3DToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CPrototypeView    m_wndProtoView;
	CPropertiesWnd    m_wndProperties;
	CMFCShellTreeCtrl m_wndTree;
	CMFCCaptionBar    m_wndCaptionBar;

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
	afx_msg void OnOptions();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	BOOL CreateCaptionBar();
};


