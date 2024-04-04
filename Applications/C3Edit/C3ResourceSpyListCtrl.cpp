// C3ResourceSpyListCtrl.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "C3EditFrame.h"
#include "C3EditDoc.h"
#include "C3EditView.h"
#include "C3ResourceSpyListCtrl.h"


// C3ResourceSpyListCtrl

IMPLEMENT_DYNAMIC(C3ResourceSpyListCtrl, CListCtrl)

C3ResourceSpyListCtrl::C3ResourceSpyListCtrl()
{

}

C3ResourceSpyListCtrl::~C3ResourceSpyListCtrl()
{
}


BEGIN_MESSAGE_MAP(C3ResourceSpyListCtrl, CListCtrl)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_RESOURCE_RELOAD, &C3ResourceSpyListCtrl::OnUpdateResourceReload)
	ON_COMMAND(ID_RESOURCE_RELOAD, &C3ResourceSpyListCtrl::OnResourceReload)
END_MESSAGE_MAP()



// C3ResourceSpyListCtrl message handlers




void C3ResourceSpyListCtrl::DrawItem(LPDRAWITEMSTRUCT pdi)
{
	CPaintDC *pdc = (CPaintDC *)CDC::FromHandle(pdi->hDC);

	Gdiplus::Graphics gr(*pdc);

	Gdiplus::SolidBrush br_selected(Gdiplus::Color(255, 45, 45, 192));
	Gdiplus::SolidBrush br_bg_dark(Gdiplus::Color(255, 64, 64, 64));
	Gdiplus::SolidBrush br_bg(Gdiplus::Color(255, 80, 80, 80));
	Gdiplus::SolidBrush br_text(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush br_text_dark(Gdiplus::Color(255, 128, 128, 128));
	Gdiplus::SolidBrush br_text_seldark(Gdiplus::Color(255, 160, 160, 192));

	CRect ritem = pdi->rcItem, rcol[4];

	GetSubItemRect(pdi->itemID, 0, LVIR_BOUNDS, rcol[0]);
	GetSubItemRect(pdi->itemID, 1, LVIR_BOUNDS, rcol[1]);
	GetSubItemRect(pdi->itemID, 2, LVIR_BOUNDS, rcol[2]);
	GetSubItemRect(pdi->itemID, 3, LVIR_BOUNDS, rcol[3]);

	if (pdi->itemState & ODS_SELECTED)
	{
		gr.FillRectangle(&br_selected, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}
	else
	{
		gr.FillRectangle((pdi->itemID & 1) ? &br_bg_dark : &br_bg, ritem.left, ritem.top, ritem.Width(), ritem.Height());
	}

	Gdiplus::Font f_list(pdi->hDC);
	LOGFONT lf_guid;
	f_list.GetLogFontW(&gr, &lf_guid);
	lf_guid.lfWeight = FW_EXTRALIGHT;
	lf_guid.lfHeight = lf_guid.lfHeight;
	Gdiplus::Font f_guid(pdi->hDC, &lf_guid);

	Gdiplus::Rect rg;
	Gdiplus::RectF tnr;

	TCHAR s[512];
	size_t slen;

	// DRAW FILENAME
	slen = GetItemText(pdi->itemID, 0, s, 512);
	rg = Gdiplus::Rect(rcol[0].left, rcol[0].top, rcol[1].left - 1, rcol[0].Height());
	gr.SetClip(rg);
	gr.DrawString(s, (INT)slen, &f_list, Gdiplus::PointF((Gdiplus::REAL)rcol[0].left, (Gdiplus::REAL)rcol[0].top), &br_text);

	// DRAW TYPE
	slen = GetItemText(pdi->itemID, 1, s, 512);
	rg = Gdiplus::Rect(rcol[1].left, rcol[1].top, rcol[2].left - 1, rcol[1].Height());
	gr.SetClip(rg);
	gr.DrawString(s, (INT)slen, &f_list, Gdiplus::PointF((Gdiplus::REAL)rcol[1].left, (Gdiplus::REAL)rcol[1].top), &br_text);

	// DRAW STATUS
	slen = GetItemText(pdi->itemID, 2, s, 512);
	rg = Gdiplus::Rect(rcol[2].left, rcol[2].top, rcol[3].left - 1, rcol[2].Height());
	gr.SetClip(rg);
	gr.DrawString(s, (INT)slen, &f_list, Gdiplus::PointF((Gdiplus::REAL)rcol[2].left, (Gdiplus::REAL)rcol[2].top), &br_text);

	// DRAW OPTIONS
	slen = GetItemText(pdi->itemID, 3, s, 512);
	rg = Gdiplus::Rect(rcol[3].left, rcol[3].top, rcol[0].right, rcol[3].Height());
	gr.SetClip(rg);
	gr.DrawString(s, (INT)slen, &f_list, Gdiplus::PointF((Gdiplus::REAL)rcol[3].left, (Gdiplus::REAL)rcol[3].top), &br_text);
}


void C3ResourceSpyListCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_RESOURCESPY, point.x, point.y, this, TRUE);
#endif
}



void C3ResourceSpyListCtrl::OnUpdateResourceReload(CCmdUI *pCmdUI)
{
	c3::ResourceManager *prm = theApp.m_C3->GetResourceManager();

	POSITION sp = GetFirstSelectedItemPosition();
	c3::Resource *pr = prm->GetResourceByIndex(GetNextSelectedItem(sp));

	pCmdUI->Enable(pr != nullptr);
}


void C3ResourceSpyListCtrl::OnResourceReload()
{
	c3::ResourceManager *prm = theApp.m_C3->GetResourceManager();

	POSITION sp = GetFirstSelectedItemPosition();
	for (int i = 0, maxi = GetSelectedCount(); i < maxi; i++)
	{
		c3::Resource *pr = prm->GetResourceByIndex(GetNextSelectedItem(sp));

		if (pr)
		{
			pr->DelRef();
			pr->DelRef();
			pr->DelRef();

			pr->AddRef();
		}
	}
}
