// CodeEditCtrl.cpp : implementation file
//

#include "pch.h"
#include "CodeEditCtrl.h"


#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID CDECL name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,0x00,0x47,0xBE,0x5D);

// CCodeEditCtrl

IMPLEMENT_DYNAMIC(CCodeEditCtrl, CRichEditCtrl)

CCodeEditCtrl::CCodeEditCtrl()
{
    m_pRichEdOle = nullptr;
    m_pTextDoc = nullptr;
}

CCodeEditCtrl::~CCodeEditCtrl()
{
    if (m_pTextDoc)
        m_pTextDoc->Release();

    if (m_pRichEdOle)
        m_pRichEdOle->Release();

	if (m_CodeFont.GetSafeHandle() != NULL)
	{
		::DeleteObject(m_CodeFont.Detach());
	}
}


BEGIN_MESSAGE_MAP(CCodeEditCtrl, CRichEditCtrl)
	ON_CONTROL_REFLECT(EN_CHANGE, &CCodeEditCtrl::OnEnChange)
	ON_CONTROL_REFLECT(EN_UPDATE, &CCodeEditCtrl::OnEnUpdate)
END_MESSAGE_MAP()



// CCodeEditCtrl message handlers


void CCodeEditCtrl::EnableUndo(bool en)
{
    if (m_pTextDoc)
        m_pTextDoc->Undo(en ? tomResume : tomSuspend, 0);
}


BOOL CCodeEditCtrl::PreCreateWindow(CREATESTRUCT &cs)
{
	return CRichEditCtrl::PreCreateWindow(cs);
}


BOOL CCodeEditCtrl::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	BOOL ret = CRichEditCtrl::Create(dwStyle, rect, pParentWnd, nID);

	if (ret)
	{
		SetEventMask(ENM_CHANGE);

		m_pRichEdOle = GetIRichEditOle();

		if (m_pRichEdOle)
			m_pRichEdOle->QueryInterface(IID_ITextDocument, (LPVOID *)&m_pTextDoc);

		LOGFONT lf;
		afxGlobalData.fontRegular.GetLogFont(&lf);
		_tcscpy_s(lf.lfFaceName, _T("Consolas"));
		m_CodeFont.CreateFontIndirect(&lf);

		SetFont(&m_CodeFont);
	}

    return ret;
}


void CCodeEditCtrl::OnEnChange()
{
}


void CCodeEditCtrl::OnEnUpdate()
{
	//EnableUndo(false);

	//EnableUndo(true);
}
