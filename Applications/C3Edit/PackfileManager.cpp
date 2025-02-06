// PackfileManager.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "afxdialogex.h"
#include "PackfileManager.h"
#include "wtfvslistbox.h"

#include <afxcoll.h>

void ZipArchiveVSListBox::OnBrowse()
{
	CString filter = _T("Supported Archive Files (*.c3z; *.zip)|*.c3z;*.zip|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("c3z"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter);
	if (dlg.DoModal() == IDOK)
	{
		CString filepath = dlg.GetPathName();
		const TCHAR *fn = PathFindFileName(filepath);
		TCHAR _filepath[MAX_PATH * 2];
		theApp.m_C3->GetFileMapper()->FindFile(fn, _filepath, MAX_PATH * 2);

		// if we found the file but it's in a different location, then it's not "the same", so use the full path
		if (_tcsicmp(filepath, _filepath))
			fn = filepath;

		SetItemText(m_itemIdx, fn);  // Set the selected file path in the list box
	}
}

void ZipArchiveVSListBox::SetItemText(int iIndex, const CString& strText)
{
	if (strText == _T(""))
		m_itemIdx = iIndex;

	__super::SetItemText(iIndex, strText);
}

int ZipArchiveVSListBox::AddItem(const CString& strText, DWORD_PTR dwData, int iIndex)
{
	int ret = __super::AddItem(strText, dwData, iIndex);
	if (strText == _T(""))
		m_itemIdx = ret;

	return ret;
}


// CPackfileManager dialog

IMPLEMENT_DYNAMIC(CPackfileManager, CDialog)

CPackfileManager::CPackfileManager(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PACKFILEMANAGER, pParent)
{

}

CPackfileManager::~CPackfileManager()
{
}

void CPackfileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PACKFILELIST, m_ArcListBox);
}


BEGIN_MESSAGE_MAP(CPackfileManager, CDialog)
END_MESSAGE_MAP()


// CPackfileManager message handlers


BOOL CPackfileManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();
	for (size_t i = 0, maxi = presman->GetNumRegisteredZipArchives(); i < maxi; i++)
		m_ArcListBox.AddItem(presman->GetRegisteredZipArchivePath(i));

	return TRUE;  // return TRUE unless you set the focus to a control, EXCEPTION: OCX Property Pages should return FALSE
}


void CPackfileManager::OnOK()
{
	// this looks more complicated than it really is...
	// basically, see which archives are loaded already and make them to be unloaded...
	// then see which archives should be loaded according to our list here...
	// either mark existing ones as true or insert a new one...
	// finally, go through should_load map and load things marked true, unload things marked false

	c3::ResourceManager *presman = theApp.m_C3->GetResourceManager();

	TCHAR kn[256];

	std::map<tstring, bool> should_load;
	for (size_t i = 0, maxi = presman->GetNumRegisteredZipArchives(); i < maxi; i++)
	{
		tstring t = presman->GetRegisteredZipArchivePath(i);
		std::transform(t.begin(), t.end(), t.begin(), std::tolower);

		should_load.insert(std::pair<tstring, bool>(t, false));

		_stprintf_s(kn, _T("resources.packfiles.archives.packfile#%zu"), i);
		theApp.m_Config->RemoveKey(kn);
	}

	for (size_t i = 0, maxi = m_ArcListBox.GetCount(); i < maxi; i++)
	{
		CString t = m_ArcListBox.GetItemText((int)i);
		t.MakeLower();
		auto it = should_load.find((LPCTSTR)t);
		if (it != should_load.end())
			it->second = true;
		else
			should_load.insert(std::pair<tstring, bool>((LPCTSTR)t, true));
	}

	size_t i = 0;
	for (auto t : should_load)
	{
		if (t.second)
		{
			theApp.m_C3->GetResourceManager()->RegisterZipArchive(t.first.c_str());

			_stprintf_s(kn, _T("resources.packfiles.archives.packfile#%zu"), i++);
			theApp.m_Config->SetString(kn, t.first.c_str());
		}
		else
		{
			theApp.m_C3->GetResourceManager()->UnregisterZipArchive(t.first.c_str());
		}
	}

	CDialog::OnOK();
}
