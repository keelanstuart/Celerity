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
#include "EditPrototypeDlg.h"
#include <tinyxml2.h>
#include "C3EditDoc.h"
#include "C3EditView.h"


#define IMGIDX_GROUP			2
#define IMGIDX_PROTOTYPE		5

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPrototypeView::CPrototypeView() noexcept
{
	m_hEditItem = 0;
	m_DragImage = nullptr;
}


CPrototypeView::~CPrototypeView()
{
}


#define PROTOTREE_ID		2

BEGIN_MESSAGE_MAP(CPrototypeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_PROPERTIES, OnPrototypeSearch)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, PROTOTREE_ID, OnSelectionChanged)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ACTIVATE()
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
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_EDITLABELS;

	if (!m_wndPrototypeView.Create(dwViewStyle, rectDummy, this, PROTOTREE_ID))
	{
		TRACE0("Failed to create Prototype View\n");
		return -1;      // fail to create
	}

	OnChangeVisualStyle();
	m_wndPrototypeView.SetBkColor(RGB(64, 64, 64));
	m_wndPrototypeView.SetTextColor(RGB(255, 255, 255));

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
		hcurr = hexisting ? hexisting : m_wndPrototypeView.InsertItem(gname, IMGIDX_GROUP, IMGIDX_GROUP, hcurr);

		HTREEITEM hnext = MakeProtoGroup(hcurr, group);

		m_wndPrototypeView.Expand(hcurr, TVE_EXPAND);

		hcurr = hnext;
	}

	return hcurr;
}


HTREEITEM CPrototypeView::FindItemByPrototype(HTREEITEM hroot, const c3::Prototype *pproto)
{
	if ((const c3::Prototype *)(m_wndPrototypeView.GetItemData(hroot)) == pproto)
		return hroot;

	HTREEITEM hc = m_wndPrototypeView.GetNextItem(hroot, TVGN_CHILD);
	while (hc)
	{
		HTREEITEM hret = FindItemByPrototype(hc, pproto);
		if (hret)
			return hret;

		hc = m_wndPrototypeView.GetNextItem(hc, TVGN_NEXT);
	}

	return NULL;
}


void CPrototypeView::UpdateItem(const c3::Prototype *pproto)
{
	HTREEITEM hitem = FindItemByPrototype(m_wndPrototypeView.GetRootItem(), pproto);
	m_wndPrototypeView.SetItemText(hitem, pproto->GetName());
}


void CPrototypeView::FillPrototypeView()
{
	m_wndPrototypeView.DeleteAllItems();

	HTREEITEM hRoot = m_wndPrototypeView.InsertItem(_T("All Prototypes"), IMGIDX_GROUP, IMGIDX_GROUP);
	m_wndPrototypeView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	c3::Factory *pfac = theApp.m_C3->GetFactory();
	for (size_t i = 0, maxi = pfac->GetNumPrototypes(); i < maxi; i++)
	{
		c3::Prototype *pproto = pfac->GetPrototype(i);
		assert(pproto);

		HTREEITEM hgroup = MakeProtoGroup(hRoot, pproto->GetGroup());

		CString tmp = pproto->GetName();
		HTREEITEM hitem = m_wndPrototypeView.InsertItem(tmp, IMGIDX_PROTOTYPE, IMGIDX_PROTOTYPE, hgroup);
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
	PC_IMPORT,
	PC_IMPORT_MODELS,
	PC_SAVEAS,
	PC_DUPLICATE,
	PC_RENAME,
	PC_CREATEABSORB,
	PC_ABSORB
};


void Absorb(c3::Prototype *ptarg, c3::Object *psrc)
{
	if (!ptarg || !psrc)
		return;

	ptarg->GetProperties()->AppendPropertySet(psrc->GetProperties(), false);
	ptarg->Flags().SetAll(psrc->Flags());
	for (size_t i = 0; i < psrc->GetNumComponents(); i++)
		ptarg->AddComponent(psrc->GetComponent(i)->GetType());
}

void ForEachTreeSubItemDo(CTreeCtrl* pTreeCtrl, HTREEITEM hRoot, std::function<void(c3::Prototype *)> func, bool procsiblings = false);

void ForEachTreeSubItemDo(CTreeCtrl* pTreeCtrl, HTREEITEM hRoot, std::function<void(c3::Prototype *)> func, bool procsiblings)
{
	if (!pTreeCtrl || !hRoot || !func)
		return;

	HTREEITEM hItem = hRoot;

	while (hItem)
	{
		// Get the item data
		c3::Prototype *pp = (c3::Prototype *)pTreeCtrl->GetItemData(hItem);
		if (pp)
		{
			// Call the user-provided function
			func(pp);
		}

		// Recursively process child items
		HTREEITEM hChild = pTreeCtrl->GetChildItem(hItem);
		if (hChild)
		{
			ForEachTreeSubItemDo(pTreeCtrl, hChild, func, true);
		}

		// Move to the next sibling item
		hItem = procsiblings ? pTreeCtrl->GetNextSiblingItem(hItem) : NULL;
	}
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

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	c3::Factory *pfac = theApp.m_C3->GetFactory();

	HMENU menu = ::CreatePopupMenu();
	HMENU submenu = ::CreatePopupMenu();

	MyAppendMenuItem(submenu, MFT_STRING, _T("Prototype"), true, 0, PC_CREATEPROTO);
	if (pdoc->GetNumSelected() == 1)
		MyAppendMenuItem(submenu, MFT_STRING, _T("Prototype (Absorb Selection)"), true, 0, PC_CREATEABSORB);
	MyAppendMenuItem(submenu, MFT_STRING, _T("Group"), true, 0, PC_CREATEGROUP);

	MyAppendMenuItem(menu, MFT_STRING, _T("New"), true, submenu);
	MyAppendMenuItem(menu, MFT_STRING, _T("Import From Model(s) ..."), true, 0, PC_IMPORT_MODELS);
	MyAppendMenuItem(menu, MFT_STRING, _T("Import Prototypes From ..."), true, 0, PC_IMPORT);
	MyAppendMenuItem(menu, MFT_STRING, _T("Save Prototypes As ..."), true, 0, PC_SAVEAS);

	MyAppendMenuItem(menu, MFT_SEPARATOR);

	std::function<bool(HTREEITEM)> IsGroupItem = [&](HTREEITEM testhti) -> bool
	{
		DWORD_PTR p = pWndTree->GetItemData(testhti);

		if (p)
			return false;

		return true;
	};

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
				MyAppendMenuItem(menu, MFT_STRING, _T("Duplicate"), true, 0, PC_DUPLICATE);
			}

			MyAppendMenuItem(menu, MFT_STRING, _T("Rename"), true, 0, PC_RENAME);
		}

		HTREEITEM hpi = hTreeItem;
		if (!hpi || !pWndTree->GetParentItem(hpi))
			hpi = pWndTree->GetRootItem();
		else if (!IsGroupItem(hpi))
			hpi = pWndTree->GetParentItem(hpi);

		std::function<void(HTREEITEM, tstring &)> BuildGroupName = [&](HTREEITEM hitem, tstring &retname)
		{
			if (!IsGroupItem(hitem))
				hitem = pWndTree->GetParentItem(hitem);

			while (hitem && (hitem != pWndTree->GetRootItem()))
			{
				CString s = pWndTree->GetItemText(hitem);
				if (!retname.empty())
					retname.insert(retname.begin(), _T('/'));

				retname.insert(0, s);
				hitem = pWndTree->GetParentItem(hitem);
			}
		};

		UINT ret = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN, point.x, point.y, 0, GetSafeHwnd(), NULL);
		switch (ret)
		{
			case PC_CREATEABSORB:
			case PC_CREATEPROTO:
			{
				c3::Prototype *pcp = pfac->CreatePrototype();

				if (ret != PC_CREATEABSORB)
				{
					TCHAR protoname[256];
					GUID g = pcp->GetGUID();
					_stprintf_s(protoname, _T("proto_%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"), g.Data1, g.Data2, g.Data3,
								g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
					pcp->SetName(protoname);
				}
				else
				{
					pcp->SetName(pdoc->GetSelection(0)->GetName());
				}

				tstring groupname;
				if (pproto)
					groupname = pproto->GetGroup();
				else
					BuildGroupName(hpi, groupname);

				pcp->SetGroup(groupname.c_str());

				CString tmp = pcp->GetName();
				HTREEITEM hitem = m_wndPrototypeView.InsertItem(tmp, IMGIDX_PROTOTYPE, IMGIDX_PROTOTYPE, hpi);
				m_wndPrototypeView.SetItemData(hitem, (DWORD_PTR)pcp);

				if (ret == PC_CREATEABSORB)
					Absorb(pcp, pdoc->GetSelection(0));

				pWndTree->EnsureVisible(hitem);
				theApp.SetActivePrototype(pcp);

				break;
			}

			case PC_CREATEGROUP:
			{
				if (pproto)
					hpi = pWndTree->GetParentItem(hpi);

				HTREEITEM hgroup = MakeProtoGroup(hpi, _T("New Group"));
				m_hEditItem = hgroup;
				pWndTree->EnsureVisible(hgroup);
				pWndTree->EditLabel(hgroup);

				break;
			}

			case PC_RENAME:
			{
				m_hEditItem = hpi;
				pWndTree->EnsureVisible(hpi);
				pWndTree->EditLabel(hpi);

				break;
			}

			case PC_SAVEAS:
			{
				tstring filter = _T("Celerity Prototypes (ASCII)|*.c3protoa");
				filter += _T("|Celerity Prototypes (Binary)|*.c3protob");
				filter += _T("|*.*|All Files (*.*)||");

				tstring groupname;
				BuildGroupName(hpi, groupname);

				c3::Factory::PROTO_SAVE_HUERISTIC_FUNCTION GroupSaveFunc = [&](c3::Prototype *pp) -> bool
				{
					if (!_tcsicmp(groupname.c_str(), pp->GetGroup()))
						return true;

					return false;
				};

				c3::Factory::PROTO_SAVE_HUERISTIC_FUNCTION ProtoSaveFunc = [&](c3::Prototype *pp) -> bool
				{
					return (pp == pproto);
				};

				c3::Factory::PROTO_SAVE_HUERISTIC_FUNCTION ProtoSaveFilter = IsGroupItem(hpi) ? GroupSaveFunc : ProtoSaveFunc;

				CFileDialog fd(FALSE, nullptr, nullptr, OFN_ENABLESIZING, filter.c_str(), nullptr, sizeof(OPENFILENAME));
				if (fd.DoModal() == IDOK)
				{
					if (!PathFileExists(fd.GetPathName()) || (MessageBox(_T("Selected file exists already; overwrite?"), _T("Confirm Overwrite"), MB_YESNO) == IDYES))
					{
						switch (fd.GetOFN().nFilterIndex - 1)
						{
							default:
							case 0:
							{
								tinyxml2::XMLDocument protodoc;
								tinyxml2::XMLElement *protoroot = protodoc.NewElement("prototypes");
								protodoc.InsertEndChild(protoroot);
								theApp.m_C3->GetFactory()->SavePrototypes(protoroot, ProtoSaveFilter);

								char *fn;
								CONVERT_TCS2MBCS(fd.GetPathName(), fn);
								protodoc.SaveFile(fn);

								break;
							}

							case 1:
							{
								genio::IOutputStream *pos = genio::IOutputStream::Create();
								if (pos)
								{
									pos->Assign(fd.GetPathName());
									if (pos->Open())
									{
										theApp.m_C3->GetFactory()->SavePrototypes(pos, ProtoSaveFilter);
										pos->Close();
									}

									pos->Release();
								}

								break;
							}
						}
					}
				}

				break;
			}

			case PC_IMPORT:
			{
				tstring filter = _T("C3 Prototypes Files (*.c3protoa; *.c3protob)|*.c3protoa;*.c3protob|*.*|All Files (*.*)||");

				CFileDialog fd(TRUE, nullptr, nullptr,
							   OFN_ALLOWMULTISELECT | OFN_ENABLESIZING,
							   filter.c_str(), nullptr, 8192);

				// Allocate a large buffer
				std::vector<TCHAR> file_buffer;

				// Allocate a large buffer (say 262144 characters)
				file_buffer.resize(1 << 16, _T('\0'));

				// Assign the buffer to the OPENFILENAME structure.
				fd.m_ofn.lpstrFile = file_buffer.data();
				fd.m_ofn.nMaxFile  = static_cast<DWORD>(file_buffer.size());

				if (fd.DoModal() == IDOK)
				{
					POSITION fpos = fd.GetStartPosition();
					while (fpos)
					{
						CPath fn = fd.GetNextPathName(fpos);
						theApp.m_C3->GetFactory()->LoadPrototypes((tinyxml2::XMLDocument *)nullptr, fn);
					}

					FillPrototypeView();
				}
				break;
			}

			case PC_IMPORT_MODELS:
			{
				const c3::ResourceType *pModResType = theApp.m_C3->GetResourceManager()->FindResourceTypeByName(_T("Model"));
				assert(pModResType);

				tstring exts = pModResType->GetReadableExtensions();
				tstring::iterator extsit = exts.begin();
				while (extsit != exts.end())
				{
					size_t o = std::distance(exts.begin(), extsit);
					exts.insert(o, _T("*."));
					extsit = std::find(exts.begin() + o, exts.end(), _T(';'));
					if (extsit != exts.end())
						extsit++;
				}

				tstring filter = _T("Model Files|");
				filter += exts;
				filter += _T("|*.*|All Files (*.*)||");

				CFileDialog fd(TRUE, nullptr, nullptr,
							   OFN_ALLOWMULTISELECT | OFN_ENABLESIZING,
							   filter.c_str(), nullptr, 8192);

				// Allocate a large buffer
				std::vector<TCHAR> file_buffer;

				// Allocate a large buffer (say 262144 characters)
				file_buffer.resize(1 << 16, _T('\0'));

				// Assign the buffer to the OPENFILENAME structure.
				fd.m_ofn.lpstrFile = file_buffer.data();
				fd.m_ofn.nMaxFile  = static_cast<DWORD>(file_buffer.size());

				if (fd.DoModal() == IDOK)
				{
					if (pproto)
						hpi = pWndTree->GetParentItem(hpi);

					tstring groupname;
					BuildGroupName(hpi, groupname);

					POSITION fpos = fd.GetStartPosition();
					while (fpos)
					{
						CPath fn = fd.GetNextPathName(fpos);

						c3::Prototype *ptmp = theApp.m_C3->GetFactory()->CreatePrototype();
						ptmp->SetGroup(groupname.c_str());

						ptmp->AddComponent(c3::Positionable::Type());
						ptmp->AddComponent(c3::ModelRenderer::Type());

						ptmp->GetProperties()->CreateProperty(_T("RenderMethod"), 'C3RM')->SetString(_T("std.c3rm"));
						ptmp->GetProperties()->CreateProperty(_T("Model"), 'MODF')->SetString(fn);
						ptmp->Flags().Set(OF_CASTSHADOW | OF_DRAW);

						fn.RemoveExtension();
						ptmp->SetName((LPCTSTR)fn + fn.FindFileName());

						HTREEITEM hitem = m_wndPrototypeView.InsertItem(ptmp->GetName(), IMGIDX_PROTOTYPE, IMGIDX_PROTOTYPE, hpi);
						m_wndPrototypeView.SetItemData(hitem, (DWORD_PTR)ptmp);
					}
				}
				break;
			}

			case PC_DELETE:
			{
				bool d = false;
				if (IsGroupItem(hTreeItem))
					d = (MessageBox(_T("Do you really want to delete this group and all prototypes in it?"), _T("Confirm Group Deletion"), MB_YESNO) == IDYES);
				else
					d = (MessageBox(_T("Do you really want to delete this prototype?"), _T("Confirm Prototype Deletion"), MB_YESNO) == IDYES);

				if (d)
				{
					ForEachTreeSubItemDo(pWndTree, hTreeItem, [&](c3::Prototype *pp)
					{
						pfac->RemovePrototype(pp);
					});

					theApp.SetActivePrototype(nullptr);
					theApp.SetActiveProperties(nullptr);
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

	m_wndPrototypeView.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BOOL CPrototypeView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE)))
	{
		if (m_hEditItem)
		{
			CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;

			if (pMsg->wParam == VK_ESCAPE)
				pWndTree->GetEditControl()->SetWindowText(pWndTree->GetItemText(m_hEditItem));

			// Cancel the edit by changing focus away from the edit control
			SetFocus();

			m_hEditItem = NULL;

			return FALSE;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
			C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
			POSITION vp = pdoc->GetFirstViewPosition();
			C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

			pv->SetFocus();
		}
	}

	return CDockablePane::PreTranslateMessage(pMsg);
}


void CPrototypeView::OnPrototypeSearch()
{
	// TODO: Add your command handler code here
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
		theApp.SetActivePrototype(pproto);
	}
}


BOOL CPrototypeView::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
	return CDockablePane::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL CPrototypeView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;
	NMTREEVIEW *pntv = (NMTREEVIEW *)lParam;

	switch (pntv->hdr.code)
	{
		case TVN_BEGINDRAG:
		{
			m_DragItem = std::make_optional<TVITEM>(pntv->itemNew);
			m_DragItem->pszText = _tcsdup(pWndTree->GetItemText(pntv->itemNew.hItem));
			m_DragItem->state = pWndTree->GetItemState(pntv->itemNew.hItem, -1);

			// Tell the tree-view control to create an image to use for dragging
			m_DragImage = pWndTree->CreateDragImage(pntv->itemNew.hItem); 

			// Get the bounding rectangle of the item being dragged. 
			RECT rcItem;
			pWndTree->GetItemRect(pntv->itemNew.hItem, &rcItem, TRUE); 

			// Start the drag operation. 
			m_DragImage->BeginDrag(0, pntv->ptDrag);
			m_DragImage->DragEnter(this, pntv->ptDrag); 

			// Hide the mouse pointer, and direct mouse input to the 
			// parent window. 
			ShowCursor(FALSE); 
			SetCapture(); 

			break;
		}

		case TVN_ENDLABELEDIT:
		{
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)lParam;

			CString name;
			pWndTree->GetEditControl()->GetWindowText(name);
			if (!name.IsEmpty())
			{
				c3::Prototype *pp = (c3::Prototype *)pWndTree->GetItemData(pdi->item.hItem);
				if (pp)
				{
					pp->SetName(name);
					theApp.SetActivePrototype(nullptr);
					theApp.SetActivePrototype(pp);
				}

				pWndTree->SetItem(pdi->item.hItem, TVIF_TEXT, name, 0, 0, 0, 0, 0);
			}
			m_hEditItem = 0;

			*pResult = 0;
			break;
		}

		case TVN_BEGINLABELEDIT:
		{
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)lParam;
			m_hEditItem = pdi->item.hItem;

			*pResult = 0;
			break;
		}

		default:
		{
			//return true;
			break;
		}
	}

	return CDockablePane::OnNotify(wParam, lParam, pResult);
}


void CPrototypeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;

	if (m_DragItem.has_value())
	{
		HTREEITEM hti = pWndTree->GetDropHilightItem();
		if (hti != NULL)
		{
			if (!pWndTree->GetItemData(hti))
			{
				pWndTree->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE, m_DragItem->pszText, m_DragItem->iImage,
									 m_DragItem->iSelectedImage, m_DragItem->state, -1,
									 m_DragItem->lParam, hti, pWndTree->GetChildItem(hti));
			}
			else
			{
				pWndTree->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE, m_DragItem->pszText, m_DragItem->iImage,
									 m_DragItem->iSelectedImage, m_DragItem->state, -1,
									 m_DragItem->lParam, pWndTree->GetParentItem(hti), hti);
			}

			pWndTree->DeleteItem(m_DragItem->hItem);
		}
		m_DragImage->EndDrag(); 
		pWndTree->SelectDropTarget(NULL);
		ReleaseCapture(); 
		ShowCursor(TRUE); 

		delete m_DragImage;

		free(m_DragItem->pszText);
		m_DragItem.reset();
	}

	CDockablePane::OnLButtonUp(nFlags, point);
}


void CPrototypeView::OnMouseMove(UINT nFlags, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndPrototypeView;

	if (m_DragItem.has_value())
	{ 
		HTREEITEM hti;			// Handle to target item. 
		TVHITTESTINFO tvht;		// Hit test information. 

		// Turn off the dragged image so the background can be refreshed.
		m_DragImage->DragShowNolock(FALSE);

		CPoint sp = point;

		// Find out if the pointer is on the item. If it is,
		// highlight the item as a drop target
		ClientToScreen(&sp);
		pWndTree->ScreenToClient(&sp);
		m_DragImage->DragMove(sp);

		tvht.pt.x = sp.x;
		tvht.pt.y = sp.y;
		if ((hti = pWndTree->HitTest(&tvht)) != NULL)
		{
			//if (!pWndTree->GetItemData(hti) || pWndTree->GetParentItem(hti))
			{
				pWndTree->SelectDropTarget(hti);
			}
		}

		m_DragImage->DragShowNolock(TRUE);
	} 

	CDockablePane::OnMouseMove(nFlags, point);
}


void CPrototypeView::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDockablePane::OnActivate(nState, pWndOther, bMinimized);
}


void CPrototypeView::ActOnSelection(PrototypeActionFunc func, bool recursive)
{
	CTreeCtrl* pwt = (CTreeCtrl*)&m_wndPrototypeView;
	HTREEITEM hti = pwt->GetSelectedItem();

	std::function<void(HTREEITEM, PrototypeActionFunc, bool)> internalfunc = [&](HTREEITEM h, PrototypeActionFunc f, bool r)
	{
		if (!h)
			return;

		c3::Prototype *p = (c3::Prototype *)pwt->GetItemData(h);
		if (p)
		{
			func(p);
		}
		else if (r)
		{
			HTREEITEM hc = pwt->GetChildItem(h);
			while (hc)
			{
				internalfunc(hc, func, r);
				hc = pwt->GetNextSiblingItem(hc);
			}
		}
	};

	internalfunc(pwt->GetSelectedItem(), func, recursive);
}
