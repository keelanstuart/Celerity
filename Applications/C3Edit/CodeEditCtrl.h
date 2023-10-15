#pragma once

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

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
	virtual BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	afx_msg void OnEnChange();
	afx_msg void OnEnUpdate();

};


