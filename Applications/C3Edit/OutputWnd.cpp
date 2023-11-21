// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CDebugOutputEdit, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH CDebugOutputEdit::CtlColor(CDC *pDC, UINT)
{
	COLORREF bkColor = RGB(64, 64, 64);
	pDC->SetBkColor(bkColor);
	pDC->SetTextColor(RGB(255, 255, 255));
	return CreateSolidBrush(bkColor);
}

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd() noexcept
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	// Create output panes:
	const DWORD dwStyle_ol = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LBS_OWNERDRAWFIXED;// | LBS_NOINTEGRALHEIGHT;
	const DWORD dwStyle_e = ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputDebug.Create(dwStyle_e, r, this, 2))
	{
		TRACE0("Failed to create output window\n");
		return -1;      // fail to create
	}

	UpdateFonts();

	theApp.m_C3->GetLog()->SetRedirectFunction([](void *userdata, const TCHAR *msg)
	{
		assert(msg);

		CEdit *pw = (CEdit *)userdata;
		CPoint cpos = pw->GetCaretPos();

		// we have to replace all '\n' with "\r\n" pairs

		// count '\n' chars
		size_t sl = 1;
		TCHAR *c = (TCHAR *)msg;
		while (*c)
		{
			sl += (*c != _T('\n')) ? 1 : 2;
			c++;
		}

		// allocate a new string -- on the stack
		TCHAR *m = (TCHAR *)_alloca(sl * sizeof(TCHAR));
		c = m;
		while (*msg)
		{
			if (*msg == _T('\n'))
				*(c++) = _T('\r');	// insert a '\r'

			*(c++)  = *(msg++);
		}
		*c = _T('\0');

		pw->SetSel(-1, -1);
		pw->ReplaceSel(m);
		pw->SetSel(-1, -1);
	}, &m_wndOutputDebug);

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndOutputDebug.SetWindowPos (nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::FillDebugWindow()
{
}

void COutputWnd::UpdateFonts()
{
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
}



BOOL COutputWnd::DestroyWindow()
{
	theApp.m_C3->GetLog()->SetRedirectFunction(nullptr, nullptr);

	return CDockablePane::DestroyWindow();
}


void COutputWnd::OnClose()
{
	theApp.m_C3->GetLog()->SetRedirectFunction(nullptr, nullptr);

	CDockablePane::OnClose();
}
