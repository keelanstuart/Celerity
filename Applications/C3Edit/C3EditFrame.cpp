
// C3EditFrame.cpp : implementation of the C3EditFrame class
//

#include "pch.h"
#include "framework.h"

#include "C3EditFrame.h"

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

	ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &C3EditFrame::OnUpdateViewCaptionBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROTOVIEW, &C3EditFrame::OnUpdateViewProtoView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &C3EditFrame::OnUpdateViewOutputWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIESWND, &C3EditFrame::OnUpdateViewPropertiesWindow)

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

	// ****** standard toolbar

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME_256))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	// ****** axes toolbar

	if (!m_wndAxesToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndAxesToolBar.LoadToolBar(IDR_TOOLBAR_AXES))
	{
		TRACE0("Failed to create axes toolbar\n");
		return -1;      // fail to create
	}

	CString strAxesToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_AXES);
	ASSERT(bNameValid);
	m_wndAxesToolBar.SetWindowText(strAxesToolBarName);

	// ****** 3D tools toolbar

	if (!m_wnd3DToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wnd3DToolBar.LoadToolBar(IDR_TOOLBAR_3DTOOLS))
	{
		TRACE0("Failed to create 3d tools toolbar\n");
		return -1;      // fail to create
	}

	CString str3DToolBarName;
	bNameValid = str3DToolBarName.LoadString(IDS_TOOLBAR_3DTOOLS);
	ASSERT(bNameValid);
	m_wnd3DToolBar.SetWindowText(str3DToolBarName);

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
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndAxesToolBar);
	DockPane(&m_wnd3DToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Navigation pane will be created at left, so temporary disable docking at the left side:
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);

	// Create and setup "Outlook" navigation bar:
	if (!CreateOutlookBar(m_wndNavigationBar, ID_VIEW_NAVIGATION, m_wndTree, m_wndCalendar, 250))
	{
		TRACE0("Failed to create navigation pane\n");
		return -1;      // fail to create
	}

	// Create a caption bar:
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	// Outlook bar is created and docking on the left side should be allowed.
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

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


	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

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

}

BOOL C3EditFrame::CreateOutlookBar(CMFCOutlookBar& bar, UINT uiID, CMFCShellTreeCtrl& tree, CCalendarBar& calendar, int nInitialWidth)
{
	bar.SetMode2003();

	BOOL bNameValid;
	CString strTemp;
	bNameValid = strTemp.LoadString(IDS_SHORTCUTS);
	ASSERT(bNameValid);
	if (!bar.Create(strTemp, this, CRect(0, 0, nInitialWidth, 32000), uiID, WS_CHILD | WS_VISIBLE | CBRS_LEFT))
	{
		return FALSE; // fail to create
	}

	CMFCOutlookBarTabCtrl* pOutlookBar = (CMFCOutlookBarTabCtrl*)bar.GetUnderlyingWindow();

	if (pOutlookBar == nullptr)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pOutlookBar->EnableInPlaceEdit(TRUE);

	static UINT uiPageID = 1;

	// can float, can autohide, can resize, CAN NOT CLOSE
	DWORD dwStyle = AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE | AFX_CBRS_RESIZE;

	CRect rectDummy(0, 0, 0, 0);
	const DWORD dwTreeStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	tree.Create(dwTreeStyle, rectDummy, &bar, 1200);
	bNameValid = strTemp.LoadString(IDS_FOLDERS);
	ASSERT(bNameValid);
	pOutlookBar->AddControl(&tree, strTemp, 2, TRUE, dwStyle);

	calendar.Create(rectDummy, &bar, 1201);
	bNameValid = strTemp.LoadString(IDS_CALENDAR);
	ASSERT(bNameValid);
	pOutlookBar->AddControl(&calendar, strTemp, 3, TRUE, dwStyle);

	bar.SetPaneStyle(bar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	pOutlookBar->SetImageList(IDB_PAGES_HC, 24);
	pOutlookBar->SetToolbarImageList(IDB_PAGES_SMALL_HC, 16);
	pOutlookBar->RecalcLayout();

	BOOL bAnimation = theApp.GetInt(_T("OutlookAnimation"), TRUE);
	CMFCOutlookBarTabCtrl::EnableAnimation(bAnimation);

	bar.SetButtonsFont(&afxGlobalData.fontBold);

	return TRUE;
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

	theApp.m_Config->SetInt(_T("environment.active.axis"), axes);
}


props::TFlags64 C3EditFrame::GetAxes()
{
	return theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL);
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
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL) & C3EditApp::AT_SCREENREL);
}

void C3EditFrame::OnUpdateAxisX(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL) & C3EditApp::AT_X);
}

void C3EditFrame::OnUpdateAxisY(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL) & C3EditApp::AT_Y);
}

void C3EditFrame::OnUpdateAxisZ(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL) & C3EditApp::AT_Z);
}

void C3EditFrame::OnAxisScreenrel()
{
	props::TFlags64 tmp = theApp.m_Config->GetInt(_T("environment.active.axis"), C3EditApp::AT_X | C3EditApp::AT_Y | C3EditApp::AT_SCREENREL);
	tmp.Toggle(C3EditApp::AT_SCREENREL);
	theApp.m_Config->SetInt(_T("environment.active.axis"), tmp);
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