// GraphEditorDlg.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "GraphView.h"
#include "GraphEditorDlg.h"
#include "afxdialogex.h"


// CGraphEditorDlg dialog


CGraphEditorDlg *CGraphEditorDlg::DoModeless(CWnd *pParent)
{
	CGraphEditorDlg *ret = new CGraphEditorDlg(pParent);
	if (ret)
	{
		ret->Create(IDD_GRAPHEDITOR, pParent);
		ret->ShowWindow(SW_SHOW);
	}

	return ret;
}


IMPLEMENT_DYNAMIC(CGraphEditorDlg, CDialog)

CGraphEditorDlg::CGraphEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_GRAPHEDITOR, pParent)
{
}

CGraphEditorDlg::~CGraphEditorDlg()
{
	delete m_GraphView;
}

void CGraphEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphEditorDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CGraphEditorDlg message handlers


void CGraphEditorDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


BOOL CGraphEditorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect r;
	GetClientRect(r);
	r.DeflateRect(1, 1, 1, 1);

	m_GraphView.Create(nullptr, _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN, r, this, 1);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CGraphEditorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect r;
	GetClientRect(r);
	r.DeflateRect(1, 1, 1, 1);

	if (m_GraphView.GetSafeHwnd())
		m_GraphView.MoveWindow(r);
}
