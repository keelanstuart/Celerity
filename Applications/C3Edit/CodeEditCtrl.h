#pragma once

#include <imm.h>
#include <textserv.h>
#include <RichOle.h>
#include <Richedit.h>
#include <TOM.h>

// CCodeEditCtrl

class CCodeEditCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CCodeEditCtrl)

public:
	CCodeEditCtrl();
	virtual ~CCodeEditCtrl();

	void EnableUndo(bool en);

protected:

	IRichEditOle *m_pRichEdOle;
	ITextDocument *m_pTextDoc;
	ITextServices *m_pTextSvc;

	CFont m_CodeFont;
	HACCEL m_Accel;

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
	virtual BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	afx_msg void OnEnChange();
	afx_msg void OnEnUpdate();

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnEnDropfiles(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnVscroll();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnContextMenu(CWnd * /*pWnd*/, CPoint /*point*/);
    afx_msg void OnEditCut();
    afx_msg void OnUpdateEditCut(CCmdUI *pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
};


