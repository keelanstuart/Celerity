// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"
#include "C3Edit.h"
#include "EditPrototypeDlg.h"
#include "afxdialogex.h"


// CEditPrototypeDlg dialog

IMPLEMENT_DYNAMIC(CEditPrototypeDlg, CDialog)

CEditPrototypeDlg::CEditPrototypeDlg(c3::Prototype *pproto, CWnd* pParent) : CDialog(IDD_EDITPROTOTYPE, pParent)
{
	m_pProto = pproto;
	m_pTempProps = props::IPropertySet::CreatePropertySet();
	if (m_pProto)
		m_pTempProps->AppendPropertySet(pproto->GetProperties());
}

CEditPrototypeDlg::~CEditPrototypeDlg()
{
	if (m_pTempProps)
	{
		m_pTempProps->Release();
		m_pTempProps = nullptr;
	}
}

void CEditPrototypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate)
	{
		DDX_Control(pDX, IDC_NAME, m_wndNameEdit);
		DDX_Control(pDX, IDC_PROPERTIES, m_wndPropList);
		DDX_Control(pDX, IDC_COMPONENTS, m_wndCompList);
		DDX_Control(pDX, IDC_FLAGS, m_wndFlagList);
	}
}


BEGIN_MESSAGE_MAP(CEditPrototypeDlg, CDialog)
END_MESSAGE_MAP()


// CEditPrototypeDlg message handlers


BOOL CEditPrototypeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_pProto)
		return FALSE;

	m_wndNameEdit.SetWindowText(m_pProto->GetName());

	for (size_t i = 0, maxi = theApp.m_C3->GetFactory()->GetNumComponentTypes(); i < maxi; i++)
	{
		c3::ComponentType *pct = (c3::ComponentType *)theApp.m_C3->GetFactory()->GetComponentType(i);
		if (!pct)
			continue;

		int ic = m_wndCompList.AddString(pct->GetName());
		m_wndCompList.SetItemDataPtr(ic, (void *)pct);
		m_wndCompList.SetCheck(ic, m_pProto->HasComponent(pct));
	}

	for (size_t i = 0, maxi = FlagInfo.size(); i < maxi; i++)
	{
		int ic = m_wndFlagList.AddString(FlagInfo[i].first);
		m_wndFlagList.SetItemData(ic, FlagInfo[i].second);
		m_wndFlagList.SetCheck(ic, m_pProto->Flags().IsSet(1LL << FlagInfo[i].second));
	}

	m_wndPropList.SetActiveProperties(m_pTempProps);
	m_wndPropList.SetGroupNameFullWidth(TRUE, FALSE);
	m_wndPropList.AdjustLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditPrototypeDlg::OnOK()
{
	if (m_pProto)
	{
		CString name;
		m_wndNameEdit.GetWindowText(name);
		m_pProto->SetName(name);

		for (int i = 0, maxi = m_wndCompList.GetCount(); i < maxi; i++)
		{
			const c3::ComponentType *pct = (const c3::ComponentType *)m_wndCompList.GetItemDataPtr(i);
			if (!pct)
				continue;

			if (m_pProto->HasComponent(pct) && !m_wndCompList.GetCheck(i))
				m_pProto->RemoveComponent(pct);
			else if (!m_pProto->HasComponent(pct) && m_wndCompList.GetCheck(i))
				m_pProto->AddComponent(pct);
		}

		for (int i = 0, maxi = m_wndFlagList.GetCount(); i < maxi; i++)
		{
			if (!m_wndFlagList.GetCheck(i))
				m_pProto->Flags().Set(1LL << m_wndFlagList.GetItemData(i));
			else
				m_pProto->Flags().Clear(1LL << m_wndFlagList.GetItemData(i));
		}

		m_pProto->GetProperties()->AppendPropertySet(m_pTempProps);
	}

	CDialog::OnOK();
}
