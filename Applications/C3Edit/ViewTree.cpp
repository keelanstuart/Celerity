
#include "pch.h"
#include "framework.h"
#include "ViewTree.h"
#include "C3Edit.h"
#include "C3EditDoc.h"
#include "C3EditFrame.h"

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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree message handlers

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != nullptr);

#pragma warning(suppress : 26454)
	if (pNMHDR)
	{
		switch (pNMHDR->code)
		{
			case TTN_SHOW:
				if (GetToolTips() != nullptr)
					GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
				break;

			case NM_DBLCLK:
				break;
		}
	}

	return bRes;
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
		c3::Object *pcam = pdoc->GetPerViewInfo(h)->obj;
		c3::Camera *pcampos = (c3::Camera *)(pcam->FindComponent(c3::Camera::Type()));
		pcam->Update();

		c3::Object *pobj = theApp.m_C3->GetFactory()->Build(pproto);
		pdoc->m_RootObj->AddChild(pobj);

		c3::Positionable *pobjpos = (c3::Positionable *)(pobj->FindComponent(c3::Positionable::Type()));

		glm::fvec3 ct;
		pcampos->GetTargetPos(&ct);
		pobjpos->SetPosVec(&ct);

		pdoc->SetModifiedFlag();
	}

	*pResult = 0;
}
