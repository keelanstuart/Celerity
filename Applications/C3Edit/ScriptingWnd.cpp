// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "ScriptingWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"


CScriptingWnd::CScriptingWnd() noexcept
{
	m_pRes = nullptr;
}

CScriptingWnd::~CScriptingWnd()
{
}

BEGIN_MESSAGE_MAP(CScriptingWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CScriptingWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	m_wndTabs.Create(CMFCTabCtrl::STYLE_3D, r, this, 3, CMFCBaseTabCtrl::LOCATION_TOP);

	const DWORD dwStyle_e = WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_OEMCONVERT;

	m_wndCodeEditorImm.Create(dwStyle_e, r, &m_wndTabs, 700);
	m_wndTabs.AddTab(&m_wndCodeEditorImm, _T("Immediate"), 100, FALSE);

	m_wndCodeEditorRes.Create(dwStyle_e, r, &m_wndTabs, 800);
	m_wndTabs.AddTab(&m_wndCodeEditorRes, _T("Resource"), 200, FALSE);

	return 0;
}



void CScriptingWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CScriptingWnd::AdjustLayout()
{
	CRect rc;
	GetClientRect(rc);

	m_wndTabs.SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


void CScriptingWnd::EditScriptResource(c3::Resource *pres)
{
	m_wndCodeEditorRes.SetSel(0, -1);

	if (pres && pres->GetData() && (pres->GetType() == theApp.m_C3->GetResourceManager()->FindResourceTypeByName(_T("Script"))))
	{
		m_pRes = pres;

		tstring *scr = (tstring *)(pres->GetData());

		m_wndCodeEditorRes.ReplaceSel(scr->c_str());
	}
	else
	{
		m_wndCodeEditorRes.ReplaceSel(_T(""));
	}

	m_wndCodeEditorRes.SetSel(0, 0);
}

bool CScriptingWnd::IsEditingScriptResource(c3::Resource *pres, int &tabidx)
{
	return false;
}


BOOL CScriptingWnd::PreTranslateMessage(MSG *pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

BOOL CScriptingWnd::OnShowControlBarMenu(CPoint point)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	if(rc.PtInRect(point))
		return FALSE;

	return CDockablePane::OnShowControlBarMenu(point);
}

HBRUSH CScriptingWnd::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDockablePane::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}


BOOL CScriptingWnd::OnEraseBkgnd(CDC *pDC)
{
	CRect r;
	GetClientRect(r);
	pDC->FillSolidRect(r, RGB(64, 64, 64));

	return false;//CDockablePane::OnEraseBkgnd(pDC);
}


void CScriptingWnd::OnSetFocus(CWnd *pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	int t = m_wndTabs.GetActiveTab();
	switch (t)
	{
		case 0:
			m_wndCodeEditorImm.SetFocus();
			break;
		case 1:
			m_wndCodeEditorRes.SetFocus();
			break;
	}
}


bool CScriptingWnd::ImmediateScriptEmpty()
{
	if (!m_wndCodeEditorImm.GetSafeHwnd())
		return true;

	CString s;
	m_wndCodeEditorImm.GetWindowText(s);

	return s.IsEmpty();
}


bool CScriptingWnd::ResourceScriptChanged()
{
	return m_wndCodeEditorRes.GetSafeHwnd() ? m_wndCodeEditorRes.GetModify() : false;
}


void CScriptingWnd::UpdateResourceScript()
{
	if (!m_pRes)
		return;

	CString s;
	GetResourceScript(s);

	FILE *f;
	if (_tfopen_s(&f, m_pRes->GetFilename(), _T("wt")) != EINVAL)
	{
		char *ss;
		CONVERT_TCS2MBCS((LPCTSTR)s, ss);
		fprintf(f, ss);

		fclose(f);
	}

	m_pRes->DelRef();
}


void CScriptingWnd::AssignImmediateScript()
{

}

void CScriptingWnd::GetImmediateScript(CString &s)
{
	m_wndCodeEditorImm.GetWindowText(s);

	// The richedit uses the \r\n Windowsy convention for newlines - just use \n though
	s.Remove(_T('\r'));
}


void CScriptingWnd::GetResourceScript(CString &s)
{
	m_wndCodeEditorRes.GetWindowText(s);

	// The richedit uses the \r\n Windowsy convention for newlines - just use \n though
	s.Remove(_T('\r'));
}
