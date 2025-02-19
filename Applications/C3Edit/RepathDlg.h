#pragma once
#include "afxdialogex.h"


// CRepathDlg dialog

class CRepathDlg : public CDialog
{
	DECLARE_DYNAMIC(CRepathDlg)

public:
	CRepathDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRepathDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REPATH };
#endif

	using RepathMode = enum
	{
		RPM_SELECTEDOBJS = 0,
		RPM_ALLOBJS,
		RPM_SELECTEDPROTO,
		RPM_GROUPEDPROTOS,
		RPM_ALLPROTOS
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};
