// RepathDlg.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "afxdialogex.h"
#include "RepathDlg.h"
#include "C3EditFrame.h"
#include "C3EditDoc.h"
#include "C3EditView.h"

#include <C3Utility.h>


// CRepathDlg dialog

IMPLEMENT_DYNAMIC(CRepathDlg, CDialog)

CRepathDlg::CRepathDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_REPATH, pParent)
{

}

CRepathDlg::~CRepathDlg()
{
}

void CRepathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRepathDlg, CDialog)
END_MESSAGE_MAP()


// CRepathDlg message handlers

void RepathProperty(props::IPropertySet *propset, props::FOURCHARCODE propid, const std::vector<tstring> &paths)
{
	if (!propset)
		return;

	props::IProperty *pp = propset->GetPropertyById(propid);
	if (!pp)
		return;

	if (paths.empty())
		return;

	TCHAR modf[512];
	if (c3::util::FindShortestRelativePath(paths, pp->AsString(), modf, 512))
		pp->SetString(modf);
}

void CRepathDlg::OnOK()
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());

	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	if (!pdoc)
		return;

	C3EditView *pv = dynamic_cast<C3EditView *>(pfrm->GetActiveView());
	if (!pv)
		return;

	// Set up the path vectors that we'll use below
	std::vector<tstring> models_paths, textures_paths, scripts_paths, rendermethods_paths;
	theApp.ExtractPaths(theApp.m_Config->GetString(_T("resources.models.paths"), _T("")), models_paths);
	theApp.ExtractPaths(theApp.m_Config->GetString(_T("resources.textures.paths"), _T("")), textures_paths);
	theApp.ExtractPaths(theApp.m_Config->GetString(_T("resources.scripts.paths"), _T("")), scripts_paths);
	theApp.ExtractPaths(theApp.m_Config->GetString(_T("resources.rendermethods.paths"), _T("")), rendermethods_paths);

	// Here we choose which properties we want to repath
	std::function<void(props::IPropertySet *)> propsremap = [&](props::IPropertySet *pps)
	{
		RepathProperty(pps, 'MODF', models_paths);
		RepathProperty(pps, 'SCRF', scripts_paths);
		RepathProperty(pps, 'PEtx', textures_paths);
		RepathProperty(pps, 'C3RM', rendermethods_paths);
		RepathProperty(pps, 'PErm', rendermethods_paths);
	};

	// In this variant, we get the properties from an Object
	c3::util::ObjectActionFunc objremap = [&](c3::Object *pobj)
	{
		props::IPropertySet *pps = pobj->GetProperties();
		propsremap(pps);
	};

	// In this variant, we get the properties from a Prototype
	std::function<void(c3::Prototype *)> protoremap = [&](c3::Prototype *pproto)
	{
		props::IPropertySet *pps = pproto->GetProperties();
		propsremap(pps);
	};

	int mode = ((CComboBox *)GetDlgItem(IDC_REPATH_MODE))->GetCurSel();
	switch (mode)
	{
		// call the Object variant for all objects
		case RPM_SELECTEDOBJS:
			pdoc->DoForAllSelected(objremap);
			break;

		// call the Object variant for selected objects
		case RPM_ALLOBJS:
			c3::util::RecursiveObjectAction(pdoc->m_RootObj, objremap);
			break;

		case RPM_SELECTEDPROTO:
			pfrm->m_wndProtoView.ActOnSelection(protoremap, false);
			break;

		case RPM_GROUPEDPROTOS:
			pfrm->m_wndProtoView.ActOnSelection(protoremap, true);
			break;

		case RPM_ALLPROTOS:
		{
			for (size_t i = 0; i < theApp.m_C3->GetFactory()->GetNumPrototypes(); i++)
				protoremap(theApp.m_C3->GetFactory()->GetPrototype(i));
			break;
		}
	}

	theApp.m_Config->SetInt(_T("tools.remap.mode"), mode);

	CDialog::OnOK();
}


BOOL CRepathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CComboBox *)GetDlgItem(IDC_REPATH_MODE))->SetCurSel((int)(theApp.m_Config->GetInt(_T("tools.remap.mode"), 0)));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
