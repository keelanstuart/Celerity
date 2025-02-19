// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "C3EditFrame.h"
#include "C3EditDoc.h"
#include "C3EditView.h"
#include <shellscalingapi.h>
#include "ResourcePathEditorDlg.h"
#include "PackfileManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// C3EditFrame

IMPLEMENT_DYNCREATE(C3EditFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(C3EditFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &C3EditFrame::OnToolbarCreateNew)
	ON_COMMAND(ID_FILE_CLOSE, &C3EditFrame::OnFileClose)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &C3EditFrame::OnViewCustomize)
	ON_COMMAND(ID_VIEW_CAPTION_BAR, &C3EditFrame::OnViewCaptionBar)
	ON_COMMAND(ID_TOOLS_OPTIONS, &C3EditFrame::OnOptions)
	ON_COMMAND(ID_VIEW_PROTOVIEW, &C3EditFrame::OnViewProtoView)
	ON_COMMAND(ID_VIEW_OUTPUTWND, &C3EditFrame::OnViewOutputWindow)
	ON_COMMAND(ID_VIEW_PROPERTIESWND, &C3EditFrame::OnViewPropertiesWindow)
	ON_COMMAND(ID_VIEW_SCRIPTINGWND, &C3EditFrame::OnViewScriptingWindow)
	ON_COMMAND(ID_VIEW_RESOURCESPYWND, &C3EditFrame::OnViewResourceSpyWindow)
	ON_COMMAND(ID_VIEW_OBJECTSWND, &C3EditFrame::OnViewObjectsWindow)

	ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &C3EditFrame::OnUpdateViewCaptionBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROTOVIEW, &C3EditFrame::OnUpdateViewProtoView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &C3EditFrame::OnUpdateViewOutputWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIESWND, &C3EditFrame::OnUpdateViewPropertiesWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCRIPTINGWND, &C3EditFrame::OnUpdateViewScriptingWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RESOURCESPYWND, &C3EditFrame::OnUpdateViewResourceSpyWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTSWND, &C3EditFrame::OnUpdateViewObjectsWindow)

	ON_WM_SETTINGCHANGE()

	ON_COMMAND(ID_TOOL_SELECT, &C3EditFrame::OnToolSelect)
	ON_COMMAND(ID_TOOL_TRANSLATE, &C3EditFrame::OnToolTranslate)
	ON_COMMAND(ID_TOOL_ROTATE, &C3EditFrame::OnToolRotate)
	ON_COMMAND(ID_TOOL_SCALE, &C3EditFrame::OnToolScale)
	ON_COMMAND(ID_TOOL_UNISCALE, &C3EditFrame::OnToolUniscale)
	ON_COMMAND(ID_TOOL_WAND, &C3EditFrame::OnToolWand)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SELECT, &C3EditFrame::OnUpdateToolSelect)
	ON_UPDATE_COMMAND_UI(ID_TOOL_TRANSLATE, &C3EditFrame::OnUpdateToolTranslate)
	ON_UPDATE_COMMAND_UI(ID_TOOL_ROTATE, &C3EditFrame::OnUpdateToolRotate)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SCALE, &C3EditFrame::OnUpdateToolScale)
	ON_UPDATE_COMMAND_UI(ID_TOOL_UNISCALE, &C3EditFrame::OnUpdateToolUniscale)
	ON_UPDATE_COMMAND_UI(ID_TOOL_WAND, &C3EditFrame::OnUpdateToolWand)

	ON_COMMAND(ID_AXIS_SCREENREL, &C3EditFrame::OnAxisScreenrel)
	ON_COMMAND(ID_AXIS_X, &C3EditFrame::OnAxisX)
	ON_COMMAND(ID_AXIS_Y, &C3EditFrame::OnAxisY)
	ON_COMMAND(ID_AXIS_Z, &C3EditFrame::OnAxisZ)
	ON_UPDATE_COMMAND_UI(ID_AXIS_SCREENREL, &C3EditFrame::OnUpdateAxisScreenrel)
	ON_UPDATE_COMMAND_UI(ID_AXIS_X, &C3EditFrame::OnUpdateAxisX)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Y, &C3EditFrame::OnUpdateAxisY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Z, &C3EditFrame::OnUpdateAxisZ)

	ON_COMMAND(ID_SCRIPT_ASSIGN, &C3EditFrame::OnAssignScript)
	ON_COMMAND(ID_SCRIPT_RUN, &C3EditFrame::OnRunScript)
	ON_COMMAND(ID_SCRIPT_UPDATE, &C3EditFrame::OnUpdateScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_ASSIGN, &C3EditFrame::OnUpdateAssignScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_RUN, &C3EditFrame::OnUpdateRunScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_UPDATE, &C3EditFrame::OnUpdateUpdateScript)

	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_EDIT_RESOURCEPATHCONFIGURATION, &C3EditFrame::OnEditResourcePathConfiguration)
    ON_UPDATE_COMMAND_UI(ID_APP_ADVANCETIME, &C3EditFrame::OnUpdateAppAdvanceTime)
	ON_COMMAND(ID_APP_ADVANCETIME, &C3EditFrame::OnAppAdvanceTime)
	ON_UPDATE_COMMAND_UI(ID_APP_EDITORDRAW, &C3EditFrame::OnUpdateAppEditorDraw)
	ON_COMMAND(ID_APP_EDITORDRAW, &C3EditFrame::OnAppEditorDraw)
	ON_COMMAND(ID_DOCK_DEBUG, &C3EditFrame::OnDockDebug)
	ON_UPDATE_COMMAND_UI(ID_DOCK_DEBUG, &C3EditFrame::OnUpdateDockDebug)
	ON_COMMAND(ID_DOCK_OBJECTS, &C3EditFrame::OnDockObjects)
	ON_UPDATE_COMMAND_UI(ID_DOCK_OBJECTS, &C3EditFrame::OnUpdateDockObjects)
	ON_COMMAND(ID_DOCK_PROPERTIES, &C3EditFrame::OnDockProperties)
	ON_UPDATE_COMMAND_UI(ID_DOCK_PROPERTIES, &C3EditFrame::OnUpdateDockProperties)
	ON_COMMAND(ID_DOCK_PROTOTYPES, &C3EditFrame::OnDockPrototypes)
	ON_UPDATE_COMMAND_UI(ID_DOCK_PROTOTYPES, &C3EditFrame::OnUpdateDockPrototypes)
	ON_COMMAND(ID_DOCK_RESOURCES, &C3EditFrame::OnDockResources)
	ON_UPDATE_COMMAND_UI(ID_DOCK_RESOURCES, &C3EditFrame::OnUpdateDockResources)
	ON_COMMAND(ID_DOCK_SCRIPTS, &C3EditFrame::OnDockScripts)
	ON_UPDATE_COMMAND_UI(ID_DOCK_SCRIPTS, &C3EditFrame::OnUpdateDockScripts)
	ON_COMMAND(ID_HELP_SDKDOCS, &C3EditFrame::OnHelpSdkDocs)
	ON_UPDATE_COMMAND_UI(ID_HELP_SDKDOCS, &C3EditFrame::OnUpdateHelpSdkDocs)
	ON_COMMAND(ID_HELP_GENCOMPONENT, &C3EditFrame::OnHelpGenComponent)
	ON_UPDATE_COMMAND_UI(ID_HELP_GENCOMPONENT, &C3EditFrame::OnUpdateHelpGenComponent)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PACKFILEMANAGEMENT, &C3EditFrame::OnUpdateEditPackfileManagement)
	ON_COMMAND(ID_EDIT_PACKFILEMANAGEMENT, &C3EditFrame::OnEditPackfileManagement)
	ON_COMMAND(ID_RUN, &C3EditFrame::OnRun)
	ON_UPDATE_COMMAND_UI(ID_RUN, &C3EditFrame::OnUpdateRun)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// C3EditFrame construction/destruction

C3EditFrame::C3EditFrame() noexcept
{
	// TODO: add member initialization code here
}

C3EditFrame::~C3EditFrame()
{
}

int C3EditFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	GetGlobalData()->EnableRibbonImageScale();

	// ****** standard toolbar

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, AFX_DEFAULT_TOOLBAR_STYLE, CRect(1,1,1,1), IDR_MAINFRAME_256) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME_256, 0, 0, TRUE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	// ****** axes toolbar

	if (!m_wndAxesToolBar.CreateEx(this, TBSTYLE_FLAT, AFX_DEFAULT_TOOLBAR_STYLE, CRect(1,1,1,1), IDR_TOOLBAR_AXES) ||
		!m_wndAxesToolBar.LoadToolBar(IDR_TOOLBAR_AXES, 0, 0, TRUE))
	{
		TRACE0("Failed to create axes toolbar\n");
		return -1;      // fail to create
	}

	CString strAxesToolBarName;
	bNameValid = strAxesToolBarName.LoadString(IDS_TOOLBAR_AXES);
	ASSERT(bNameValid);
	m_wndAxesToolBar.SetWindowText(strAxesToolBarName);

	// ****** 3D tools toolbar

	if (!m_wnd3DToolBar.CreateEx(this, TBSTYLE_FLAT, AFX_DEFAULT_TOOLBAR_STYLE, CRect(1,1,1,1), IDR_TOOLBAR_3DTOOLS) ||
		!m_wnd3DToolBar.LoadToolBar(IDR_TOOLBAR_3DTOOLS, 0, 0, TRUE))
	{
		TRACE0("Failed to create 3d tools toolbar\n");
		return -1;      // fail to create
	}

	CString str3DToolBarName;
	bNameValid = str3DToolBarName.LoadString(IDS_TOOLBAR_3DTOOLS);
	ASSERT(bNameValid);
	m_wnd3DToolBar.SetWindowText(str3DToolBarName);

	// ****** scripting toolbar

	if (!m_wndScriptingToolBar.CreateEx(this, TBSTYLE_FLAT, AFX_DEFAULT_TOOLBAR_STYLE, CRect(1,1,1,1), IDR_TOOLBAR_SCRIPTING) ||
		!m_wndScriptingToolBar.LoadToolBar(IDR_TOOLBAR_SCRIPTING, 0, 0, TRUE))
	{
		TRACE0("Failed to create scripting toolbar\n");
		return -1;      // fail to create
	}

	CString strScriptingToolBarName;
	bNameValid = strScriptingToolBarName.LoadString(IDS_TOOLBAR_SCRIPTING);
	ASSERT(bNameValid);
	m_wndScriptingToolBar.SetWindowText(strScriptingToolBarName);

	// ****** status bar

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndAxesToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wnd3DToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndScriptingToolBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndAxesToolBar);
	DockPane(&m_wnd3DToolBar);
	DockPane(&m_wndScriptingToolBar);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Navigation pane will be created at left, so temporary disable docking at the left side:
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);

	// Create a caption bar:
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, IDB_MENU_IMAGES_24);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	m_wndProtoView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProtoView);

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);

	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	m_wndScripting.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndScripting);

	m_wndObjects.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndObjects);

	m_wndResourceSpy.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndResourceSpy);

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	// set the visual style to be used the by the visual manager
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

	// enable quick (Alt+drag) toolbar customization
	//CMFCToolBar::EnableQuickCustomization();

	//RegisterScaleChangeEvent([in]  HANDLE    hEvent, [out] DWORD_PTR *pdwCookie);

	return 0;
}

BOOL C3EditFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	return m_wndSplitter.Create(this,
		2, 2,               // TODO: adjust the number of rows, columns
		CSize(10, 10),      // TODO: adjust the minimum pane size
		pContext);
}

BOOL C3EditFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL C3EditFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create prototypes view
	CString strProtoView;
	bNameValid = strProtoView.LoadString(IDS_PROTOTYPE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndProtoView.Create(strProtoView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROTOVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Prototype View window\n");
		return FALSE; // failed to create
	}

	// Create output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	// Create properties window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	// Create scripting window
	CString strScriptingWnd;
	bNameValid = strScriptingWnd.LoadString(IDS_SCRIPTING_WND);
	ASSERT(bNameValid);
	if (!m_wndScripting.Create(strScriptingWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_SCRIPTINGWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Scripting window\n");
		return FALSE; // failed to create
	}

	// Create scripting window
	CString strResourceSpyWnd;
	bNameValid = strResourceSpyWnd.LoadString(IDS_RESOURCESPY_WND);
	ASSERT(bNameValid);
	if (!m_wndResourceSpy.Create(strResourceSpyWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_RESOURCESPYWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Resource Spy window\n");
		return FALSE; // failed to create
	}

	// Create objects window
	CString strObjectsWnd;
	bNameValid = strObjectsWnd.LoadString(IDS_OBJECTS_WND);
	ASSERT(bNameValid);
	if (!m_wndObjects.Create(strObjectsWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_OBJECTSWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Objects window\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(TRUE);
	return TRUE;
}

void C3EditFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hProtoViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProtoView.SetIcon(hProtoViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	HICON hScriptingBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_SCRIPTING_WND_HC : IDI_SCRIPTING_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndScripting.SetIcon(hScriptingBarIcon, FALSE);

	HICON hResourceSpyBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_RESOURCESPY_WND_HC : IDI_RESOURCESPY_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndResourceSpy.SetIcon(hResourceSpyBarIcon, FALSE);

	HICON hObjectsBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OBJECTS_WND_HC : IDI_OBJECTS_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndObjects.SetIcon(hObjectsBarIcon, FALSE);
}


BOOL C3EditFrame::CreateCaptionBar()
{
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return FALSE;
	}

	BOOL bNameValid;

	CString strTemp, strTemp2;
	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButtonToolTip(strTemp);

	bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	return TRUE;
}

// C3EditFrame diagnostics

#ifdef _DEBUG
void C3EditFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void C3EditFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// C3EditFrame message handlers

void C3EditFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->Create();
}

LRESULT C3EditFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void C3EditFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

void C3EditFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

void C3EditFrame::OnOptions()
{
}

void C3EditFrame::OnViewProtoView()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndProtoView.ShowPane(TRUE, FALSE, TRUE);
	m_wndProtoView.SetFocus();
}

void C3EditFrame::OnUpdateViewProtoView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void C3EditFrame::OnViewOutputWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndOutput.ShowPane(TRUE, FALSE, TRUE);
	m_wndOutput.SetFocus();
}

void C3EditFrame::OnUpdateViewOutputWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void C3EditFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndProperties.ShowPane(TRUE, FALSE, TRUE);
	m_wndProperties.SetFocus();
}

void C3EditFrame::OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void C3EditFrame::OnViewScriptingWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndScripting.ShowPane(TRUE, FALSE, TRUE);
	m_wndScripting.SetFocus();
}

void C3EditFrame::OnUpdateViewScriptingWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void C3EditFrame::OnViewResourceSpyWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndResourceSpy.ShowPane(TRUE, FALSE, TRUE);
	m_wndResourceSpy.SetFocus();
}

void C3EditFrame::OnUpdateViewResourceSpyWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void C3EditFrame::OnViewObjectsWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndObjects.ShowPane(TRUE, FALSE, TRUE);
	m_wndObjects.SetFocus();
}

void C3EditFrame::OnUpdateViewObjectsWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


BOOL C3EditFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	CWinApp* pApp = AfxGetApp();
	if (pApp->m_pMainWnd == nullptr)
		pApp->m_pMainWnd = this;

	return TRUE;
}

void C3EditFrame::OnFileClose()
{
	DestroyWindow();
}

void C3EditFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}


void C3EditFrame::SetActiveTool(C3EditApp::ToolType toolid)
{
	UINT32 i = 0;
	CMFCToolBarButton *pb = NULL;
	while ((pb = m_wnd3DToolBar.GetButton(i)) != NULL)
	{
		UINT bstyle = m_wnd3DToolBar.GetButtonStyle(i);
		bstyle &= ~TBBS_CHECKED;
		m_wnd3DToolBar.SetButtonStyle(i, bstyle);

		i++;
	}
	theApp.m_Config->SetInt(_T("environment.active.tool"), toolid);
}


void C3EditFrame::IncludeAxis(props::TFlags64 axis)
{
	props::TFlags64 tmp = GetAxes();

	if (tmp.IsSet(C3EditApp::AT_X | C3EditApp::AT_Y) && axis.IsSet(C3EditApp::AT_Z))
	{
		tmp.Clear(C3EditApp::AT_X | C3EditApp::AT_Y);
		tmp.Set(C3EditApp::AT_Z);
	}
	else if (tmp.IsSet(C3EditApp::AT_X | C3EditApp::AT_Z) && axis.IsSet(C3EditApp::AT_Y))
	{
		tmp.Clear(C3EditApp::AT_X | C3EditApp::AT_Z);
		tmp.Set(C3EditApp::AT_Y);
	}
	if (tmp.IsSet(C3EditApp::AT_Z | C3EditApp::AT_Y) && axis.IsSet(C3EditApp::AT_X))
	{
		tmp.Clear(C3EditApp::AT_Z | C3EditApp::AT_Y);
		tmp.Set(C3EditApp::AT_X);
	}
	else
	{
		tmp |= axis;
	}

	SetAxes(tmp);
}


void C3EditFrame::ExcludeAxis(props::TFlags64 axis)
{
	props::TFlags64 tmp = GetAxes();

	tmp.Clear(axis);

	if (!tmp.AnySet(C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_Z))
	{
		tmp.Set(axis);
	}

	SetAxes(tmp);
}


void C3EditFrame::SetAxes(props::TFlags64 axes)
{
	uint64_t i = 0;
	CMFCToolBarButton *pb = NULL;
	while ((pb = m_wndAxesToolBar.GetButton((int)i)) != NULL)
	{
		UINT bstyle = m_wndAxesToolBar.GetButtonStyle((int)i);
		bstyle &= ~TBBS_CHECKED;
		m_wndAxesToolBar.SetButtonStyle((int)i, bstyle);

		i++;
	}

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			theApp.m_Config->SetInt(_T("environment.active.axis.translation"), axes);
			break;

		case C3EditApp::TT_ROTATE:
			theApp.m_Config->SetInt(_T("environment.active.axis.rotation"), axes);
			break;

		case C3EditApp::TT_SCALE:
			theApp.m_Config->SetInt(_T("environment.active.axis.scale"), axes);
			break;
	}
}


props::TFlags64 C3EditFrame::GetAxes()
{
	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			return theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);

		case C3EditApp::TT_ROTATE:
			return theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);

		case C3EditApp::TT_SCALE:
			return theApp.m_Config->GetInt(_T("environment.active.axis.scale"), C3EditApp::AT_X | C3EditApp::AT_Y);
	}

	return 0;
}


BOOL C3EditFrame::AxisActive(props::TFlags64 axis)
{
	return axis.IsSet(GetAxes());
}


CMFCToolBarButton *C3EditFrame::GetToolButtonByID(CMFCToolBar *toolbar, DWORD butid)
{
	UINT32 i = 0;
	CMFCToolBarButton *pb = NULL;
	while ((pb = toolbar->GetButton(i)) != NULL)
	{
		if (pb->m_nID == butid)
			return pb;

		i++;
	}

	return NULL;
}


void C3EditFrame::OnUpdateToolSelect(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_SELECT);
}

void C3EditFrame::OnUpdateToolTranslate(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_TRANSLATE);
}

void C3EditFrame::OnUpdateToolRotate(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_ROTATE);
}

void C3EditFrame::OnUpdateToolScale(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_SCALE);
}

void C3EditFrame::OnUpdateToolUniscale(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_UNISCALE);
}

void C3EditFrame::OnUpdateToolWand(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT) == C3EditApp::TT_WAND);
}

void C3EditFrame::OnToolSelect()
{
	SetActiveTool(C3EditApp::TT_SELECT);
}

void C3EditFrame::OnToolTranslate()
{
	SetActiveTool(C3EditApp::TT_TRANSLATE);
}

void C3EditFrame::OnToolRotate()
{
	SetActiveTool(C3EditApp::TT_ROTATE);
}

void C3EditFrame::OnToolScale()
{
	SetActiveTool(C3EditApp::TT_SCALE);
}

void C3EditFrame::OnToolUniscale()
{
	SetActiveTool(C3EditApp::TT_UNISCALE);
}

void C3EditFrame::OnToolWand()
{
	SetActiveTool(C3EditApp::TT_WAND);
}



void C3EditFrame::OnUpdateAxisScreenrel(CCmdUI *pCmdUI)
{
	int64_t on = 0;
	pCmdUI->Enable(0);

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_ROTATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
			pCmdUI->Enable();
			break;
	}

	pCmdUI->SetRadio(on & C3EditApp::AT_SCREENREL);
}

void C3EditFrame::OnUpdateAxisX(CCmdUI *pCmdUI)
{
	int64_t on = 0;
	pCmdUI->Enable(0);

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_ROTATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_SCALE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.scale"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;
	}

	pCmdUI->SetRadio(on & C3EditApp::AT_X);
}

void C3EditFrame::OnUpdateAxisY(CCmdUI *pCmdUI)
{
	int64_t on = 0;
	pCmdUI->Enable(0);

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_ROTATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_SCALE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.scale"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;
	}

	pCmdUI->SetRadio(on & C3EditApp::AT_Y);
}

void C3EditFrame::OnUpdateAxisZ(CCmdUI *pCmdUI)
{
	int64_t on = 0;
	pCmdUI->Enable(0);

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_ROTATE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
			pCmdUI->Enable();
			break;

		case C3EditApp::TT_SCALE:
			on = theApp.m_Config->GetInt(_T("environment.active.axis.scale"), C3EditApp::AT_X | C3EditApp::AT_Y);
			pCmdUI->Enable();
			break;
	}

	pCmdUI->SetRadio(on & C3EditApp::AT_Z);
}

void C3EditFrame::OnAxisScreenrel()
{
	props::TFlags64 tmp = 0;

	C3EditApp::ToolType tt = (C3EditApp::ToolType)theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	switch (tt)
	{
		case C3EditApp::TT_TRANSLATE:
			tmp = theApp.m_Config->GetInt(_T("environment.active.axis.translation"), C3EditApp::AT_X | C3EditApp::AT_Y);
			tmp.Toggle(C3EditApp::AT_SCREENREL);
			theApp.m_Config->SetInt(_T("environment.active.axis.translation"), tmp);
			break;

		case C3EditApp::TT_ROTATE:
			tmp = theApp.m_Config->GetInt(_T("environment.active.axis.rotation"), C3EditApp::AT_Z);
			tmp.Toggle(C3EditApp::AT_SCREENREL);
			theApp.m_Config->SetInt(_T("environment.active.axis.rotation"), tmp);
			break;
	}
}

void C3EditFrame::OnAxisX()
{
	if (!AxisActive(C3EditApp::AT_X))
	{
		IncludeAxis(C3EditApp::AT_X);
	}
	else
	{
		ExcludeAxis(C3EditApp::AT_X);
	}
}

void C3EditFrame::OnAxisY()
{
	if (!AxisActive(C3EditApp::AT_Y))
	{
		IncludeAxis(C3EditApp::AT_Y);
	}
	else
	{
		ExcludeAxis(C3EditApp::AT_Y);
	}
}

void C3EditFrame::OnAxisZ()
{
	if (!AxisActive(C3EditApp::AT_Z))
	{
		IncludeAxis(C3EditApp::AT_Z);
	}
	else
	{
		ExcludeAxis(C3EditApp::AT_Z);
	}
}

void C3EditFrame::SetActiveProperties(props::IPropertySet *props, bool readonly, const TCHAR *title)
{
	if (m_wndProperties.GetSafeHwnd())
		m_wndProperties.SetActiveProperties(props, readonly, title);
}

void C3EditFrame::SetActivePrototype(c3::Prototype *pproto, bool readonly, const TCHAR *title)
{
	int64_t active_tool = theApp.m_Config->GetInt(_T("environment.active.tool"), C3EditApp::TT_SELECT);
	if (active_tool == C3EditApp::ToolType::TT_WAND)
	{
		C3EditDoc *pdoc = (C3EditDoc *)GetActiveDocument();
		if (pdoc)
			pdoc->SetBrush(pproto);
	}

	if (m_wndProperties.GetSafeHwnd())
		m_wndProperties.SetActivePrototype(pproto);
}

void C3EditFrame::SetActiveObject(c3::Object *pobj, bool readonly, const TCHAR *title)
{
	if (m_wndProperties.GetSafeHwnd())
		m_wndProperties.SetActiveObject(pobj);
}

void C3EditFrame::RefreshActiveProperties()
{
	if (m_wndProperties.GetSafeHwnd())
		m_wndProperties.UpdateCurrentProperties();
}

void C3EditFrame::UpdateObjectList()
{
	m_wndObjects.UpdateContents();
}

void C3EditFrame::OnUpdateAssignScript(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_wndScripting.ImmediateScriptEmpty());
}

void C3EditFrame::OnAssignScript()
{

}

void C3EditFrame::OnUpdateRunScript(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_wndScripting.ImmediateScriptEmpty());
}

void C3EditFrame::OnRunScript()
{
	C3EditView *pSceneView = dynamic_cast<C3EditView *>(GetActiveView());
	if (pSceneView)
	{
		C3EditDoc *pdoc = pSceneView->GetDocument();

		CString s;
		m_wndScripting.GetImmediateScript(s);

		size_t maxi = pdoc->GetNumSelected();
		if (!maxi)
		{
			c3::Scriptable *psc = (c3::Scriptable *)pSceneView->GetDocument()->m_RootObj->FindComponent(c3::Scriptable::Type());
			if (psc)
				psc->Execute((LPCTSTR)s);

			return;
		}

		pdoc->DoForAllSelected([&](c3::Object *pobj)
		{
			c3::Scriptable *psc = (c3::Scriptable *)pobj->FindComponent(c3::Scriptable::Type());
			if (psc)
				psc->Execute((LPCTSTR)s);
		});
	}
}

void C3EditFrame::OnUpdateUpdateScript(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_wndScripting.ResourceScriptChanged());
}

void C3EditFrame::OnUpdateScript()
{
	m_wndScripting.UpdateResourceScript();

	C3EditView *pSceneView = dynamic_cast<C3EditView *>(GetActiveView());
	if (pSceneView)
	{
		C3EditDoc *pdoc = pSceneView->GetDocument();

		pdoc->DoForAllSelected([&](c3::Object *pobj)
		{
			props::IProperty *pp = pobj->GetProperties()->GetPropertyById('SRCF');
			if (pp)
				pobj->PropertyChanged(pp);
		});
	}
}

void C3EditFrame::OnMove(int x, int y)
{
	CFrameWndEx::OnMove(x, y);

	HMONITOR hmon = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);

	DEVICE_SCALE_FACTOR sf;
	if (SUCCEEDED(GetScaleFactorForMonitor(hmon, &sf)))
	{
		switch (sf)
		{
			default:
			case SCALE_100_PERCENT: m_Scale = 1.0f; break;
			case SCALE_120_PERCENT: m_Scale = 1.2f; break;
			case SCALE_125_PERCENT: m_Scale = 1.25f; break;
			case SCALE_140_PERCENT: m_Scale = 1.4f; break;
			case SCALE_150_PERCENT: m_Scale = 1.5f; break;
			case SCALE_160_PERCENT: m_Scale = 1.6f; break;
			case SCALE_175_PERCENT: m_Scale = 1.75f; break;
			case SCALE_180_PERCENT: m_Scale = 1.8f; break;
			case SCALE_200_PERCENT: m_Scale = 2.0f; break;
			case SCALE_225_PERCENT: m_Scale = 2.25f; break;
			case SCALE_250_PERCENT: m_Scale = 2.5f; break;
			case SCALE_300_PERCENT: m_Scale = 3.0f; break;
			case SCALE_350_PERCENT: m_Scale = 3.5f; break;
			case SCALE_400_PERCENT: m_Scale = 4.0f; break;
			case SCALE_450_PERCENT: m_Scale = 4.5f; break;
			case SCALE_500_PERCENT: m_Scale = 5.0f; break;
		}
	}
}


void C3EditFrame::OnClose()
{
	theApp.m_C3->GetLog()->SetRedirectFunction(nullptr, nullptr);
	//UnregisterScaleChangeEvent([in]  HANDLE    hEvent, [out] DWORD_PTR *pdwCookie);

	CFrameWndEx::OnClose();
}


void C3EditFrame::OnEditResourcePathConfiguration()
{
	CResourcePathEditorDlg dlg;
	dlg.DoModal();
}


void C3EditFrame::OnUpdateAppAdvanceTime(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();

	bool b = theApp.m_Config->GetBool(_T("environment.advancetime"), true);
	pCmdUI->SetCheck(b);
}


void C3EditFrame::OnAppAdvanceTime()
{
	bool b = theApp.m_Config->GetBool(_T("environment.advancetime"), true);
	theApp.m_Config->SetBool(_T("environment.advancetime"), !b);
}


void C3EditFrame::OnUpdateAppEditorDraw(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();

	bool b = theApp.m_Config->GetBool(_T("environment.editordraw"), true);
	pCmdUI->SetCheck(b);
}


void C3EditFrame::OnAppEditorDraw()
{
	bool b = theApp.m_Config->GetBool(_T("environment.editordraw"), true);
	theApp.m_Config->SetBool(_T("environment.editordraw"), !b);
}


void C3EditFrame::OnDockDebug()
{
	m_wndOutput.ShowPane(!m_wndOutput.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockDebug(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndOutput.IsPaneVisible());
}


void C3EditFrame::OnDockObjects()
{
	m_wndObjects.ShowPane(!m_wndObjects.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockObjects(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndObjects.IsPaneVisible());
}


void C3EditFrame::OnDockProperties()
{
	m_wndProperties.ShowPane(!m_wndProperties.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockProperties(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndProperties.IsPaneVisible());
}


void C3EditFrame::OnDockPrototypes()
{
	m_wndProtoView.ShowPane(!m_wndProtoView.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockPrototypes(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndProtoView.IsPaneVisible());
}


void C3EditFrame::OnDockResources()
{
	m_wndResourceSpy.ShowPane(!m_wndResourceSpy.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockResources(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndResourceSpy.IsPaneVisible());
}


void C3EditFrame::OnDockScripts()
{
	m_wndScripting.ShowPane(!m_wndScripting.IsPaneVisible(), FALSE, FALSE);
}


void C3EditFrame::OnUpdateDockScripts(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_wndScripting.IsPaneVisible());
}


void C3EditFrame::OnHelpSdkDocs()
{
	ShellExecute(NULL, _T("open"), _T("..\\Docs\\html\\index.html"), NULL, NULL, SW_SHOW);
}


void C3EditFrame::OnUpdateHelpSdkDocs(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void C3EditFrame::OnHelpGenComponent()
{
	ShellExecute(NULL, _T("open"), _T(".\\C3ComponentGenerator.exe"), NULL, NULL, SW_SHOW);
}


void C3EditFrame::OnUpdateHelpGenComponent(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}



void C3EditFrame::OnUpdateEditPackfileManagement(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void C3EditFrame::OnEditPackfileManagement()
{
	CPackfileManager pfm;
	pfm.DoModal();
}


void C3EditFrame::OnRun()
{
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	TCHAR command[MAX_PATH * 2];
#if defined(_DEBUG)
	_stprintf_s(command, _T("C3App64D.exe d:\\proj\\Celerity\\Samples\\Stuff\\RacerApp.c3js"), (LPCTSTR)GetActiveDocument()->GetPathName());
#else
	_stprintf_s(command, _T("C3App64.exe d:\\proj\\Celerity\\Samples\\Stuff\\RacerApp.c3js"), (LPCTSTR)GetActiveDocument()->GetPathName());
#endif

	BOOL created = CreateProcess(NULL, command, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
}


void C3EditFrame::OnUpdateRun(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}
