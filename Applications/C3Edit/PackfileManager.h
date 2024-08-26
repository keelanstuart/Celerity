#pragma once
#include "afxdialogex.h"

class ZipArchiveVSListBox : public CVSListBox
{
public:
	virtual void OnBrowse();  // Override to show custom file dialog
	virtual void SetItemText(int iIndex, const CString& strText);
	virtual int AddItem(const CString& strText, DWORD_PTR dwData = 0, int iIndex = -1);

	int m_itemIdx;
};

// CPackfileManager dialog

class CPackfileManager : public CDialog
{
	DECLARE_DYNAMIC(CPackfileManager)

public:
	CPackfileManager(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPackfileManager();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PACKFILEMANAGER };
#endif

	ZipArchiveVSListBox m_ArcListBox;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
