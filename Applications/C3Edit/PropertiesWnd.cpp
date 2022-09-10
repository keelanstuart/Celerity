
#include "pch.h"
#include "framework.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "C3EditFrame.h"
#include "C3Edit.h"

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
}

CPropertiesWnd::~CPropertiesWnd()
{
}

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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	RedrawWindow(nullptr, nullptr, RDW_ERASE | RDW_ERASENOW | RDW_INVALIDATE);

	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rc;
	GetClientRect(rc);

	int lh = rc.Height() / 6;
	#define CONTROL_SPACING		0

	m_wndNameEdit.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);
	m_wndCompList.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);
	m_wndFlagList.ShowWindow((m_pProto || m_pObj) ? SW_NORMAL : SW_HIDE);

	if (m_pProto || m_pObj)
	{
		CRect rne(0, 0, 0, 21);
		m_wndNameEdit.SetWindowPos(nullptr, rc.left, rc.top, rc.Width() - CONTROL_SPACING, rne.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

		rc.DeflateRect(0, rne.Height() + CONTROL_SPACING, 0,  0);
		m_wndCompList.SetWindowPos(nullptr, rc.left, rc.top, rc.Width() - CONTROL_SPACING, lh, SWP_NOACTIVATE | SWP_NOZORDER);

		rc.DeflateRect(0, lh + CONTROL_SPACING, 0,  0);
		m_wndFlagList.SetWindowPos(nullptr, rc.left, rc.top, rc.Width() - CONTROL_SPACING, lh, SWP_NOACTIVATE | SWP_NOZORDER);

		rc.DeflateRect(0, lh + CONTROL_SPACING, 0,  0);
	}

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	m_wndToolBar.SetWindowPos(nullptr, rc.left, rc.top, rc.Width() - CONTROL_SPACING, cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rc.left, rc.top + cyTlb, rc.Width() - CONTROL_SPACING, rc.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

#define PWID_EDITNAME		1
#define PWID_COMPLIST		2
#define PWID_FLAGLIST		3
#define PWID_PROPLIST		4

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndNameEdit.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, PWID_EDITNAME))
		return -1;      // fail to create

	if (!m_wndCompList.Create(WS_VISIBLE | WS_CHILD | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, rectDummy, this, PWID_COMPLIST))
		return -1;      // fail to create

	if (!m_wndFlagList.Create(WS_VISIBLE | WS_CHILD | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, rectDummy, this, PWID_FLAGLIST))
		return -1;      // fail to create

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, PWID_PROPLIST))
		return -1;      // fail to create

	for (auto f : FlagInfo)
	{
		int ic = m_wndFlagList.AddString(f.first);
		m_wndFlagList.SetItemData(ic, f.second);
	}

	m_wndNameEdit.SetFont(m_wndPropList.GetFont());
	m_wndFlagList.SetFont(m_wndPropList.GetFont());
	m_wndCompList.SetFont(m_wndPropList.GetFont());

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
		m_wndPropList.SetActiveProperties(m_pProto->GetProperties());
		m_wndPropList.ExpandAll(m_bExpanded);
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
		m_wndPropList.ExpandAll(m_bExpanded);
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
		m_wndPropList.ExpandAll(m_bExpanded);
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
		m_wndFlagList.SetCheck(i, f ? (f->IsSet(1LL << m_wndFlagList.GetItemData(i))) : 0);
	}
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
		m_wndCompList.SetItemDataPtr(ic, (void *)pct);

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
}
