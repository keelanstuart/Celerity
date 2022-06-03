// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Prototypees Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "pch.h"
#include "framework.h"
#include "C3EditFrame.h"
#include "PrototypeView.h"
#include "Resource.h"
#include "C3Edit.h"
//#include "PrototypeEditorDlg.h"

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

#define PROTOTREE_ID		2

BEGIN_MESSAGE_MAP(CPrototypeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnPrototypeAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnPrototypeAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnPrototypeDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnPrototypeProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
	ON_NOTIFY(TVN_SELCHANGED, PROTOTREE_ID, OnSelectionChanged)
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

	if (!m_wndPrototypeView.Create(dwViewStyle, rectDummy, this, PROTOTREE_ID))
	{
		TRACE0("Failed to create Prototype View\n");
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
	FillPrototypeView();

	return 0;
}

void CPrototypeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

HTREEITEM CPrototypeView::FindChildItem(HTREEITEM hroot, const TCHAR *itemname)
{
	HTREEITEM hret = m_wndPrototypeView.GetNextItem(hroot, TVGN_CHILD);
	while (hret)
	{
		if (!_tcsicmp(m_wndPrototypeView.GetItemText(hret), itemname))
			break;

		hret = m_wndPrototypeView.GetNextItem(hret, TVGN_NEXT);
	}

	return hret;
}

HTREEITEM CPrototypeView::MakeProtoGroup(HTREEITEM hroot, const TCHAR *group)
{
	if (!group || !*group)
	{
		return hroot;
	}

	HTREEITEM hcurr = hroot;

	CString gname;
	while (*group && (*group != '/') && (*group != '\\'))
	{
		gname += *group;
		if (*group)
			group++;
	}

	if (!gname.IsEmpty())
	{
		if (*group)
			group++;

		HTREEITEM hexisting = FindChildItem(hcurr, gname);
		hcurr = hexisting ? hexisting : m_wndPrototypeView.InsertItem(gname, 0, 0, hcurr);

		HTREEITEM hnext = MakeProtoGroup(hcurr, group);

		m_wndPrototypeView.Expand(hcurr, TVE_EXPAND);

		hcurr = hnext;
	}

	return hcurr;
}
void CPrototypeView::FillPrototypeView()
{
	HTREEITEM hRoot = m_wndPrototypeView.InsertItem(_T("All Prototypes"), 0, 0);
	m_wndPrototypeView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	c3::Factory *pfac = theApp.m_C3->GetFactory();
	for (size_t i = 0, maxi = pfac->GetNumPrototypes(); i < maxi; i++)
	{
		c3::Prototype *pproto = pfac->GetPrototype(i);
		assert(pproto);

		HTREEITEM hgroup = MakeProtoGroup(hRoot, pproto->GetGroup());

		CString tmp = pproto->GetName();
		HTREEITEM hitem = m_wndPrototypeView.InsertItem(tmp, 1, 1, hgroup);
		m_wndPrototypeView.SetItemData(hitem, (DWORD_PTR)pproto);
	}

	m_wndPrototypeView.Expand(hRoot, TVE_EXPAND);
}

void MyAppendMenuItem(HMENU menu, UINT32 type, const TCHAR *text = NULL, BOOL enabled = true, HMENU submenu = NULL, UINT id = -1, DWORD_PTR data = NULL);

void MyAppendMenuItem(HMENU menu, UINT32 type, const TCHAR *text, BOOL enabled, HMENU submenu, UINT id, DWORD_PTR data)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));

	mii.cbSize = sizeof(MENUITEMINFO);

	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA;

	mii.wID = (id == -1) ? GetMenuItemCount(menu) : id;

	mii.fType = type;

	if ((type == MFT_STRING) || text)
	{
		mii.dwTypeData = (LPWSTR)text;
		mii.cch = (UINT)_tcslen(text);
	}

	if (!enabled)
	{
		mii.fMask |= MIIM_STATE;
		mii.fState = MFS_GRAYED;
	}

	if (submenu)
	{
		mii.hSubMenu = submenu;
		mii.fMask |= MIIM_SUBMENU;
	}

	mii.dwItemData = data;

	InsertMenuItem(menu, GetMenuItemCount(menu), true, &mii);
}

enum EPopupCommand
{
	PC_DELETE = 1,
	PC_CREATEPROTO,
	PC_CREATEGROUP,
	PC_EDIT,
	PC_DUPLICATE
};

void CPrototypeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	c3::Factory *pfac = theApp.m_C3->GetFactory();

	HMENU menu = ::CreatePopupMenu();
	HMENU submenu = ::CreatePopupMenu();

	MyAppendMenuItem(submenu, MFT_STRING, _T("Prototype"), true, 0, PC_CREATEPROTO);
	MyAppendMenuItem(submenu, MFT_STRING, _T("Group"), true, 0, PC_CREATEGROUP);

	MyAppendMenuItem(menu, MFT_STRING, _T("New"), TRUE, submenu);
	MyAppendMenuItem(menu, MFT_SEPARATOR);

	if (point != CPoint(-1, -1))
	{
		c3::Prototype *pproto = nullptr;

		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			MyAppendMenuItem(menu, MFT_STRING, _T("Delete"), true, 0, PC_DELETE);

			pWndTree->SelectItem(hTreeItem);
			pproto = (c3::Prototype *)pWndTree->GetItemData(hTreeItem);
			if (pproto)
			{
				MyAppendMenuItem(menu, MFT_STRING, _T("Edit"), true, 0, PC_EDIT);
				MyAppendMenuItem(menu, MFT_STRING, _T("Duplicate"), true, 0, PC_DUPLICATE);
			}
		}

		HTREEITEM hpi = hTreeItem;
		if (!hpi || !pWndTree->GetParentItem(hpi))
			hpi = pWndTree->GetRootItem();
		else
			hpi = pWndTree->GetParentItem(hpi);

		UINT ret = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN, point.x, point.y, 0, GetSafeHwnd(), NULL);
		switch (ret)
		{
			case PC_CREATEPROTO:
			{
				c3::Prototype *pcp = pfac->CreatePrototype();

				TCHAR protoname[256];
				GUID g = pcp->GetGUID();
				_stprintf_s(protoname, _T("proto_%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"), g.Data1, g.Data2, g.Data3,
					g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
				pcp->SetName(protoname);

				tstring groupname;
				if (pproto)
				{
					pcp->SetGroup(pproto->GetGroup());
				}
				else
				{
					HTREEITEM hi = hpi;
					while (hi && pWndTree->GetParentItem(hi))
					{
						if (!groupname.empty())
							groupname.insert(_T('/'), 0);

						groupname = tstring((LPCTSTR)(pWndTree->GetItemText(hi))) + groupname;
						pWndTree->Expand(hi, TVE_EXPAND);

						hi = pWndTree->GetParentItem(hi);
					}
				}

				CString tmp = pcp->GetName();
				HTREEITEM hitem = m_wndPrototypeView.InsertItem(tmp, 1, 1, hpi);
				m_wndPrototypeView.SetItemData(hitem, (DWORD_PTR)pcp);
				break;
			}

			case PC_CREATEGROUP:
			{
				break;
			}

			case PC_EDIT:
			{
				break;
			}

			case PC_DELETE:
			{
				if (pproto)
				{
					pfac->RemovePrototype(pproto);
					pWndTree->DeleteItem(hTreeItem);
				}
				break;
			}
		}
	}

	pWndTree->RedrawWindow();
	pWndTree->SetFocus();

	::DestroyMenu(submenu);
	::DestroyMenu(menu);
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

void CPrototypeView::OnPrototypeAddMemberFunction()
{
	AfxMessageBox(_T("Add member function..."));
}

void CPrototypeView::OnPrototypeAddMemberVariable()
{
	// TODO: Add your command handler code here
}

void CPrototypeView::OnPrototypeDefinition()
{
	// TODO: Add your command handler code here
}

void CPrototypeView::OnPrototypeProperties()
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
	m_PrototypeViewImages.DeleteImageList();

	UINT uiBmpId = IDB_CLASS_VIEW_24;

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

	nFlags |= ILC_COLOR24;

	m_PrototypeViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_PrototypeViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndPrototypeView.SetImageList(&m_PrototypeViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(IDB_SORT_24, 0, 0, TRUE /* Locked */);
}

void CPrototypeView::OnSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;

	if (pNMHDR->code != TVN_SELCHANGED)
		return;

	HTREEITEM hti = m_wndPrototypeView.GetSelectedItem();
	c3::Prototype *pproto = (c3::Prototype *)(m_wndPrototypeView.GetItemData(hti));
	if (pproto)
	{
		theApp.SetActiveProperties(pproto->GetProperties());
	}
}

BOOL CPrototypeView::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
	if (nID == PROTOTREE_ID)
	{
		switch (nCode)
		{
			case NM_DBLCLK:
			{
				break;
			}

			case TVN_ENDLABELEDIT:
			{
				break;
			}

			case TVN_BEGINLABELEDIT:
			{
				break;
			}
		}
	}

	return CDockablePane::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
