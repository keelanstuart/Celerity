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
#include "ObjectsWnd.h"
#include "Resource.h"
#include "C3Edit.h"
#include "C3EditDoc.h"
#include "EditPrototypeDlg.h"

#include <C3Object.h>

#define IDC_OBJLIST		1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CObjectWnd::CObjectWnd() noexcept
{
}


CObjectWnd::~CObjectWnd()
{
}


BEGIN_MESSAGE_MAP(CObjectWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CObjectWnd message handlers

int CObjectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_OWNERDATA;

	if (!m_ObjList.Create(dwViewStyle, r, this, IDC_OBJLIST))
	{
		TRACE0("Failed to create the object list\n");
		return -1;      // fail to create
	}

	//OnChangeVisualStyle();

	return 0;
}


void CObjectWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}



void CObjectWnd::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect r;
	GetClientRect(r);

	m_ObjList.SetWindowPos(nullptr, r.left, r.top, r.Width(), r.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BOOL CObjectWnd::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}


void CObjectWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_ObjList.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}


void CObjectWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_ObjList.SetFocus();
}


BOOL CObjectWnd::OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
{
	return CDockablePane::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL CObjectWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
#if 0
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);
	LVITEM *pItem = &(pDispInfo)->item;

	int iItem = pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		switch (pItem->iSubItem)
		{
			case 0: //fill in main text
				_tcscpy_s(pItem->pszText, pItem->cchTextMax,
						  m_Items[iItem].m_strItemText);
				break;
			case 1: //fill in sub item 1 text
				_tcscpy_s(pItem->pszText, pItem->cchTextMax,
						  m_Items[iItem].m_strSubItem1Text);
				break;
			case 2: //fill in sub item 2 text
				_tcscpy_s(pItem->pszText, pItem->cchTextMax,
						  m_Items[iItem].m_strSubItem2Text);
				break;
		}
	}

	if (pItem->mask & LVIF_IMAGE) //valid image?
	{
		pItem->iImage = m_Items[iItem].m_iImage;
	}
#endif

	return CDockablePane::OnNotify(wParam, lParam, pResult);
}

size_t CObjectWnd::GetVisibleItemCount(const c3::Object *proot)
{
	assert(proot);

	size_t ret = 0;

	if (((c3::Object *)proot)->Flags().IsSet(OF_EXPANDED))
		for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++)
			ret += GetVisibleItemCount(proot->GetChild(i));

	return ret + 1;
}

const c3::Object *CObjectWnd::GetItemByIndex(const c3::Object *proot, size_t index)
{
	assert(proot);

	if (!index)
		return proot;

	index--;

	if (((c3::Object *)proot)->Flags().IsSet(OF_EXPANDED))
	{
		for (size_t i = 0, maxi = proot->GetNumChildren(); i < maxi; i++, index--)
		{
			const c3::Object *o = GetItemByIndex(proot->GetChild(i), index);
			if (o)
				return o;
		}
	}

	return nullptr;
}

void CObjectWnd::UpdateContents()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());

	if (!pdoc || !pdoc->m_RootObj)
	{
		m_ObjList.SetItemCount(1);
		return;
	}

	size_t ct = GetVisibleItemCount(pdoc->m_RootObj);
	m_ObjList.SetItemCount((int)ct);
}
