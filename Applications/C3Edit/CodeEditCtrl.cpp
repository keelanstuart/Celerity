// CodeEditCtrl.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "CodeEditCtrl.h"
#include "Resource.h"
#include "C3EditDoc.h"
#include "C3EditView.h"
#include "C3EditFrame.h"



// CCodeEditCtrl

IMPLEMENT_DYNAMIC(CCodeEditCtrl, CRichEditCtrl)

CCodeEditCtrl::CCodeEditCtrl()
{
    m_pRichEdOle = nullptr;
    m_pTextDoc = nullptr;
	m_pTextSvc = nullptr;
	m_Accel = NULL;
}

CCodeEditCtrl::~CCodeEditCtrl()
{
    if (m_pTextDoc)
        m_pTextDoc->Release();

	if (m_pTextSvc)
		m_pTextSvc->Release();

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
	ON_NOTIFY_REFLECT(EN_DROPFILES, &CCodeEditCtrl::OnEnDropfiles)
	ON_NOTIFY_REFLECT(EN_LINK, &CCodeEditCtrl::OnEnLink)
	ON_CONTROL_REFLECT(EN_VSCROLL, &CCodeEditCtrl::OnEnVscroll)
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_CUT, &CCodeEditCtrl::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CCodeEditCtrl::OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_COPY, &CCodeEditCtrl::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CCodeEditCtrl::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CCodeEditCtrl::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CCodeEditCtrl::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CCodeEditCtrl::OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CCodeEditCtrl::OnEditRedo)
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

#ifdef GetLogFont
#undef GetLogFont
#endif

BOOL CCodeEditCtrl::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	dwStyle |= WS_VSCROLL | WS_HSCROLL;
	BOOL ret = CRichEditCtrl::Create(dwStyle, rect, pParentWnd, nID);

	if (ret)
	{
		SetEventMask(ENM_CHANGE | ENM_MOUSEEVENTS);

		m_pRichEdOle = GetIRichEditOle();

		if (m_pRichEdOle)
		{
			if (HMODULE hmodRichEdit = LoadLibrary(L"Msftedit.dll"))
			{
				if (IID *pIID_ITD = (IID *)GetProcAddress(hmodRichEdit, "IID_ITextDocument"))
				{
					m_pRichEdOle->QueryInterface(*pIID_ITD, (LPVOID *)&m_pTextDoc);
				}

				// Turns off the beep sound when you hit up at the top or down at the bottom. Irritating.
				// Wizardry uncovered here: https://stackoverflow.com/questions/55884687/how-to-eliminate-the-messagebeep-from-the-richedit-control
				if (IID *pIID_ITS = (IID *)GetProcAddress(hmodRichEdit, "IID_ITextServices"))
				{
					if (SUCCEEDED(m_pRichEdOle->QueryInterface(*pIID_ITS, (LPVOID *)&m_pTextSvc)))
						m_pTextSvc->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 0);
				}
			}
		}

		ModifyStyleEx(0, ES_EX_ZOOMABLE);
		SetOptions(ECOOP_SET, ECO_AUTOWORDSELECTION | ECO_AUTOHSCROLL | ECO_AUTOVSCROLL | ECO_NOHIDESEL | ECO_SAVESEL | ECO_WANTRETURN);

		LOGFONT lf;
		afxGlobalData.fontRegular.GetLogFont(&lf);
		_tcscpy_s(lf.lfFaceName, _T("Consolas"));
		m_CodeFont.CreateFontIndirect(&lf);

		SetFont(&m_CodeFont);

		SetBackgroundColor(FALSE, RGB(0, 0, 0));

		CHARFORMAT2 cf = { 0 };
		cf.cbSize = sizeof(CHARFORMAT2);
		GetDefaultCharFormat(cf);
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = 0;
		cf.crTextColor = RGB(255, 255, 255);
		SetDefaultCharFormat(cf);

		m_Accel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACC_CODE_EDIT));
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


BOOL CCodeEditCtrl::PreTranslateMessage(MSG *pMsg)
{
	if (GetFocus() == this)
	{
		if (m_Accel)
		{
			if (::TranslateAccelerator(GetSafeHwnd(), m_Accel, pMsg))
				return TRUE;
		}

		switch (pMsg->message)
		{
			case WM_KEYDOWN:
			{
				switch (pMsg->wParam)
				{
					case VK_ESCAPE:
					{
						C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
						C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
						POSITION vp = pdoc->GetFirstViewPosition();
						C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

						pv->SetFocus();
						return TRUE;
						break;
					}

					case VK_TAB:
					{
						ReplaceSel(_T("\t"));
						return TRUE;
						break;
					}

					case VK_DELETE:
					{
						long ss, se;
						GetSel(ss, se);
						if (ss == se)
							SetSel(ss, ss + 1);
						ReplaceSel(_T(""));
						return TRUE;
						break;
					}
				}
				break;
			}

			case WM_CUT:
				Cut();
				break;

			case WM_COPY:
				Copy();
				break;

			case WM_PASTE:
				Paste();
				break;

			case WM_CLEAR:
				Clear();
				break;
		}
	}

	return CRichEditCtrl::PreTranslateMessage(pMsg);
}


void CCodeEditCtrl::OnEnDropfiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	ENDROPFILES *pEnDropFiles = reinterpret_cast<ENDROPFILES *>(pNMHDR);
	// TODO:  The control will not send this notification unless you override the
	// CRichEditCtrl::OnInitDialog() function to send the EM_SETEVENTMASK message
	// to the control with the ENM_DROPFILES flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here

	*pResult = 0;
}


void CCodeEditCtrl::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
	ENLINK *pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CCodeEditCtrl::OnEnVscroll()
{
	// TODO: Add your control notification handler code here
}


BOOL CCodeEditCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CRichEditCtrl::OnMouseWheel(nFlags, zDelta, pt);
}


void CCodeEditCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_SCRIPTING, point.x, point.y, this, TRUE);
#endif
}


void CCodeEditCtrl::OnEditCut()
{
	Cut();
}


void CCodeEditCtrl::OnUpdateEditCut(CCmdUI *pCmdUI)
{
}


void CCodeEditCtrl::OnEditCopy()
{
	Copy();
}


void CCodeEditCtrl::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
}


void CCodeEditCtrl::OnEditPaste()
{
	Paste();
}


void CCodeEditCtrl::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
}


void CCodeEditCtrl::OnEditUndo()
{
	Undo();
}


void CCodeEditCtrl::OnEditRedo()
{
	Redo();
}
