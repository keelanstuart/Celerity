// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "ResourceSpyWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"


CResourceSpyWnd::CResourceSpyWnd() noexcept
{
	m_Frame = 0;
}

CResourceSpyWnd::~CResourceSpyWnd()
{
}

#define CTLID_RESSPY_LIST		1

BEGIN_MESSAGE_MAP(CResourceSpyWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, CTLID_RESSPY_LIST, &CResourceSpyWnd::OnNotifyListData)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

int CResourceSpyWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	DWORD cs = WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_OWNERDATA;

	if (!m_ResList.CreateEx(0, cs, r, this, CTLID_RESSPY_LIST))
		return -1;

	m_ResList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_ResList.InsertColumn(0, _T("Filename"), LVCFMT_LEFT, 200, 0);
	m_ResList.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 100, 1);
	m_ResList.InsertColumn(2, _T("Status"), LVCFMT_LEFT, 80, 2);
	m_ResList.InsertColumn(3, _T("Options"), LVCFMT_LEFT, 100, 3);

	COLORREF background = RGB(64, 64, 64);
	m_ResList.SetBkColor(background);
	m_ResList.SetTextBkColor(background);
	m_ResList.SetTextColor(RGB(255, 255, 255));

	ModifyStyle(0, WS_CLIPCHILDREN);

	AdjustLayout();
	SetTimer('RSPY', 500, nullptr);

	return 0;
}

void CResourceSpyWnd::OnDestroy()
{
	KillTimer('RSPY');

	__super::OnDestroy();
}

void CResourceSpyWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CResourceSpyWnd::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect r;
	GetClientRect(r);

	m_ResList.SetWindowPos(nullptr, r.left, r.top, r.Width(), r.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BOOL CResourceSpyWnd::PreTranslateMessage(MSG *pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}


BOOL CResourceSpyWnd::OnShowControlBarMenu(CPoint point)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);

	if(rc.PtInRect(point))
		return FALSE;

	return CDockablePane::OnShowControlBarMenu(point);
}


HBRUSH CResourceSpyWnd::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDockablePane::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}


BOOL CResourceSpyWnd::OnEraseBkgnd(CDC *pDC)
{
	CRect r;
	GetClientRect(r);
	pDC->FillSolidRect(r, RGB(64, 64, 64));

	return false;
}


void CResourceSpyWnd::OnSetFocus(CWnd *pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}


void CResourceSpyWnd::OnNotifyListData(NMHDR *phdr, LRESULT *pres)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)phdr;
	LVITEM *pItem = &(pDispInfo)->item;

	int i = pItem->iItem;

	c3::ResourceManager *prm = theApp.m_C3->GetResourceManager();
	c3::Resource *pr = prm->GetResourceByIndex(i);

	const TCHAR *status_text[c3::Resource::Status::RS_NUMSTATUSES] = {_T("None"), _T("Loaded"), _T("Loading"), _T("Not Found"), _T("Error") };

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		if (!pr)
		{
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, _T(""));
			return;
		}

		const TCHAR *src = nullptr;
		switch (pItem->iSubItem)
		{
			case 0: //fill in main text with filename
				src = pr->GetFilename();
				break;

			case 1: //fill in sub item 1 text with type
				src = pr->GetType()->GetName();
				break;

			case 2: //fill in sub item 2 text with status
				src = status_text[pr->GetStatus()];
				break;

			case 3: //fill in sub item 2 text with optiosn
				src = pr->GetOptions();
				break;
		}

		_tcsncpy_s(pItem->pszText, pItem->cchTextMax, src ? src : _T(""), std::min(pItem->cchTextMax - 1, pItem->cchTextMax));
	}
}


void CResourceSpyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_ResList.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}


void CResourceSpyWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ((nIDEvent == 'RSPY') && (m_ResList.GetSafeHwnd() != nullptr) &&	(theApp.m_C3 != nullptr))
	{
		c3::ResourceManager *prm = theApp.m_C3->GetResourceManager();

		if (prm)
		{
			int count = (int)prm->GetNumResources();

			if (count != m_ResList.GetItemCount())
				m_ResList.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

			m_ResList.Invalidate(FALSE);
		}
	}

	CDockablePane::OnTimer(nIDEvent);
}