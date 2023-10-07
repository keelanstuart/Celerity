#pragma once


// CGraphNode dialog

class CGraphNode : public CDialog
{
	DECLARE_DYNAMIC(CGraphNode)

public:

	static CGraphNode *DoModeless(CWnd *pParent);

	CGraphNode(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGraphNode();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRAPHNODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
    afx_msg void OnMove(int x, int y);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
