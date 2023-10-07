#pragma once

#include "ScrollPanel.h"


// CGraphView view

class CGraphView : public CScrollPanel
{
	DECLARE_DYNCREATE(CGraphView)

public:
	CGraphView();           // protected constructor used by dynamic creation
	virtual ~CGraphView();


protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextMenu(CWnd * /*pWnd*/, CPoint /*point*/);
	afx_msg void OnGraphAddNode();
	afx_msg void OnUpdateGraphAddNode(CCmdUI *pCmdUI);
	afx_msg void OnGraphDeleteNode();
	afx_msg void OnUpdateGraphDeleteNode(CCmdUI *pCmdUI);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnPaint();
};


