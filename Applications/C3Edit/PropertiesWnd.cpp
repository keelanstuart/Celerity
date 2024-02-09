// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"
#include "framework.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"
#include "C3Edit.h"
#include <gdiplus.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd() noexcept
{
	m_pProto = nullptr;
	m_pObj = nullptr;
	m_pProps = nullptr;
	m_bExpanded = false;

	InitializeCriticalSection(&m_PropLock);
}

CPropertiesWnd::~CPropertiesWnd()
{
	DeleteCriticalSection(&m_PropLock);
}

#define PWID_EDITNAME		1
#define PWID_COMPLIST		2
#define PWID_FLAGLIST		3
#define PWID_PROPLIST		4

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_CTLCOLORLISTBOX, &CPropertiesWnd::OnCtlcolorlistbox)
	ON_CLBN_CHKCHANGE(PWID_FLAGLIST, OnCheckChangeFlags)
	ON_CLBN_CHKCHANGE(PWID_COMPLIST, OnCheckChangeComponents)
	ON_EN_CHANGE(PWID_EDITNAME, OnChangeName)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rc;
	GetClientRect(rc);

	int lh = rc.Height() / 6;
	#define CTRL_SPACE		4
	#define EDIT_HT			21

	if (m_pProto || m_pObj)
	{
		CRect rne = rc;
		rne.left = EDIT_HT / 2;
		rne.bottom = rne.top + EDIT_HT;
		rne.OffsetRect(0, EDIT_HT);
		m_wndNameEdit.SetWindowPos(nullptr, rne.left, rne.top, rne.Width(), rne.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

		CRect rcl = rc;
		rcl.top = rne.bottom + EDIT_HT + CTRL_SPACE;
		rcl.bottom = rcl.top + lh;
		m_wndCompList.SetWindowPos(nullptr, rcl.left, rcl.top, rcl.Width(), rcl.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

		CRect rfl = rc;
		rfl.top = rcl.bottom;
		rfl.bottom = rfl.top + lh;
		m_wndFlagList.SetWindowPos(nullptr, rfl.left, rfl.top, rfl.Width(), rfl.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

		rc.top = rfl.bottom + 2;
	}

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	m_wndToolBar.SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rc.left, rc.top + cyTlb, rc.Width(), rc.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndNameEdit.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);
	m_wndCompList.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);
	m_wndFlagList.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);

	RedrawWindow(nullptr, nullptr, RDW_ERASE | RDW_ERASENOW | RDW_INVALIDATE);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	if (!m_wndNameEdit.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, r, this, PWID_EDITNAME))
		return -1;      // fail to create

	if (!m_wndCompList.Create(WS_VISIBLE | WS_CHILD | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, r, this, PWID_COMPLIST))
		return -1;      // fail to create

	if (!m_wndFlagList.Create(WS_VISIBLE | WS_CHILD | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, r, this, PWID_FLAGLIST))
		return -1;      // fail to create

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, r, this, PWID_PROPLIST))
		return -1;      // fail to create

	for (auto f : FlagInfo)
	{
		int ic = m_wndFlagList.AddString(f.first);
		m_wndFlagList.SetItemData(ic, f.second);
	}
	m_wndFlagList.SetFont(m_wndPropList.GetFont());

	m_wndNameEdit.SetFont(m_wndPropList.GetFont());

	m_wndPropList.SetCustomColors(RGB(64, 64, 64), RGB(255, 255, 255), RGB(64, 64, 64), RGB(255, 255, 255), RGB(0, 0, 0), RGB(200, 200, 200), RGB(128, 128, 128));
	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(IDB_PROPERTIES_HC, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_bExpanded ^= true;
	m_wndPropList.ExpandAll(m_bExpanded);
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
	m_wndPropList.EnableDescriptionArea(FALSE);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
}

void CPropertiesWnd::SetActivePrototype(c3::Prototype *pproto)
{
	m_pObj = nullptr;
	m_pProto = pproto;
	m_pProps = nullptr;

	m_wndNameEdit.SetWindowText(m_pProto ? m_pProto->GetName() : _T(""));

	if (m_wndPropList.GetSafeHwnd())
	{
		m_wndPropList.SetActiveProperties(m_pProto ? m_pProto->GetProperties() : nullptr);
	}

	FillOutFlags();
	FillOutComponents();

	AdjustLayout();
}

void CPropertiesWnd::SetActiveObject(c3::Object *pobj)
{
	m_pObj = pobj;
	m_pProto = nullptr;
	m_pProps = nullptr;

	m_wndNameEdit.SetWindowText(m_pObj ? m_pObj->GetName() : _T(""));

	if (m_wndPropList.GetSafeHwnd())
	{
		m_wndPropList.SetActiveProperties(m_pObj ? m_pObj->GetProperties() : nullptr);
	}

	FillOutFlags();
	FillOutComponents();

	AdjustLayout();
}

void CPropertiesWnd::SetActiveProperties(props::IPropertySet* props, bool readonly, const TCHAR* title)
{
	m_pObj = nullptr;
	m_pProto = nullptr;
	m_pProps = props;

	if (m_wndPropList.GetSafeHwnd())
	{
		m_wndPropList.SetActiveProperties(props);
	}

	AdjustLayout();
}


void CPropertiesWnd::FillOutFlags()
{
	props::TFlags64 *f = nullptr;
	if (m_pProto)
		f = &(m_pProto->Flags());
	else if (m_pObj)
		f = &(m_pObj->Flags());

	for (int i = 0, maxi = m_wndFlagList.GetCount(); i < maxi; i++)
	{
		m_wndFlagList.SetCheck(i, f ? (f->IsSet(m_wndFlagList.GetItemData(i))) : 0);
	}

	m_wndFlagList.RedrawWindow(0, 0, RDW_ALLCHILDREN | RDW_ERASENOW | RDW_INTERNALPAINT | RDW_VALIDATE | RDW_UPDATENOW);
}


void CPropertiesWnd::FillOutComponents()
{
	m_wndCompList.ResetContent();
	for (size_t j = 0, maxj = theApp.m_C3->GetFactory()->GetNumComponentTypes(); j < maxj; j++)
	{
		c3::ComponentType *pct = (c3::ComponentType *)theApp.m_C3->GetFactory()->GetComponentType(j);
		if (!pct)
			continue;

		int ic = m_wndCompList.AddString(pct->GetName());
		m_wndCompList.SetItemData(ic, (DWORD_PTR)pct);
		//assert((DWORD_PTR)pct == m_wndCompList.GetItemData(ic));

		if (m_pProto)
		{
			m_wndCompList.SetCheck(ic, m_pProto->HasComponent(pct));
		}
		else if (m_pObj)
		{
			m_wndCompList.SetCheck(ic, m_pObj->HasComponent(pct));
		}
		else
		{
			m_wndCompList.SetCheck(ic, FALSE);
		}
	}

	m_wndCompList.SetFont(m_wndPropList.GetFont());
}


HBRUSH CPropertiesWnd::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDockablePane::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}


BOOL CPropertiesWnd::OnEraseBkgnd(CDC *pDC)
{
	HDC hdc = pDC->GetSafeHdc();
	if (!hdc)
		return true;

	CRect r;
	GetClientRect(r);
	pDC->FillSolidRect(r, RGB(64, 64, 64));

	if (m_pObj || m_pProto)
	{
		CPaintDC *pdc = (CPaintDC *)CDC::FromHandle(pDC->GetSafeHdc());

		CRect rc, r;
		m_wndNameEdit.GetWindowRect(rc);
		ScreenToClient(rc);

		Gdiplus::RectF tnr;

		// Get GUID
		TCHAR gs[128];
		INT gslen;
		GUID g;

		if (m_pObj)
		{
			_tcscpy_s(gs, _T("Object Name"));
			g = m_pObj->GetGuid();
		}
		else if (m_pProto)
		{
			_tcscpy_s(gs, _T("Prototype Name"));
			g = m_pProto->GetGUID();
		}

		COLORREF old_color = pdc->SetTextColor(RGB(255,255,255));

		r = rc;
		r.left = 1;
		r.OffsetRect(0, 2 - r.Height());
		gslen = (INT)_tcslen(gs);
		pdc->DrawText(gs, gslen, r, DT_LEFT);

		gslen = _stprintf_s(gs, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
								g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

		// DRAW OBJECT GUID
		pdc->SetTextColor(RGB(128,128,128));
		r = rc;
		r.OffsetRect(0, r.Height());
		pdc->DrawText(gs, gslen, r, DT_LEFT | DT_WORD_ELLIPSIS);

		pdc->SetTextColor(old_color);
	}

	return false;//CDockablePane::OnEraseBkgnd(pDC);
}

afx_msg LRESULT CPropertiesWnd::OnCtlcolorlistbox(WPARAM wParam, LPARAM lParam)
{
	HDC hdc = ::GetDC((HWND)lParam);
	CDC dc;
	dc.Attach(hdc);
	COLORREF bkColor = RGB(64, 64, 64);
	dc.SetBkColor(bkColor);
	return (LRESULT)CreateSolidBrush(bkColor);
}


afx_msg void CPropertiesWnd::OnCheckChangeFlags()
{
	props::TFlags64 f = 0;

	for (int i = 0, maxi = m_wndFlagList.GetCount(); i < maxi; i++)
	{
		if (m_wndFlagList.GetCheck(i) == BST_CHECKED)
			f.Set(m_wndFlagList.GetItemData(i));
	}

	if (m_pProto)
	{
		m_pProto->Flags().SetAll(f);
	}
	else if (m_pObj)
	{
		m_pObj->Flags().SetAll(f);
		m_pObj->Flags().Set(OF_SCLCHANGED);
	}

	C3EditFrame *pef = (C3EditFrame *)theApp.m_pMainWnd;
	if (pef->GetSafeHwnd() && pef->m_wndObjects.GetSafeHwnd())
		pef->m_wndObjects.UpdateContents();
}


afx_msg void CPropertiesWnd::OnCheckChangeComponents()
{
	EnterCriticalSection(&m_PropLock);

	for (int i = 0, maxi = m_wndCompList.GetCount(); i < maxi; i++)
	{
		// this is awful... for whatever reason, WtfCheckListBox is using item data (?) so for now, look up components by name
		CString name;
		m_wndCompList.GetText(i, name);
		const c3::ComponentType *pct = theApp.m_C3->GetFactory()->FindComponentType(name);

		if (m_wndCompList.GetCheck(i) == BST_CHECKED)
		{
			if (m_pProto && !m_pProto->HasComponent(pct))
				m_pProto->AddComponent(pct);
			else if (m_pObj && !m_pObj->HasComponent(pct))
				m_pObj->AddComponent(pct);
		}
		else
		{
			if (m_pProto && m_pProto->HasComponent(pct))
				m_pProto->RemoveComponent(pct);
			else if (m_pObj && m_pObj->HasComponent(pct))
			{
				c3::Component *pc = m_pObj->FindComponent(pct);
				if (pc)
					m_pObj->RemoveComponent(pc);
			}
		}
	}

	m_RebuildProps = true;

	LeaveCriticalSection(&m_PropLock);
}

afx_msg void CPropertiesWnd::OnChangeName()
{
	CString name;
	m_wndNameEdit.GetWindowText(name);

	if (m_pProto)
	{
		m_pProto->SetName(name);
		((C3EditFrame *)theApp.GetMainWnd())->m_wndProtoView.UpdateData();
	}
	else if (m_pObj)
	{
		m_pObj->SetName(name);
		((C3EditFrame *)theApp.GetMainWnd())->m_wndObjects.UpdateData();
	}
}


void CPropertiesWnd::UpdateCurrentProperties()
{
	if (TryEnterCriticalSection(&m_PropLock))
	{
		if (!m_RebuildProps)
		{
			m_wndPropList.UpdateCurrentProperties();
		}
		else
		{
			m_wndPropList.SetActiveProperties(nullptr);
			m_wndPropList.SetActiveProperties(m_pObj ? m_pObj->GetProperties() : (m_pProto ? m_pProto->GetProperties() : nullptr));
			m_RebuildProps = false;
		}

		LeaveCriticalSection(&m_PropLock);
	}
}
