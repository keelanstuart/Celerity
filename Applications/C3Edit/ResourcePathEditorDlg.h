#pragma once


// CResourcePathEditorDlg dialog

class CResourcePathEditorDlg : public CDialog
{
	DECLARE_DYNAMIC(CResourcePathEditorDlg)

public:
	enum { IDD = IDD_EXTENSIONMAPPER };

	std::vector<tstring> cfgname_exts;
	std::vector<tstring> cfgname_paths;
	std::vector<tstring> cfg_exts;
	std::vector<tstring> cfg_paths;

	int m_LastSel;

	CResourcePathEditorDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CResourcePathEditorDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void UpdateExtList(const TCHAR *exts);
	void CreateExtString(tstring &exts);

	void UpdatePathList(const TCHAR *paths);
	void CreatePathString(tstring &paths);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeResType();
	virtual void OnOK();
	virtual void OnCancel();
};
