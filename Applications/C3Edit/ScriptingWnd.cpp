// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "ScriptingWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"


CScriptingWnd::CScriptingWnd() noexcept
{
}

CScriptingWnd::~CScriptingWnd()
{
}

BEGIN_MESSAGE_MAP(CScriptingWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_SCRIPT_NEW, OnNewScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_NEW, OnUpdateNewScript)
	ON_COMMAND(ID_SCRIPT_LOAD, OnLoadScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_LOAD, OnUpdateLoadScript)
	ON_COMMAND(ID_SCRIPT_SAVE, OnSaveScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_SAVE, OnUpdateSaveScript)
	ON_COMMAND(ID_SCRIPT_RUN, OnRunScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_RUN, OnUpdateRunScript)
END_MESSAGE_MAP()

int CScriptingWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SCRIPTING);
	m_wndToolBar.LoadToolBar(IDR_SCRIPTING, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(IDB_SCRIPTING_HC, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	m_wndTabs.Create(CMFCTabCtrl::STYLE_3D, r, this, 3, CMFCBaseTabCtrl::LOCATION_TOP);

	const DWORD dwStyle_e = WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_OEMCONVERT;

	m_wndCodeEditorImm.Create(dwStyle_e, r, &m_wndTabs, 1);
	m_wndCodeEditorImm.ModifyStyleEx(0, ES_EX_ZOOMABLE);
	m_wndCodeEditorImm.SetOptions(ECOOP_SET, ECO_AUTOWORDSELECTION | ECO_AUTOHSCROLL | ECO_AUTOVSCROLL | ECO_NOHIDESEL | ECO_SAVESEL | ECO_WANTRETURN);
	m_wndTabs.AddTab(&m_wndCodeEditorImm, _T("Immediate"), 0, FALSE);

	m_wndCodeEditorRes.Create(dwStyle_e, r, &m_wndTabs, 2);
	m_wndCodeEditorRes.ModifyStyleEx(0, ES_EX_ZOOMABLE);
	m_wndCodeEditorRes.SetOptions(ECOOP_SET, ECO_AUTOWORDSELECTION | ECO_AUTOHSCROLL | ECO_AUTOVSCROLL | ECO_NOHIDESEL | ECO_SAVESEL | ECO_WANTRETURN);
	m_wndTabs.AddTab(&m_wndCodeEditorRes, _T("Resource"), 1, FALSE);

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

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	m_wndToolBar.SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTabs.SetWindowPos(nullptr, rc.left, rc.top + cyTlb, rc.Width(), rc.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CScriptingWnd::OnNewScript()
{

}

void CScriptingWnd::OnUpdateNewScript(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable();
}

void CScriptingWnd::OnLoadScript()
{

}

void CScriptingWnd::OnUpdateLoadScript(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable();
}

void CScriptingWnd::OnSaveScript()
{

}

void CScriptingWnd::OnUpdateSaveScript(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable();
}

void CScriptingWnd::OnRunScript()
{
	MessageBox(L"", L"",  MB_OK);
}

void CScriptingWnd::OnUpdateRunScript(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable();
}

void CScriptingWnd::SetResourceScript(c3::Resource *pres)
{
	if (pres && pres->GetData() && (pres->GetType() == theApp.m_C3->GetResourceManager()->FindResourceTypeByName(_T("Script"))))
	{
		tstring *scr = (tstring *)(pres->GetData());
#if 0
		m_wndCodeEditorRes.SetSel(0, -1);
		m_wndCodeEditorRes.ReplaceSel(scr->c_str());
#else
		m_wndCodeEditorRes.SetWindowText(scr->c_str());
		m_wndCodeEditorRes.UpdateWindow();
#endif
	}
}
