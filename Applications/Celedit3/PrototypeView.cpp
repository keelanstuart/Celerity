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

#include "pch.h"
#include "framework.h"
#include "MainFrm.h"
#include "PrototypeView.h"
#include "Resource.h"
#include "Celedit3.h"

class CPrototypeViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CPrototypeView;

	DECLARE_SERIAL(CPrototypeViewMenuButton)

public:
	CPrototypeViewMenuButton(HMENU hMenu = nullptr) noexcept : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CPrototypeViewMenuButton, CMFCToolBarMenuButton, 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPrototypeView::CPrototypeView() noexcept
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CPrototypeView::~CPrototypeView()
{
}

BEGIN_MESSAGE_MAP(CPrototypeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrototypeView message handlers

int CPrototypeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndPrototypeView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	// Load images:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CPrototypeViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CPrototypeViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CPrototypeViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != nullptr)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillClassView();

	return 0;
}

void CPrototypeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPrototypeView::FillClassView()
{
	HTREEITEM hRoot = m_wndPrototypeView.InsertItem(_T("FakeApp classes"), 0, 0);
	m_wndPrototypeView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hClass = m_wndPrototypeView.InsertItem(_T("CFakeAboutDlg"), 1, 1, hRoot);
	m_wndPrototypeView.InsertItem(_T("CFakeAboutDlg()"), 3, 3, hClass);

	m_wndPrototypeView.Expand(hRoot, TVE_EXPAND);

	hClass = m_wndPrototypeView.InsertItem(_T("CFakeApp"), 1, 1, hRoot);
	m_wndPrototypeView.InsertItem(_T("CFakeApp()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("InitInstance()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("OnAppAbout()"), 3, 3, hClass);

	hClass = m_wndPrototypeView.InsertItem(_T("CFakeAppDoc"), 1, 1, hRoot);
	m_wndPrototypeView.InsertItem(_T("CFakeAppDoc()"), 4, 4, hClass);
	m_wndPrototypeView.InsertItem(_T("~CFakeAppDoc()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("OnNewDocument()"), 3, 3, hClass);

	hClass = m_wndPrototypeView.InsertItem(_T("CFakeAppView"), 1, 1, hRoot);
	m_wndPrototypeView.InsertItem(_T("CFakeAppView()"), 4, 4, hClass);
	m_wndPrototypeView.InsertItem(_T("~CFakeAppView()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("GetDocument()"), 3, 3, hClass);
	m_wndPrototypeView.Expand(hClass, TVE_EXPAND);

	hClass = m_wndPrototypeView.InsertItem(_T("CFakeAppFrame"), 1, 1, hRoot);
	m_wndPrototypeView.InsertItem(_T("CFakeAppFrame()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("~CFakeAppFrame()"), 3, 3, hClass);
	m_wndPrototypeView.InsertItem(_T("m_wndMenuBar"), 6, 6, hClass);
	m_wndPrototypeView.InsertItem(_T("m_wndToolBar"), 6, 6, hClass);
	m_wndPrototypeView.InsertItem(_T("m_wndStatusBar"), 6, 6, hClass);

	hClass = m_wndPrototypeView.InsertItem(_T("Globals"), 2, 2, hRoot);
	m_wndPrototypeView.InsertItem(_T("theFakeApp"), 5, 5, hClass);
	m_wndPrototypeView.Expand(hClass, TVE_EXPAND);
}

void CPrototypeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CPrototypeView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPrototypeView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CPrototypeView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CPrototypeView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CPrototypeViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CPrototypeViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != nullptr)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CPrototypeView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CPrototypeView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("Add member function..."));
}

void CPrototypeView::OnClassAddMemberVariable()
{
	// TODO: Add your command handler code here
}

void CPrototypeView::OnClassDefinition()
{
	// TODO: Add your command handler code here
}

void CPrototypeView::OnClassProperties()
{
	// TODO: Add your command handler code here
}

void CPrototypeView::OnNewFolder()
{
	AfxMessageBox(_T("New Folder..."));
}

void CPrototypeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndPrototypeView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CPrototypeView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndPrototypeView.SetFocus();
}

void CPrototypeView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndPrototypeView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);
}
