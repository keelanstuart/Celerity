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


#define ICON_COUNT		2
#define ICON_SIZE		17


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


#if defined(UNICODE)
#define GetLogFont	GetLogFontW
#else
#define GetLogFont	GetLogFontA
#endif

void C3ObjectListCtrl::DrawItem(LPDRAWITEMSTRUCT pdi)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	const c3::Object *pobj = ((CObjectWnd *)GetParent())->GetItemByIndex(pdoc->m_RootObj, pdi->itemID);

	// Get object name
	const TCHAR *name = pdi->itemID ? pobj->GetName() : pdoc->GetTitle();
	INT namelen = (INT)_tcslen(name);

	// Get object GUID
	TCHAR gs[128];
	GUID g = pobj->GetGuid();
	INT gslen = _stprintf_s(gs, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
							g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

	CPaintDC *pdc = (CPaintDC *)CDC::FromHandle(pdi->hDC);

	Gdiplus::Graphics gr(*pdc);

	Gdiplus::SolidBrush br_selected(Gdiplus::Color(255, 45, 45, 192));
	Gdiplus::SolidBrush br_bg(Gdiplus::Color(255, 45, 45, 45));
	Gdiplus::SolidBrush br_text(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush br_text_dark(Gdiplus::Color(255, 128, 128, 128));
	Gdiplus::SolidBrush br_text_seldark(Gdiplus::Color(255, 160, 160, 192));
	Gdiplus::SolidBrush br_sym(Gdiplus::Color(255, 192, 192, 192));
	Gdiplus::SolidBrush br_sym_hi(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush br_sym_lo(Gdiplus::Color(255, 0, 0, 0));

	CRect ritem = pdi->rcItem, rcol[2];

	GetSubItemRect(pdi->itemID, 0, LVIR_BOUNDS, rcol[0]);
	GetSubItemRect(pdi->itemID, 1, LVIR_BOUNDS, rcol[1]);

	LONG od = 0;
	const c3::Object *ppar = pobj;
	while (ppar = ppar->GetParent())
		od++;

	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	bool issel = pv->IsSelected(pobj);

	if (issel)
	{
		gr.FillRectangle(&br_selected, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}
	else
	{
		if (!(pdi->itemID & 1))
			gr.FillRectangle(&br_bg, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}

	Gdiplus::Font f_list(pdi->hDC);
	LOGFONT lf_guid;
	f_list.GetLogFont(&gr, &lf_guid);
	lf_guid.lfWeight = FW_EXTRALIGHT;
	lf_guid.lfHeight = lf_guid.lfHeight * 3 / 4;
	lf_guid.lfItalic = TRUE;
	Gdiplus::Font f_guid(pdi->hDC, &lf_guid);

	Gdiplus::Font f_sym1(_T("Webdings"), Gdiplus::REAL(14.0));
	Gdiplus::Font f_sym2(_T("Wingdings"), Gdiplus::REAL(14.0));

	Gdiplus::PointF pt;
	Gdiplus::RectF tnr;

	TCHAR *exps = ((c3::Object *)pobj)->Flags().IsSet(OF_EXPANDED) ? _T("6") : _T("4");
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


	gr.MeasureString(_T("N"), 1, &f_sym1, Gdiplus::PointF(0.0, 0.0), &tnr);
	Gdiplus::REAL widw = tnr.Width;

	// DRAW VISIBILITY WIDGET
	pt.X = (Gdiplus::REAL)(rcol[1].left);
	pt.Y = (Gdiplus::REAL)(rcol[1].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0) - (Gdiplus::REAL)(tnr.Height / 3.0);
	gr.DrawString(_T("N"), 1, &f_sym1, pt, ((c3::Object *)pobj)->Flags().IsSet(OF_DRAW) ? &br_sym : &br_sym_lo);

	// DRAW UPDATE WIDGET
	pt.X = (Gdiplus::REAL)(rcol[1].left + (widw * 2 / 3) + 2);
	pt.Y = (Gdiplus::REAL)(rcol[1].CenterPoint().y) - (Gdiplus::REAL)(tnr.Height / 2.0) - (Gdiplus::REAL)(tnr.Height / 3.0);
	gr.DrawString(_T("6"), 1, &f_sym2, pt, ((c3::Object *)pobj)->Flags().IsSet(OF_UPDATE) ? &br_sym : &br_sym_lo);
}

void C3ObjectListCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CMemDC mdc(dc, this);

	Gdiplus::Graphics gr(mdc.GetDC());

	Gdiplus::SolidBrush br_bg(Gdiplus::Color(255, 40, 40, 40));

	CRect r;
	GetClientRect(r);
	gr.FillRectangle(&br_bg, r.left, r.top, r.Width(), r.Height());

	DefWindowProc(WM_PAINT, (WPARAM)(mdc.GetDC().GetSafeHdc()), (LPARAM)0);

	dc.BitBlt(0, 0, r.right, r.bottom, &(mdc.GetDC()), 0, 0, SRCCOPY);
}

void C3ObjectListCtrl::MeasureItem(LPMEASUREITEMSTRUCT pmi)
{
	CRect r;
	GetClientRect(r);
	pmi->itemHeight = 36;
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
	// TODO: Add your message handler code here and/or call default

	CListCtrl::OnRButtonUp(nFlags, point);
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
		pv->ClearSelection();
		pv->AddToSelection(pobj);
	}
	else
	{
		if (pv->IsSelected(pobj))
			pv->RemoveFromSelection(pobj);
		else
			pv->AddToSelection(pobj);
	}

	UpdateData();

	*pResult = 0;
}
