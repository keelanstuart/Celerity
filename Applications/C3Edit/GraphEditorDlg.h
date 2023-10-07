#pragma once

#include "GraphView.h"

// CGraphEditorDlg dialog

class CGraphEditorDlg : public CDialog
{
	DECLARE_DYNAMIC(CGraphEditorDlg)

public:

	static CGraphEditorDlg *DoModeless(CWnd *pParent);

	CGraphEditorDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGraphEditorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRAPHEDITOR };
#endif

	CGraphView m_GraphView;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
