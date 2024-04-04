#pragma once


// C3ResourceSpyListCtrl

class C3ResourceSpyListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(C3ResourceSpyListCtrl)

public:
	C3ResourceSpyListCtrl();
	virtual ~C3ResourceSpyListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT pdi);
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);

	afx_msg void OnUpdateResourceReload(CCmdUI *pCmdUI);
	afx_msg void OnResourceReload();

};


