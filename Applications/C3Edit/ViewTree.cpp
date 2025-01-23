// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include "framework.h"
#include "ViewTree.h"
#include "C3Edit.h"
#include "C3EditDoc.h"
#include "C3EditFrame.h"
#include "C3EditView.h"

#include <C3Utility.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree() noexcept
{
}

CViewTree::~CViewTree()
{
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CViewTree::OnNMDblclk)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree message handlers

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != nullptr);

#pragma warning(suppress : 26454)
	if (pNMHDR && (pNMHDR->code == TTN_SHOW) && (GetToolTips() != nullptr))
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);

	return CTreeCtrl::OnNotify(wParam, lParam, pResult);
}


void CViewTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hti = GetSelectedItem();

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());

	c3::Prototype *pproto = (c3::Prototype *)GetItemData(hti);
	if (pproto)
	{
		HWND h = pfrm->GetActiveView()->GetSafeHwnd();
		c3::Object *pcam = pdoc->GetPerViewInfo(h)->m_Camera;
		c3::Camera *pcampos = (c3::Camera *)(pcam->FindComponent(c3::Camera::Type()));
		pcam->Update();

		c3::Object *pobj = theApp.m_C3->GetFactory()->Build(pproto);

		c3::Object *root = pdoc->m_OperationalRootObj ? pdoc->m_OperationalRootObj : pdoc->m_RootObj;
		root->AddChild(pobj);

		glm::fvec3 ct;
		pcampos->GetTargetPos(&ct);

		glm::fmat4x4 root_mat;
		c3::util::ComputeFinalTransform(root, &root_mat);
		glm::fvec3 pt = root_mat * glm::fvec4(0, 0, 0, 1);
		ct -= pt;

		c3::Positionable *pobjpos = (c3::Positionable *)(pobj->FindComponent(c3::Positionable::Type()));
		if (pobjpos)
			pobjpos->SetPosVec(&ct);

		pfrm->GetActiveView()->SetFocus();

		pdoc->ClearSelection();
		pdoc->AddToSelection(pobj);

		pdoc->SetModifiedFlag();

		pfrm->UpdateObjectList();
	}

	*pResult = 0;
}


HBRUSH CViewTree::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTreeCtrl::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}


void CViewTree::OnLButtonDown(UINT nFlags, CPoint point)
{
	TVHITTESTINFO tvht;		// Hit test information. 
	CPoint sp = point;

	tvht.pt.x = point.x;
	tvht.pt.y = point.y;
	HTREEITEM hti = HitTest(&tvht);
	if (hti)
	{
		c3::Prototype *pp = (c3::Prototype *)GetItemData(hti);
		if (pp)
		{
			theApp.SetActivePrototype(pp);
		}
	}

	CTreeCtrl::OnLButtonDown(nFlags, point);
}
