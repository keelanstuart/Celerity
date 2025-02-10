#pragma once
#include "afxdialogex.h"
#include "C3EditDoc.h"


// CCreatePropertyDlg dialog

class CCreatePropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreatePropertyDlg)

public:
	CCreatePropertyDlg(props::IPropertySet *pprops, CWnd* pParent = nullptr);   // standard constructor
	CCreatePropertyDlg(C3EditDoc *pdoc, CWnd* pParent = nullptr);   // document selections
	virtual ~CCreatePropertyDlg();

// Dialog Data
	enum { IDD = IDD_CREATEPROPERTY };

	CString m_PropName;
	props::FOURCHARCODE m_PropFCC;
	props::IProperty::PROPERTY_TYPE m_PropType;
	props::IProperty::PROPERTY_ASPECT m_PropAspect;

protected:
	props::IPropertySet *m_pProps;
	C3EditDoc *m_pDoc;
	CEdit m_wndEditName;
	CEdit m_wndEditFCC;
	CComboBox m_wndComboType;
	CComboBox m_wndComboAspect;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
