#pragma once

#include "PropertyGrid.h"
#include <C3Prototype.h>

// CEditPrototypeDlg dialog

class CEditPrototypeDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditPrototypeDlg)

public:
	CEditPrototypeDlg(c3::Prototype *pproto, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditPrototypeDlg();

protected:
	CEdit m_wndNameEdit;
	CPropertyGrid m_wndPropList;
	CCheckListBox m_wndCompList;
	CCheckListBox m_wndFlagList;

	c3::Prototype *m_pProto;
	props::IPropertySet *m_pTempProps;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDITPROTOTYPE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
