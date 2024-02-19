// ResourcePathEditorDlg.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "ResourcePathEditorDlg.h"


// CResourcePathEditorDlg dialog

IMPLEMENT_DYNAMIC(CResourcePathEditorDlg, CDialog)

CResourcePathEditorDlg::CResourcePathEditorDlg(CWnd *pParent) : CDialog(CResourcePathEditorDlg::IDD, pParent)
{

}

CResourcePathEditorDlg::~CResourcePathEditorDlg()
{
}

void CResourcePathEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResourcePathEditorDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_RESTYPECB, &CResourcePathEditorDlg::OnSelchangeResType)
END_MESSAGE_MAP()


// CResourcePathEditorDlg message handlers


std::vector<tstring> typenames =
{
	_T("animations"),
	_T("animstates"),
	_T("models"),
	_T("prototypes"),
	_T("rendermethods"),
	_T("scripts"),
	_T("shaders"),
	_T("sounds"),
	_T("textures")
};


BOOL CResourcePathEditorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_RESTYPECB);
	if (!pcb)
		return FALSE;

	tstring s;
	for (auto t : typenames)
	{
		s = _T("resources.");
		s += t;

		cfgname_exts.push_back(s);
		cfgname_exts.back() += _T(".extensions");

		cfgname_paths.push_back(s);
		cfgname_paths.back() += _T(".paths");

		cfg_exts.push_back(theApp.m_Config->GetString(cfgname_exts.back().c_str(), _T("")));
		cfg_paths.push_back(theApp.m_Config->GetString(cfgname_paths.back().c_str(), _T("")));

		int i = pcb->AddString(t.c_str());
	}

	m_LastSel = (int)theApp.m_Config->GetInt(_T("gui.resource_path_editor.selection"), 0);
	pcb->SetCurSel(m_LastSel);

	UpdateExtList(cfg_exts.front().c_str());
	UpdatePathList(cfg_paths.front().c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CResourcePathEditorDlg::UpdateExtList(const TCHAR *exts)
{
	if (!exts)
		return;

	CVSListBox *pel = (CVSListBox *)GetDlgItem(IDC_EXTLIST);
	if (!pel)
		return;

	while (pel->GetCount())
		pel->RemoveItem(0);

	tstring e;
	e.reserve(_tcslen(exts));

	TCHAR c;
	while ((c = *exts) != 0)
	{
		while (((c = *exts) != 0) && (c != _T(';')))
		{
			e.push_back(c);
			exts++;
		}

		if (!e.empty())
		{
			pel->AddItem(CString(e.c_str()));
			e.clear();
		}

		if (*exts == _T(';'))
			exts++;
	}
}


void CResourcePathEditorDlg::CreateExtString(tstring &exts)
{
	exts.clear();

	CVSListBox *pel = (CVSListBox *)GetDlgItem(IDC_EXTLIST);
	if (!pel)
		return;

	for (int i = 0, maxi = pel->GetCount(); i < maxi; i++)
	{
		CString e = pel->GetItemText(i);
		exts += (const TCHAR *)e;
		if (i != (maxi - 1))
			exts += _T(';');
	}
}


void CResourcePathEditorDlg::UpdatePathList(const TCHAR *paths)
{
	if (!paths)
		return;

	CVSListBox *ppl = (CVSListBox *)GetDlgItem(IDC_PATHLIST);
	if (!ppl)
		return;

	while (ppl->GetCount())
		ppl->RemoveItem(0);

	tstring p;
	p.reserve(_tcslen(paths));

	TCHAR c;
	while ((c = *paths) != 0)
	{
		while (((c = *paths) != 0) && (c != _T(';')))
		{
			p.push_back(c);
			paths++;
		}

		if (!p.empty())
		{
			ppl->AddItem(CString(p.c_str()));
			p.clear();
		}

		if (*paths == _T(';'))
			paths++;
	}
}


void CResourcePathEditorDlg::CreatePathString(tstring &paths)
{
	paths.clear();

	CVSListBox *ppl = (CVSListBox *)GetDlgItem(IDC_PATHLIST);
	if (!ppl)
		return;

	for (int i = 0, maxi = ppl->GetCount(); i < maxi; i++)
	{
		CString p = ppl->GetItemText(i);
		paths += (const TCHAR *)p;
		if (i != (maxi - 1))
			paths += _T(';');
	}
}


void CResourcePathEditorDlg::OnSelchangeResType()
{
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_RESTYPECB);
	if (!pcb)
		return;

	CreatePathString(cfg_paths[m_LastSel]);
	CreateExtString(cfg_exts[m_LastSel]);

	m_LastSel = pcb->GetCurSel();

	UpdateExtList(cfg_exts[pcb->GetCurSel()].c_str());
	UpdatePathList(cfg_paths[pcb->GetCurSel()].c_str());
}


void CResourcePathEditorDlg::OnOK()
{
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_RESTYPECB);
	if (!pcb)
		return;

	m_LastSel = pcb->GetCurSel();

	CreatePathString(cfg_paths[m_LastSel]);
	CreateExtString(cfg_exts[m_LastSel]);

	for (size_t i = 0, maxi = typenames.size(); i < maxi; i++)
	{
		theApp.m_Config->SetString(cfgname_exts[i].c_str(), cfg_exts[i].c_str());
		theApp.m_Config->SetString(cfgname_paths[i].c_str(), cfg_paths[i].c_str());
	}

	theApp.m_Config->SetInt(_T("gui.resource_path_editor.selection"), m_LastSel);

	CDialog::OnOK();
}


void CResourcePathEditorDlg::OnCancel()
{
	CComboBox *pcb = (CComboBox *)GetDlgItem(IDC_RESTYPECB);
	if (!pcb)
		return;

	m_LastSel = pcb->GetCurSel();

	theApp.m_Config->SetInt(_T("gui.resource_path_editor.selection"), m_LastSel);

	CDialog::OnCancel();
}
