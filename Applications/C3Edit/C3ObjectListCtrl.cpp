// C3ObjectListCtrl.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "C3EditDoc.h"
#include "C3EditFrame.h"
#include "ObjectsWnd.h"
#include "C3ObjectListCtrl.h"
#include <gdiplus.h>
#include "C3EditView.h"


// C3ObjectListCtrl

IMPLEMENT_DYNAMIC(C3ObjectListCtrl, CListCtrl)


#define ICON_COUNT		3
#define ICON_SIZE		15


C3ObjectListCtrl::C3ObjectListCtrl()
{

}

C3ObjectListCtrl::~C3ObjectListCtrl()
{
}


BEGIN_MESSAGE_MAP(C3ObjectListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &C3ObjectListCtrl::OnLvnGetdispinfo)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CLICK, &C3ObjectListCtrl::OnNMClick)
	ON_WM_CONTEXTMENU()

	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &C3ObjectListCtrl::OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_DELETE, &C3ObjectListCtrl::OnEditDelete)

	ON_UPDATE_COMMAND_UI(ID_EDIT_DUPLICATE, &C3ObjectListCtrl::OnUpdateEditDuplicate)
	ON_COMMAND(ID_EDIT_DUPLICATE, &C3ObjectListCtrl::OnEditDuplicate)

	ON_UPDATE_COMMAND_UI(ID_EDIT_ASSIGNROOT, &C3ObjectListCtrl::OnUpdateEditAssignRoot)
	ON_COMMAND(ID_EDIT_ASSIGNROOT, &C3ObjectListCtrl::OnEditAssignRoot)

	ON_UPDATE_COMMAND_UI(ID_EDIT_GROUP, &C3ObjectListCtrl::OnUpdateEditGroup)
	ON_COMMAND(ID_EDIT_GROUP, &C3ObjectListCtrl::OnEditGroup)

	ON_UPDATE_COMMAND_UI(ID_EDIT_UNGROUP, &C3ObjectListCtrl::OnUpdateEditUngroup)
	ON_COMMAND(ID_EDIT_UNGROUP, &C3ObjectListCtrl::OnEditUngroup)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CENTERCAMERAON, &C3ObjectListCtrl::OnUpdateEditCenterCamera)
	ON_COMMAND(ID_EDIT_CENTERCAMERAON, &C3ObjectListCtrl::OnEditCenterCamera)

	ON_COMMAND(ID_EDIT_COPY, &C3ObjectListCtrl::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &C3ObjectListCtrl::OnUpdateEditCopy)

	ON_COMMAND(ID_EDIT_CUT, &C3ObjectListCtrl::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &C3ObjectListCtrl::OnUpdateEditCut)

	ON_COMMAND(ID_EDIT_PASTE, &C3ObjectListCtrl::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &C3ObjectListCtrl::OnUpdateEditPaste)

	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, &C3ObjectListCtrl::OnLvnBegindrag)
END_MESSAGE_MAP()



// C3ObjectListCtrl message handlers




int C3ObjectListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetBkColor(RGB(64, 64, 64));
	SetTextColor(RGB(255, 255, 255));

	InsertColumn(0, _T("Object"), LVCFMT_LEFT, 100, 0);		// Place to display object info
	InsertColumn(1, _T("Flags"), LVCFMT_LEFT, 100, 1);		// Place to display clickable icons for visibility, updates, etc. / setting flags

	return 0;
}


void C3ObjectListCtrl::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);

	//Create a pointer to the item and get the item number
	LV_ITEM *pItem = &(pDispInfo)->item;
	int itemid = pItem->iItem;

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, itemid);
	if (pobj)
	{
		//Do the list need text information?
		if (pItem->mask & LVIF_TEXT)
		{
			//Which column?
			if (pItem->iSubItem == 0)
			{
				lstrcpyn(pItem->pszText, pItem->iItem ? pobj->GetName() : pdoc->GetTitle(), pItem->cchTextMax);
			}
		}
	}

	*pResult = 0;
}


BOOL C3ObjectListCtrl::PreCreateWindow(CREATESTRUCT &cs)
{
	cs.style |= LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED;
	cs.dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_TRACKSELECT;

	return CListCtrl::PreCreateWindow(cs);
}


void C3ObjectListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	int col1_width = (ICON_COUNT * (ICON_SIZE + 2));

	SetColumnWidth(0, cx - col1_width);
	SetColumnWidth(1, col1_width);
}


void C3ObjectListCtrl::DrawItem(LPDRAWITEMSTRUCT pdi)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, pdi->itemID);
	if (!pobj)
		return;

	props::TFlags64 ofl = 0;
	ofl = ((c3::Object *)pobj)->Flags();

	// Get object name
	const TCHAR *name = pobj ? (pdi->itemID ? pobj->GetName() : pdoc->GetTitle()) : _T("ERROR");
	INT namelen = (INT)_tcslen(name);

	// Get object GUID
	TCHAR gs[128];
	GUID g = pobj->GetGuid();
	INT gslen = _stprintf_s(gs, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
							g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

	CPaintDC *pdc = (CPaintDC *)CDC::FromHandle(pdi->hDC);

	Gdiplus::Graphics gr(*pdc);

	Gdiplus::SolidBrush br_selected(Gdiplus::Color(255, 45, 45, 192));
	Gdiplus::SolidBrush br_bg(Gdiplus::Color(255, 80, 80, 80));
	Gdiplus::SolidBrush br_text(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush br_text_dark(Gdiplus::Color(255, 128, 128, 128));
	Gdiplus::SolidBrush br_text_seldark(Gdiplus::Color(255, 160, 160, 192));
	Gdiplus::SolidBrush br_sym(Gdiplus::Color(255, 192, 192, 192));
	Gdiplus::SolidBrush br_sym_hi(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush br_sym_lo(Gdiplus::Color(255, 0, 0, 0));
	Gdiplus::Pen pen_oproot(Gdiplus::Color(128, 255, 255, 128), 2.0f);

	CRect ritem = pdi->rcItem, rcol[2];

	GetSubItemRect(pdi->itemID, 0, LVIR_BOUNDS, rcol[0]);
	GetSubItemRect(pdi->itemID, 1, LVIR_BOUNDS, rcol[1]);

	LONG od = 0;
	const c3::Object *ppar = pobj;
	while (ppar = ppar->GetParent())
		od++;

	bool issel = pdoc->IsSelected(pobj);

	if (issel)
	{
		gr.FillRectangle(&br_selected, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}
	else
	{
		if (!(pdi->itemID & 1))
			gr.FillRectangle(&br_bg, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}

	if (pdoc->m_OperationalRootObj == pobj)
	{
		gr.DrawRectangle(&pen_oproot, ritem.left + 1, ritem.top + 1, ritem.Width() - 2, ritem.Height() - 2);
	}

	Gdiplus::Font f_list(pdi->hDC);
	LOGFONT lf_guid;
	f_list.GetLogFontW(&gr, &lf_guid);
	lf_guid.lfWeight = FW_EXTRALIGHT;
	lf_guid.lfHeight = lf_guid.lfHeight * 3 / 4;
	lf_guid.lfItalic = TRUE;
	Gdiplus::Font f_guid(pdi->hDC, &lf_guid);

	Gdiplus::Font f_sym1(_T("Webdings"), Gdiplus::REAL(14.0));
	Gdiplus::Font f_sym2(_T("Wingdings"), Gdiplus::REAL(14.0));

	Gdiplus::PointF pt;
	Gdiplus::RectF tnr;

	TCHAR *exps = ofl.IsSet(OF_EXPANDED) ? _T("6") : _T("4");
	gr.MeasureString(_T("4"), 1, &f_sym1, Gdiplus::PointF(0.0, 0.0), &tnr);
	Gdiplus::REAL expw = tnr.Width;

	pt.X = (Gdiplus::REAL)(rcol[0].left + (tnr.Width * od));

	// DRAW EXPANSION WIDGET
	if (pobj->GetNumChildren())
	{
		pt.Y = (Gdiplus::REAL)(rcol[0].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height * 2.0 / 3.0);
		gr.DrawString(exps, 1, &f_sym1, pt, &br_sym);
	}

	pt.X += expw;

	// DRAW OBJECT NAME
	gr.MeasureString(name, namelen, &f_list, Gdiplus::PointF(0.0, 0.0), &tnr);
	pt.Y = (Gdiplus::REAL)(rcol[0].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0) - (Gdiplus::REAL)(tnr.Height / 3.0);
	gr.DrawString(name, namelen, &f_list, pt, &br_text);

	// DRAW OBJECT GUID
	gr.MeasureString(gs, gslen, &f_guid, Gdiplus::PointF(0.0, 0.0), &tnr);
	pt.Y = (Gdiplus::REAL)(rcol[0].CenterPoint().y) + (Gdiplus::REAL)(tnr.Height / 2.0) - (Gdiplus::REAL)(tnr.Height / 3.0);
	gr.DrawString(gs, gslen, &f_guid, pt, issel ? &br_text_seldark : &br_text_dark);


	Gdiplus::REAL widw = 0;

	// DRAW VISIBILITY WIDGET
	gr.MeasureString(_T("N"), 1, &f_sym1, Gdiplus::PointF(0.0, 0.0), &tnr);
	pt.X = (Gdiplus::REAL)(rcol[1].left) + (ICON_SIZE * 0);
	pt.Y = (Gdiplus::REAL)(rcol[1].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0);
	gr.DrawString(_T("N"), 1, &f_sym1, pt, ofl.IsSet(OF_DRAW) ? &br_sym : &br_sym_lo);

	// DRAW UPDATE WIDGET
	gr.MeasureString(_T("6"), 1, &f_sym2, Gdiplus::PointF(0.0, 0.0), &tnr);
	pt.X = (Gdiplus::REAL)(rcol[1].left) + (ICON_SIZE * 1) + 4;
	pt.Y = (Gdiplus::REAL)(rcol[1].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0) + 2;
	gr.DrawString(_T("6"), 1, &f_sym2, pt, ofl.IsSet(OF_UPDATE) ? &br_sym : &br_sym_lo);

	// DRAW UPDATE WIDGET
	gr.MeasureString(_T("Ï"), 1, &f_sym1, Gdiplus::PointF(0.0, 0.0), &tnr);
	pt.X = (Gdiplus::REAL)(rcol[1].left) + (ICON_SIZE * 2) - 2;
	pt.Y = (Gdiplus::REAL)(rcol[1].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0) - 2;
	gr.DrawString(_T("Ï"), 1, &f_sym1, pt, ofl.IsSet(OF_LOCKED) ? &br_sym : &br_sym_lo);

}

void C3ObjectListCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CMemDC mdc(dc, this);

	Gdiplus::Graphics gr(mdc.GetDC());

	Gdiplus::SolidBrush br_bg(Gdiplus::Color(255, 64, 64, 64));

	CRect r;
	GetClientRect(r);
	gr.FillRectangle(&br_bg, r.left, r.top, r.Width(), r.Height());

	DefWindowProc(WM_PAINT, (WPARAM)(mdc.GetDC().GetSafeHdc()), (LPARAM)0);

	dc.BitBlt(0, 0, r.right, r.bottom, &(mdc.GetDC()), 0, 0, SRCCOPY);
}

void C3ObjectListCtrl::MeasureItem(LPMEASUREITEMSTRUCT pmi)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	float scl = pfrm ? pfrm->m_Scale : 1.0f;

	CRect r;
	GetClientRect(r);
	pmi->itemHeight = UINT(36.0f * scl);
	pmi->itemWidth = r.Width();
}



void C3ObjectListCtrl::OnMouseHover(UINT nFlags, CPoint point)
{
	CListCtrl::OnMouseHover(nFlags, point);

	m_HoverPos = point;

	UINT flags;
	int i = HitTest(point, &flags);
	RedrawItems(i, i);
}


void C3ObjectListCtrl::OnMouseLeave()
{
	m_HoverPos.reset();

	CListCtrl::OnMouseLeave();
}


void C3ObjectListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	m_HoverPos.reset();

	CListCtrl::OnMouseMove(nFlags, point);
}


void C3ObjectListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int i = HitTest(point, 0);

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, i);

	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	if (pobj->GetNumChildren())
	{
		((c3::Object *)pobj)->Flags().Toggle(OF_EXPANDED);

		pfrm->UpdateObjectList();
	}

	CListCtrl::OnLButtonDblClk(nFlags, point);
}


void C3ObjectListCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	int i = HitTest(point, 0);

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, i);

	if (!pdoc->IsSelected(pobj))
	{
		pdoc->ClearSelection();
		pdoc->AddToSelection(pobj);
		pfrm->UpdateObjectList();
	}

	ClientToScreen(&point);
	OnContextMenu(this, point);
}


void C3ObjectListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CListCtrl::OnLButtonDown(nFlags, point);
}


void C3ObjectListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CListCtrl::OnLButtonUp(nFlags, point);
}


void C3ObjectListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, pNMItemActivate->iItem);

	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	if (!(pNMItemActivate->uKeyFlags & LVKF_CONTROL))
	{
		pdoc->ClearSelection();
		pdoc->AddToSelection(pobj);
	}
	else
	{
		if (pdoc->IsSelected(pobj))
			pdoc->RemoveFromSelection(pobj);
		else
			pdoc->AddToSelection(pobj);
	}

	UpdateData();

	*pResult = 0;
}


void C3ObjectListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	CPoint cp = point;
	ScreenToClient(&cp);
	
	int i = HitTest(cp, 0);

	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, i);

	if (!pdoc->IsSelected(pobj))
	{
		pdoc->ClearSelection();
		pdoc->AddToSelection(pobj);
		pfrm->UpdateObjectList();
	}

	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

void C3ObjectListCtrl::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditDelete(pCmdUI);
}


void C3ObjectListCtrl::OnEditDelete()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditDelete();
}


void C3ObjectListCtrl::OnUpdateEditDuplicate(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditDuplicate(pCmdUI);
}


void C3ObjectListCtrl::OnEditDuplicate()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditDuplicate();
}


void C3ObjectListCtrl::OnUpdateEditAssignRoot(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditAssignRoot(pCmdUI);
}


void C3ObjectListCtrl::OnEditAssignRoot()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditAssignRoot();
}


void C3ObjectListCtrl::OnUpdateEditGroup(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditGroup(pCmdUI);
}


void C3ObjectListCtrl::OnEditGroup()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditGroup();
}


void C3ObjectListCtrl::OnUpdateEditUngroup(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditUngroup(pCmdUI);
}


void C3ObjectListCtrl::OnEditUngroup()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditUngroup();
}


void C3ObjectListCtrl::OnUpdateEditCenterCamera(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditCenterCamera(pCmdUI);
}


void C3ObjectListCtrl::OnEditCenterCamera()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditCenterCamera();
}


void C3ObjectListCtrl::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditCopy(pCmdUI);
}


void C3ObjectListCtrl::OnEditCopy()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditCopy();
}


void C3ObjectListCtrl::OnUpdateEditCut(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditCut(pCmdUI);
}


void C3ObjectListCtrl::OnEditCut()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditCut();
}


void C3ObjectListCtrl::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnUpdateEditPaste(pCmdUI);
}


void C3ObjectListCtrl::OnEditPaste()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	pv->OnEditPaste();
}


void C3ObjectListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	switch (nChar)
	{
		case VK_ESCAPE:
			pv->SetFocus();
			break;

		default:
			break;
	}

	CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}


void C3ObjectListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	switch (nChar)
	{
		case _T('C'):
			pv->OnEditCenterCamera();
			break;

		default:
			break;
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void C3ObjectListCtrl::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
}
