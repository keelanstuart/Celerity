// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "pch.h"
#include <afxcontrolbarutil.h>
#include "wtfpropertygridctrl.h"

#include "wtfcolorbar.h"
//#include "wtfcolorslidersbar.h"
#include <afxwinappex.h>
#include <afxshellmanager.h>
#include <afxmenuimages.h>
#include <afxvisualmanager.h>
#include <afxtoolbarcomboboxbutton.h>
#include <afxpopupmenu.h>
#include <afxdrawmanager.h>
#include <afxmaskededit.h>
#include <afxspinbuttonctrl.h>
#include <afxtagmanager.h>
#include <afxctrlcontainer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_ID_HEADER 1
#define AFX_ID_SCROLL_VERT 2
#define AFX_STRETCH_DELTA 2

#define AFX_TEXT_MARGIN 4

#define AFX_UM_UPDATESPIN (WM_USER + 101)

#define visualManager CMFCVisualManager::GetInstance()

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridProperty

IMPLEMENT_DYNAMIC(CWTFPropertyGridProperty, CObject)

#define AFX_PROP_HAS_LIST 0x0001
#define AFX_PROP_HAS_BUTTON 0x0002
#define AFX_PROP_HAS_SPIN 0x0004

#define AFX_FORMAT_CHAR   _T("%Tc")
#define AFX_FORMAT_SHORT  _T("%d")
#define AFX_FORMAT_LONG   _T("%ld")
#define AFX_FORMAT_USHORT _T("%u")
#define AFX_FORMAT_ULONG  _T("%u")
#define AFX_FORMAT_FLOAT  _T("%f")
#define AFX_FORMAT_DOUBLE _T("%lf")

CWTFPropertyGridProperty::CWTFPropertyGridProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr, DWORD_PTR dwData, LPCTSTR lpszEditMask, LPCTSTR lpszEditTemplate, LPCTSTR lpszValidChars) :
	m_strName(strName),
	m_varValue(varValue),
	m_varValueOrig(varValue),
	m_strDescr(lpszDescr == NULL ? _T("") : lpszDescr),
	m_strEditMask(lpszEditMask == NULL ? _T("") : lpszEditMask),
	m_strEditTempl(lpszEditTemplate == NULL ? _T("") : lpszEditTemplate),
	m_strValidChars(lpszValidChars == NULL ? _T("") : lpszValidChars),
	m_dwData(dwData)
{
	m_bGroup = FALSE;
	m_bIsValueList = FALSE;

	Init();
	SetFlags();

	if (m_varValue.vt == VT_BOOL)
	{
		m_bAllowEdit = FALSE;
	}
}

CWTFPropertyGridProperty::CWTFPropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList) :
	m_strName(strGroupName),
	m_dwData(dwData),
	m_bIsValueList(bIsValueList)
{
	m_bGroup = TRUE;

	Init();
	SetFlags();
}

void CWTFPropertyGridProperty::SetFlags()
{
	m_dwFlags = 0;

	switch (m_varValue.vt)
	{
		case VT_BSTR:
		case VT_R4:
		case VT_R8:
		case VT_UI1:
		case VT_I2:
		case VT_I4:
		case VT_I8:
		case VT_INT:
		case VT_UINT:
		case VT_UI2:
		case VT_UI4:
			break;

		case VT_DATE:
			break;

		case VT_BOOL:
			m_dwFlags = AFX_PROP_HAS_LIST;
			break;

		default:
			break;
	}
}

void CWTFPropertyGridProperty::Init()
{
	m_pWndList = NULL;
	m_bExpanded = !m_bIsValueList;
	m_bEnabled = TRUE;
	m_pParent = NULL;
	m_pWndInPlace = NULL;
	m_pWndCombo = NULL;
	m_pWndSpin = NULL;
	m_bInPlaceEdit = FALSE;
	m_bButtonIsFocused = FALSE;
	m_bButtonIsDown = FALSE;
	m_bAllowEdit = TRUE;
	m_bNameIsTruncated = FALSE;
	m_bValueIsTruncated = FALSE;

	m_Rect.SetRectEmpty();
	m_rectButton.SetRectEmpty();

	m_nMinValue = 0;
	m_nMaxValue = 0;

	m_bIsModified = FALSE;
	m_bIsVisible = TRUE;
}

CWTFPropertyGridProperty::~CWTFPropertyGridProperty()
{
	while (!m_lstSubItems.IsEmpty())
	{
		delete m_lstSubItems.RemoveHead();
	}

	OnDestroyWindow();
}

void CWTFPropertyGridProperty::OnDestroyWindow()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow();
		delete m_pWndCombo;
		m_pWndCombo = NULL;
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->DestroyWindow();
		delete m_pWndInPlace;
		m_pWndInPlace = NULL;
	}

	if (m_pWndSpin != NULL)
	{
		m_pWndSpin->DestroyWindow();
		delete m_pWndSpin;
		m_pWndSpin = NULL;
	}

	if (m_varValue.vt == VT_BOOL)
	{
		m_lstOptions.RemoveAll();
	}
}

BOOL CWTFPropertyGridProperty::HasButton() const
{
	return(m_dwFlags & AFX_PROP_HAS_LIST) ||
		(m_dwFlags & AFX_PROP_HAS_BUTTON);
}

BOOL CWTFPropertyGridProperty::AddSubItem(CWTFPropertyGridProperty* pProp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	if (!IsGroup())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (pProp->m_pWndList != NULL)
	{
		for (POSITION pos = pProp->m_pWndList->m_lstProps.GetHeadPosition(); pos != NULL;)
		{
			CWTFPropertyGridProperty* pListProp = pProp->m_pWndList->m_lstProps.GetNext(pos);
			ASSERT_VALID(pListProp);

			if (pListProp == pProp || pListProp->IsSubItem(pProp))
			{
				// Can't ad the same property twice
				ASSERT(FALSE);
				return FALSE;
			}
		}
	}

	pProp->m_pParent = this;

	m_lstSubItems.AddTail(pProp);
	pProp->m_pWndList = m_pWndList;

	return TRUE;
}

BOOL CWTFPropertyGridProperty::RemoveSubItem(CWTFPropertyGridProperty*& pProp, BOOL bDelete/* = TRUE*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;

		CWTFPropertyGridProperty* pListProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pListProp);

		if (pListProp == pProp)
		{
			m_lstSubItems.RemoveAt(posSaved);

			if (m_pWndList != NULL && m_pWndList->m_pSel == pProp)
			{
				m_pWndList->m_pSel = NULL;
			}

			if (bDelete)
			{
				delete pProp;
				pProp = NULL;
			}

			return TRUE;
		}

		if (pListProp->RemoveSubItem(pProp, bDelete))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CWTFPropertyGridProperty::AddOption(LPCTSTR lpszOption, BOOL bInsertUnique/* = TRUE*/)
{
	ASSERT_VALID(this);
	ENSURE(lpszOption != NULL);

	if (bInsertUnique)
	{
		if (m_lstOptions.Find(lpszOption) != NULL)
		{
			return FALSE;
		}
	}

	m_lstOptions.AddTail(lpszOption);
	m_dwFlags = AFX_PROP_HAS_LIST;

	return TRUE;
}

BOOL CWTFPropertyGridCtrl::DeleteProperty(CWTFPropertyGridProperty*& pProp, BOOL bRedraw, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	BOOL bFound = FALSE;

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;

		CWTFPropertyGridProperty* pListProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pListProp);

		if (pListProp == pProp) // Top level property
		{
			bFound = TRUE;

			m_lstProps.RemoveAt(posSaved);
			break;
		}

		if (pListProp->RemoveSubItem(pProp, FALSE))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		return FALSE;
	}

	if (m_pSel == pProp)
	{
		m_pSel = NULL;
	}

	if (m_pSel != NULL)
	{
		for (CWTFPropertyGridProperty* pParent = m_pSel; pParent != NULL; pParent = pParent->GetParent())
		{
			if (pParent == pProp)
			{
				m_pSel = NULL;
				break;
			}
		}
	}

	delete pProp;
	pProp = NULL;

	if (bAdjustLayout)
	{
		if (m_bAlphabeticMode)
		{
			ReposProperties();
		}

		AdjustLayout();
		return TRUE;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}

	return TRUE;
}

void CWTFPropertyGridProperty::RemoveAllOptions()
{
	ASSERT_VALID(this);

	m_lstOptions.RemoveAll();
	m_dwFlags = 0;
}

int CWTFPropertyGridProperty::GetOptionCount() const
{
	ASSERT_VALID(this);
	return(int) m_lstOptions.GetCount();
}

LPCTSTR CWTFPropertyGridProperty::GetOption(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_lstOptions.GetCount())
	{
		ASSERT(FALSE);
		return NULL;
	}

	POSITION pos = m_lstOptions.FindIndex(nIndex);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_lstOptions.GetAt(pos);
}

int CWTFPropertyGridProperty::GetExpandedSubItems(BOOL bIncludeHidden) const
{
	ASSERT_VALID(this);

	if (!m_bExpanded)
	{
		return 0;
	}

	int nCount = 0;

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		if (bIncludeHidden || pProp->IsVisible())
		{
			nCount += pProp->GetExpandedSubItems(bIncludeHidden) + 1;
		}
	}

	return nCount;
}

CWTFPropertyGridProperty* CWTFPropertyGridProperty::HitTest(CPoint point, ClickArea* pnArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_Rect.PtInRect(point))
	{
		if (pnArea != NULL)
		{
			if (IsGroup() && point.x < m_Rect.left + m_Rect.Height())
			{
				*pnArea = ClickExpandBox;
			}
			else if (HasValueField() && point.x > m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth)
			{
				*pnArea = ClickValue;
			}
			else
			{
				*pnArea = ClickName;
			}
		}

		return this;
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		CWTFPropertyGridProperty* pHit = pProp->HitTest(point, pnArea);
		if (pHit != NULL)
		{
			return pHit;
		}
	}

	return NULL;
}

void CWTFPropertyGridProperty::Expand(BOOL bExpand)
{
	ASSERT_VALID(this);
	ASSERT(IsGroup());

	if (m_bExpanded == bExpand || m_lstSubItems.IsEmpty())
	{
		return;
	}

	m_bExpanded = bExpand;

	if (m_pWndList != NULL && m_pWndList->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->AdjustLayout();

		CRect rectRedraw = m_pWndList->m_rectList;
		rectRedraw.top = m_Rect.top;

		m_pWndList->RedrawWindow(rectRedraw);
	}
}

void CWTFPropertyGridProperty::ExpandDeep(BOOL bExpand)
{
	ASSERT_VALID(this);

	m_bExpanded = bExpand;

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->ExpandDeep(bExpand);
	}
}

void CWTFPropertyGridProperty::ResetOriginalValue()
{
	ASSERT_VALID(this);

	m_bIsModified = FALSE;

	SetValue(m_varValueOrig);

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->ResetOriginalValue();
	}
}

void CWTFPropertyGridProperty::Redraw()
{
	ASSERT_VALID(this);

	if (m_pWndList != NULL)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->InvalidateRect(m_Rect);

		if (m_pParent != NULL && m_pParent->m_bIsValueList)
		{
			m_pWndList->InvalidateRect(m_pParent->m_Rect);
		}

		if (m_bIsValueList)
		{
			for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
			{
				CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
				ASSERT_VALID(pProp);

				m_pWndList->InvalidateRect(pProp->m_Rect);
			}
		}

		m_pWndList->UpdateWindow();
	}
}

void CWTFPropertyGridProperty::EnableSpinControl(BOOL bEnable, int nMin, int nMax)
{
	ASSERT_VALID(this);

	switch (m_varValue.vt)
	{
	case VT_INT:
	case VT_UINT:
	case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_UI2:
	case VT_UI4:
	case VT_UI8:
		break;

	default:
		ASSERT(FALSE);
		return;
	}

	m_nMinValue = nMin;
	m_nMaxValue = nMax;

	if (bEnable)
	{
		m_dwFlags |= AFX_PROP_HAS_SPIN;
	}
	else
	{
		m_dwFlags &= ~AFX_PROP_HAS_SPIN;
	}
}

BOOL CWTFPropertyGridProperty::IsSelected() const
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	return m_pWndList->m_pSel == this;
}

void CWTFPropertyGridProperty::SetName(LPCTSTR lpszName, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (lpszName == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_strName = lpszName;

	if (bRedraw)
	{
		Redraw();
	}
}

void CWTFPropertyGridProperty::UpdateValue(const COleVariant& varValue)
{
	ASSERT_VALID(this);

	if (m_varValue == varValue)
		return;

	SetValue(varValue);
}

void CWTFPropertyGridProperty::SetValue(const COleVariant& varValue)
{
	ASSERT_VALID(this);

	BOOL bInPlaceEdit = m_bInPlaceEdit;
	if (bInPlaceEdit)
	{
		OnEndEdit();
	}

	m_varValue = varValue;
	Redraw();

	if (bInPlaceEdit)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->EditItem(this);
	}
}

void CWTFPropertyGridProperty::SetOriginalValue(const COleVariant& varValue)
{
	ASSERT_VALID(this);

	if (m_varValueOrig.vt != VT_EMPTY && m_varValueOrig.vt != varValue.vt)
	{
		ASSERT(FALSE);
		return;
	}

	m_varValueOrig = varValue;
}

BOOL CWTFPropertyGridProperty::IsParentExpanded() const
{
	ASSERT_VALID(this);

	for (CWTFPropertyGridProperty* pProp = m_pParent; pProp != NULL;)
	{
		ASSERT_VALID(pProp);

		if (!pProp->IsExpanded())
		{
			return FALSE;
		}

		pProp = pProp->m_pParent;
	}

	return TRUE;
}

int CWTFPropertyGridProperty::GetHierarchyLevel() const
{
	ASSERT_VALID(this);

	int nLevel = 0;
	for (CWTFPropertyGridProperty* pParent = m_pParent; pParent != NULL;
		pParent = pParent->m_pParent)
	{
		nLevel++;
	}

	return nLevel;
}

CWTFPropertyGridProperty* CWTFPropertyGridProperty::GetSubItem(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_lstSubItems.GetCount())
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_lstSubItems.GetAt(m_lstSubItems.FindIndex(nIndex));
}

void CWTFPropertyGridProperty::Enable(BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (m_bEnabled != bEnable)
	{
		m_bEnabled = bEnable;

		if (m_pWndList->GetSafeHwnd() != NULL)
		{
			ASSERT_VALID(m_pWndList);
			m_pWndList->InvalidateRect(m_Rect);
		}
	}
}

void CWTFPropertyGridProperty::SetOwnerList(CWTFPropertyGridCtrl* pWndList)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWndList);

	m_pWndList = pWndList;

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->SetOwnerList(m_pWndList);
	}
}

void CWTFPropertyGridProperty::Reposition(int& y)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	CRect rectOld = m_Rect;

	BOOL bShowProperty = m_bIsVisible &&(IsParentExpanded() || m_pWndList->m_bAlphabeticMode);

	if (m_pWndList->m_bAlphabeticMode && m_pParent != NULL && m_pParent->m_bIsValueList && !IsParentExpanded())
	{
		bShowProperty = FALSE;
	}

	if (bShowProperty)
	{
		int dx = m_pWndList->m_bAlphabeticMode ? m_pWndList->m_nRowHeight : GetHierarchyLevel() * m_pWndList->m_nRowHeight;

		if (m_pWndList->m_bAlphabeticMode && m_bIsValueList)
		{
			dx = 0;
		}

		m_Rect = CRect(m_pWndList->m_rectList.left + dx, y, m_pWndList->m_rectList.right, y + m_pWndList->m_nRowHeight);

		if (!m_rectButton.IsRectEmpty())
		{
			m_rectButton.top = m_Rect.top + 1;
			m_rectButton.bottom = m_Rect.bottom;
		}

		y += m_pWndList->m_nRowHeight;

		CRect rectName = m_Rect;
		rectName.right = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth;

		if (IsWindow(m_pWndList->m_ToolTip.GetSafeHwnd()))
		{
			m_pWndList->m_ToolTip.AddTool(m_pWndList, LPSTR_TEXTCALLBACK, rectName, m_pWndList->m_nTooltipsCount + 1);
			m_pWndList->m_nTooltipsCount ++;

			if (!IsGroup())
			{
				CRect rectValue = m_Rect;
				rectValue.left = rectName.right + 1;
				m_pWndList->m_ToolTip.AddTool(m_pWndList, LPSTR_TEXTCALLBACK, rectValue, m_pWndList->m_nTooltipsCount + 1);

				m_pWndList->m_nTooltipsCount ++;
			}
		}
	}
	else
	{
		m_Rect.SetRectEmpty();
		m_rectButton.SetRectEmpty();
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->Reposition(y);
	}

	OnPosSizeChanged(rectOld);
}

void CWTFPropertyGridProperty::AddTerminalProp(CList<CWTFPropertyGridProperty*, CWTFPropertyGridProperty*>& lstProps)
{
	ASSERT_VALID(this);

	if (!m_bGroup || m_bIsValueList)
	{
		// Insert sorted:
		BOOL bInserted = FALSE;
		for (POSITION pos = lstProps.GetHeadPosition(); !bInserted && pos != NULL;)
		{
			POSITION posSave = pos;

			CWTFPropertyGridProperty* pProp = lstProps.GetNext(pos);

			if (m_pWndList->CompareProps(pProp, this) > 0)
			{
				lstProps.InsertBefore(posSave, this);
				bInserted = TRUE;
			}
		}

		if (!bInserted)
		{
			lstProps.AddTail(this);
		}
		return;
	}

	m_Rect.SetRectEmpty();

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->AddTerminalProp(lstProps);
	}
}

BOOL CWTFPropertyGridProperty::IsSubItem(CWTFPropertyGridProperty* pSubProp) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSubProp);

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		if (pSubProp == pProp || pProp->IsSubItem(pSubProp))
		{
			return TRUE;
		}
	}

	return FALSE;
}

CWTFPropertyGridProperty* CWTFPropertyGridProperty::FindSubItemByData(DWORD_PTR dwData) const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		if (pProp->m_dwData == dwData)
		{
			return pProp;
		}

		pProp = pProp->FindSubItemByData(dwData);

		if (pProp != NULL)
		{
			return pProp;
		}
	}

	return NULL;
}

CString CWTFPropertyGridProperty::FormatProperty()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	COleVariant& var = m_varValue;

	CString strVal;

	if (m_bIsValueList)
	{
		for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
		{
			CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
			ASSERT_VALID(pProp);

			strVal += pProp->FormatProperty();

			if (pos != NULL)
			{
				strVal += m_pWndList->m_cListDelimeter;
				strVal += _T(' ');
			}
		}

		return strVal;
	}

	switch (var.vt)
	{
	case VT_BSTR:
		strVal = var.bstrVal;
		break;

	case VT_I2:
		strVal.Format(AFX_FORMAT_SHORT, (short)var.iVal);
		break;

	case VT_I4:
	case VT_I8:
	case VT_INT:
		strVal.Format(AFX_FORMAT_LONG, (long)var.lVal);
		break;

	case VT_UI1:
		if ((BYTE)var.bVal != 0)
		{
			strVal.Format(AFX_FORMAT_CHAR, (TCHAR)(BYTE)var.bVal);
		}
		break;

	case VT_UI2:
		strVal.Format(AFX_FORMAT_USHORT, var.uiVal);
		break;

	case VT_UI4:
	case VT_UI8:
	case VT_UINT:
		strVal.Format(AFX_FORMAT_ULONG, var.ulVal);
		break;

	case VT_R4:
	{
		strVal.Format(_T("%0.4f") /* AFX_FORMAT_FLOAT */, (float)var.fltVal);

		// trim insignificant 0's from the end of the string
		int d = strVal.Find(_T('.')) + 1;
		int q = strVal.GetLength() - 1;
		while ((q > d) && (strVal[q] == _T('0')))
		{
			strVal.Delete(q, 1);
			q--;
		}
		break;
	}

	case VT_R8:
	{
		strVal.Format(AFX_FORMAT_DOUBLE, (double)var.dblVal);

		// trim insignificant 0's from the end of the string
		int d = strVal.Find(_T('.')) + 1;
		int q = strVal.GetLength() - 1;
		while ((q > d) && (strVal[q] == _T('0')))
		{
			strVal.Delete(q, 1);
			q--;
		}
		break;
	}

	case VT_BOOL:
		strVal = var.boolVal == VARIANT_TRUE ? m_pWndList->m_strTrue : m_pWndList->m_strFalse;
		break;

	default:
		// Unsupported type
		strVal = _T("*** error ***");
	}

	return strVal;
}

void CWTFPropertyGridProperty::OnDrawName(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	COLORREF clrTextOld = (COLORREF)-1;

	if (IsSelected() &&(!m_pWndList->m_bVSDotNetLook || !IsGroup() ||  m_bIsValueList))
	{
		CRect rectFill = rect;
		rectFill.top++;

		if (!m_pWndList->m_bFocused)
		{
			clrTextOld = pDC->SetTextColor(GetGlobalData()->clrBtnText);

			pDC->FillRect(rectFill, m_pWndList->m_bControlBarColors ? &(GetGlobalData()->brBarFace) : &(GetGlobalData()->brBtnFace));
		}
		else
		{
			clrTextOld = pDC->SetTextColor(GetGlobalData()->clrTextHilite);
			pDC->FillRect(rectFill, &(GetGlobalData()->brHilite));
		}
	}

	if (m_pWndList->m_bVSDotNetLook && IsGroup() && !m_bIsValueList)
	{
		if (m_pWndList->m_clrGroupText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor(m_pWndList->m_clrGroupText);
		}
		else
		{
#if 1
			clrTextOld = pDC->SetTextColor(m_pWndList->m_clrGroupText);
#else
			clrTextOld = pDC->SetTextColor(visualManager->GetPropertyGridGroupTextColor(m_pWndList));
#endif
		}
	}

	if (m_pParent != NULL && m_pParent->m_bIsValueList)
	{
		rect.left += rect.Height();
	}

	rect.DeflateRect(AFX_TEXT_MARGIN, 0);

	int nTextHeight = pDC->DrawText(m_strName, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	m_bNameIsTruncated = pDC->GetTextExtent(m_strName).cx > rect.Width();

	if (IsSelected() && m_pWndList->m_bVSDotNetLook && IsGroup() && !m_bIsValueList)
	{
		CRect rectFocus = rect;
		rectFocus.top = rectFocus.CenterPoint().y - nTextHeight / 2;
		rectFocus.bottom = rectFocus.top + nTextHeight;
		rectFocus.right = std::min(rect.right, rectFocus.left + pDC->GetTextExtent(m_strName).cx);
		rectFocus.InflateRect(2, 0);

		COLORREF clrShadow = m_pWndList->m_bControlBarColors ? GetGlobalData()->clrBarShadow : GetGlobalData()->clrBtnShadow;

		pDC->Draw3dRect(rectFocus, clrShadow, clrShadow);
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}

void CWTFPropertyGridProperty::OnDrawValue(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	if ((IsGroup() && !m_bIsValueList) || !HasValueField())
	{
		return;
	}

	CFont* pOldFont = NULL;
	if (IsModified() && m_pWndList->m_bMarkModifiedProperties)
	{
		pOldFont = pDC->SelectObject(&m_pWndList->m_fontBold);
	}

	CString strVal = FormatProperty();

	rect.DeflateRect(AFX_TEXT_MARGIN, 0);

	pDC->DrawText(strVal, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	m_bValueIsTruncated = pDC->GetTextExtent(strVal).cx > rect.Width();

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}
}

void CWTFPropertyGridProperty::OnDrawButton(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	CMFCToolBarComboBoxButton button;

	pDC->FillRect(rect, m_pWndList->m_bControlBarColors ? &(GetGlobalData()->brBarFace) : &(GetGlobalData()->brBtnFace));

	if (m_dwFlags & AFX_PROP_HAS_LIST)
	{
		visualManager->OnDrawComboDropButton(pDC, rect, !m_bEnabled, m_bButtonIsDown, m_bButtonIsDown, &button);
		return;
	}

	CString str = _T("...");
	pDC->DrawText(str, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	CMFCVisualManager::AFX_BUTTON_STATE state = m_bButtonIsDown ? CMFCVisualManager::ButtonsIsPressed : CMFCVisualManager::ButtonsIsRegular;
	visualManager->OnDrawButtonBorder(pDC, &button, rect, state);

	if (m_bButtonIsFocused)
	{
		CRect rectFocus = rect;
		rectFocus.DeflateRect(2, 2);

		pDC->DrawFocusRect(rectFocus);
	}
}

void CWTFPropertyGridProperty::OnDrawExpandBox(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);
	ASSERT(IsGroup());

	CPoint ptCenter = rect.CenterPoint();

	int nMaxBoxSize = 9;
	if (GetGlobalData()->GetRibbonImageScale() != 1.)
	{
		nMaxBoxSize = (int)(.5 + nMaxBoxSize * GetGlobalData()->GetRibbonImageScale());
	}

	int nBoxSize = std::min(nMaxBoxSize, rect.Width ());

	rect = CRect(ptCenter, CSize(1, 1));
	rect.InflateRect(nBoxSize / 2, nBoxSize / 2);

	if (m_bIsValueList)
	{
		if (m_pWndList->m_brBackground.GetSafeHandle() != NULL)
		{
			pDC->FillRect(rect, &m_pWndList->m_brBackground);
		}
		else
		{
			pDC->FillRect(rect, &(GetGlobalData()->brWindow));
		}
	}

	COLORREF clrShadow = m_pWndList->m_bControlBarColors ? GetGlobalData()->clrBarShadow : GetGlobalData()->clrBtnShadow;
	COLORREF clrText = m_pWndList->m_bControlBarColors ? GetGlobalData()->clrBarText : GetGlobalData()->clrBtnText;

	visualManager->OnDrawExpandingBox(pDC, rect, m_bExpanded && !m_lstSubItems.IsEmpty(), m_pWndList->m_bVSDotNetLook ? clrText : clrShadow);
}

void CWTFPropertyGridProperty::OnDrawDescription(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	HFONT hOldFont = (HFONT) ::SelectObject(pDC->GetSafeHdc(), m_pWndList->m_fontBold.GetSafeHandle());
	int nHeight = pDC->DrawText(m_strName, rect, DT_SINGLELINE | DT_NOPREFIX);

	::SelectObject(pDC->GetSafeHdc(), hOldFont);

	rect.top += nHeight + 2;

	pDC->DrawText(m_strDescr, rect, DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS);
}

BOOL CWTFPropertyGridProperty::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CString strText;
	m_pWndInPlace->GetWindowText(strText);

	BOOL bRes = FALSE;
	BOOL bIsChanged = FormatProperty() != strText;

	if (m_bIsValueList)
	{
		CString strDelimeter(m_pWndList->m_cListDelimeter);

		for (int i = 0; !strText.IsEmpty() && i < GetSubItemsCount(); i++)
		{
			CString strItem = strText.SpanExcluding(strDelimeter);

			if (strItem.GetLength() + 1 > strText.GetLength())
			{
				strText.Empty();
			}
			else
			{
				strText = strText.Mid(strItem.GetLength() + 1);
			}

			strItem.TrimLeft();
			strItem.TrimRight();

			CWTFPropertyGridProperty* pSubItem = GetSubItem(i);
			ASSERT_VALID(pSubItem);

			pSubItem->TextToVar(strItem);
		}

		bRes = TRUE;
	}
	else
	{
		bRes = TextToVar(strText);
	}

	if (bRes && bIsChanged)
	{
		m_pWndList->OnPropertyChanged(this);
	}

	return bRes;
}

BOOL CWTFPropertyGridProperty::TextToVar(const CString& strText)
{
	CString strVal = strText;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
		m_varValue = (LPCTSTR) strVal;
		return TRUE;

	case VT_UI1:
		m_varValue = strVal.IsEmpty() ?(BYTE) 0 :(BYTE) strVal[0];
		return TRUE;

	case VT_I2:
		m_varValue = (short) _ttoi(strVal);
		return TRUE;

	case VT_I4:
	case VT_I8:
	case VT_INT:
		m_varValue = _ttol(strVal);
		return TRUE;

	case VT_UI2:
		m_varValue.uiVal = unsigned short(_ttoi(strVal));
		return TRUE;

	case VT_UI4:
	case VT_UI8:
	case VT_UINT:
#ifdef _UNICODE
		m_varValue.ulVal = wcstoul(strText, NULL, 10);
#else
		m_varValue.ulVal = strtoul(strText, NULL, 10);
#endif // _UNICODE
		return TRUE;

	case VT_R4:
		{
			float fVal = 0.;

			strVal.TrimLeft();
			strVal.TrimRight();

			if (!strVal.IsEmpty())
			{
				_stscanf_s(strVal, AFX_FORMAT_FLOAT, &fVal);
			}

			m_varValue = fVal;
		}
		return TRUE;

	case VT_R8:
		{
			double dblVal = 0.;

			strVal.TrimLeft();
			strVal.TrimRight();

			if (!strVal.IsEmpty())
			{
				_stscanf_s(strVal, AFX_FORMAT_DOUBLE, &dblVal);
			}

			m_varValue = dblVal;
		}
		return TRUE;

	case VT_BOOL:
		strVal.TrimRight();
		m_varValue = (VARIANT_BOOL)(strVal == m_pWndList->m_strTrue);
		return TRUE;
	}

	return FALSE;
}

BOOL CWTFPropertyGridProperty::IsValueChanged() const
{
	ASSERT_VALID(this);

	if (m_varValueOrig.vt != m_varValue.vt)
	{
		return FALSE;
	}

	const COleVariant& var = m_varValue;
	const COleVariant& var1 = m_varValueOrig;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
		{
			CString str1 = var.bstrVal;
			CString str2 = var1.bstrVal;

			return str1 != str2;
		}
		break;

	case VT_I2:
		return(short)var.iVal != (short)var1.iVal;

	case VT_I4:
	case VT_I8:
	case VT_INT:
		return(long)var.lVal != (long)var1.lVal;

	case VT_UI1:
		return(BYTE)var.bVal != (BYTE)var1.bVal;

	case VT_UI2:
		return var.uiVal != var1.uiVal;

	case VT_UI4:
	case VT_UI8:
	case VT_UINT:
		return var.ulVal != var1.ulVal;

	case VT_R4:
		return(float)var.fltVal != (float)var1.fltVal;

	case VT_R8:
		return(double)var.dblVal != (double)var1.dblVal;

	case VT_BOOL:
		return(VARIANT_BOOL)var.boolVal != (VARIANT_BOOL)var1.boolVal;
	}

	return FALSE;
}

BOOL CWTFPropertyGridProperty::OnEdit(LPPOINT /*lptClick*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (!HasValueField())
	{
		return FALSE;
	}

	m_pWndInPlace = NULL;

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect(rectEdit, rectSpin);

	BOOL bDefaultFormat = FALSE;
	m_pWndInPlace = CreateInPlaceEdit(rectEdit, bDefaultFormat);

	if (m_pWndInPlace != NULL)
	{
		if (bDefaultFormat)
		{
			m_pWndInPlace->SetWindowText(FormatProperty());
		}

		if (m_dwFlags & AFX_PROP_HAS_LIST)
		{
			CRect rectCombo = m_Rect;
			rectCombo.left = rectEdit.left - 4;

			m_pWndCombo = CreateCombo(m_pWndList, rectCombo);
			ASSERT_VALID(m_pWndCombo);

			m_pWndCombo->SetFont(m_pWndList->GetFont());

			// Synchronize bottom edge of the combobox with the property bottom edge:
			m_pWndCombo->GetWindowRect(rectCombo);
			m_pWndList->ScreenToClient(&rectCombo);

			int dy = rectCombo.Height() - m_Rect.Height();

			m_pWndCombo->SetWindowPos(NULL, rectCombo.left, rectCombo.top - dy + 1, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			if (m_varValue.vt == VT_BOOL)
			{
				m_lstOptions.AddTail(m_pWndList->m_strTrue);
				m_lstOptions.AddTail(m_pWndList->m_strFalse);
			}

			for (POSITION pos = m_lstOptions.GetHeadPosition(); pos != NULL;)
			{
				m_pWndCombo->AddString(m_lstOptions.GetNext(pos));
			}
		}

		if (m_dwFlags & AFX_PROP_HAS_SPIN)
		{
			m_pWndSpin = CreateSpinControl(rectSpin);
		}

		m_pWndInPlace->SetFont(IsModified() && m_pWndList->m_bMarkModifiedProperties ? &m_pWndList->m_fontBold : m_pWndList->GetFont());
		m_pWndInPlace->SetFocus();

		if (!m_bAllowEdit)
		{
			m_pWndInPlace->HideCaret();
		}

		m_bInPlaceEdit = TRUE;
		return TRUE;
	}

	return FALSE;
}

void CWTFPropertyGridProperty::AdjustButtonRect()
{
	ASSERT_VALID(this);

	m_rectButton = m_Rect;
	m_rectButton.left = m_rectButton.right - m_rectButton.Height() + 3;
	m_rectButton.top ++;
}

void CWTFPropertyGridProperty::AdjustInPlaceEditRect(CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	rectEdit = m_Rect;
	rectEdit.DeflateRect(0, 2);

	int nMargin = m_pWndList->m_bMarkModifiedProperties && m_bIsModified ? m_pWndList->m_nBoldEditLeftMargin : m_pWndList->m_nEditLeftMargin;

	rectEdit.left = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth + AFX_TEXT_MARGIN - nMargin + 1;

	if (HasButton())
	{
		AdjustButtonRect();
		rectEdit.right = m_rectButton.left;
	}

	if (m_dwFlags & AFX_PROP_HAS_SPIN)
	{
		rectSpin = m_Rect;
		rectSpin.right = rectEdit.right;
		rectSpin.left = rectSpin.right - rectSpin.Height();
		rectSpin.top ++;
		rectEdit.right = rectSpin.left;
	}
	else
	{
		rectSpin.SetRectEmpty();
	}
}

CWnd* CWTFPropertyGridProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI2:
	case VT_UI4:
	case VT_UI8:
	case VT_BOOL:
		break;

	default:
		if (!m_bIsValueList)
		{
			return NULL;
		}
	}

	CEdit* pWndEdit = NULL;

	if (!m_strEditMask.IsEmpty() || !m_strEditTempl.IsEmpty() || !m_strValidChars.IsEmpty())
	{
		CMFCMaskedEdit* pWndEditMask = new CMFCMaskedEdit;
		pWndEditMask->EnableSetMaskedCharsOnly(FALSE);
		pWndEditMask->EnableGetMaskedCharsOnly(FALSE);

		if (!m_strEditMask.IsEmpty() && !m_strEditTempl.IsEmpty())
		{
			pWndEditMask->EnableMask(m_strEditMask, m_strEditTempl, _T(' '));
		}

		if (!m_strValidChars.IsEmpty())
		{
			pWndEditMask->SetValidChars(m_strValidChars);
		}

		pWndEdit = pWndEditMask;
	}
	else
	{
		pWndEdit = new CEdit;
	}

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL;

	if (!m_bEnabled || !m_bAllowEdit)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);

	bDefaultFormat = TRUE;
	return pWndEdit;
}

CSpinButtonCtrl* CWTFPropertyGridProperty::CreateSpinControl(CRect rectSpin)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	CSpinButtonCtrl* pWndSpin = new CMFCSpinButtonCtrl;

	if (!pWndSpin->Create(WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_NOTHOUSANDS, rectSpin, m_pWndList, AFX_PROPLIST_ID_INPLACE))
	{
		return NULL;
	}

	pWndSpin->SetBuddy(m_pWndInPlace);

	if (m_nMinValue != 0 || m_nMaxValue != 0)
	{
		pWndSpin->SetRange32(m_nMinValue, m_nMaxValue);
	}

	return pWndSpin;
}

BOOL CWTFPropertyGridProperty::OnEndEdit()
{
	ASSERT_VALID(this);

	m_bInPlaceEdit = FALSE;
	m_bButtonIsFocused = FALSE;

	OnDestroyWindow();
	return TRUE;
}

CComboBox* CWTFPropertyGridProperty::CreateCombo(CWnd* pWndParent, CRect rect)
{
	ASSERT_VALID(this);

	rect.bottom = rect.top + 400;

	CComboBox* pWndCombo = new CComboBox;
	if (!pWndCombo->Create(WS_CHILD | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL, rect, pWndParent, AFX_PROPLIST_ID_INPLACE))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}

#if 0
CWTFColorSlidersPopupMenu* CWTFPropertyGridProperty::CreateColorSliderEditor(CWnd* pWndParent, CRect rect)
{
	ASSERT_VALID(this);

	rect.bottom = rect.top + 200;

	CWTFColorSlidersPopupMenu *pcsp = new CWTFColorSlidersPopupMenu();
	if (!pcsp->Create(WS_CHILD | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL, rect, pWndParent, AFX_PROPLIST_ID_INPLACE))
	{
		delete pcsp;
		return NULL;
	}

	return pcsp;
}
#endif

void CWTFPropertyGridProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_pWndCombo != NULL)
	{
		m_bButtonIsDown = TRUE;
		Redraw();

		CString str;
		m_pWndInPlace->GetWindowText(str);

		m_pWndCombo->SetCurSel(m_pWndCombo->FindStringExact(-1, str));

		m_pWndCombo->SetFocus();
		m_pWndCombo->ShowDropDown();
	}
}

BOOL CWTFPropertyGridProperty::OnClickValue(UINT uiMsg, CPoint point)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pWndInPlace);

	if (m_pWndSpin != NULL)
	{
		ASSERT_VALID(m_pWndSpin);
		ASSERT(m_pWndSpin->GetSafeHwnd() != NULL);

		CRect rectSpin;
		m_pWndSpin->GetClientRect(rectSpin);
		m_pWndSpin->MapWindowPoints(m_pWndList, rectSpin);

		if (rectSpin.PtInRect(point))
		{
			m_pWndList->MapWindowPoints(m_pWndSpin, &point, 1);

			m_pWndSpin->SendMessage(uiMsg, 0, MAKELPARAM(point.x, point.y));
			return TRUE;
		}
	}

	CPoint ptEdit = point;
	::MapWindowPoints( m_pWndList->GetSafeHwnd(), m_pWndInPlace->GetSafeHwnd(), &ptEdit, 1);

	m_pWndInPlace->SendMessage(uiMsg, 0, MAKELPARAM(ptEdit.x, ptEdit.y));
	return TRUE;
}

BOOL CWTFPropertyGridProperty::OnDblClk(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	if (m_lstOptions.GetCount() > 1)
	{
		CString strText;
		m_pWndInPlace->GetWindowText(strText);

		POSITION pos = m_lstOptions.Find(strText);
		if (pos == NULL)
		{
			return FALSE;
		}

		m_lstOptions.GetNext(pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetHeadPosition();
		}

		ENSURE(pos != NULL);
		strText = m_lstOptions.GetAt(pos);

		m_pWndInPlace->SetWindowText(strText);
		OnUpdateValue();

		return TRUE;
	}

	if (m_dwFlags & AFX_PROP_HAS_BUTTON)
	{
		CWaitCursor wait;

		CString strPrevVal = FormatProperty();

		OnClickButton(CPoint(-1, -1));

		if (strPrevVal != FormatProperty())
		{
			m_pWndList->OnPropertyChanged(this);
		}

		return TRUE;
	}

	return FALSE;
}

void CWTFPropertyGridProperty::OnSelectCombo()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndCombo);
	ASSERT_VALID(m_pWndInPlace);

	int iSelIndex = m_pWndCombo->GetCurSel();
	if (iSelIndex >= 0)
	{
		CString str;
		m_pWndCombo->GetLBText(iSelIndex, str);
		m_pWndInPlace->SetWindowText(str);
		OnUpdateValue();
	}
}

void CWTFPropertyGridProperty::OnCloseCombo()
{
	ASSERT_VALID(this);

	m_bButtonIsDown = FALSE;
	Redraw();

	ASSERT_VALID(m_pWndInPlace);
	m_pWndInPlace->SetFocus();
}

BOOL CWTFPropertyGridProperty::OnSetCursor() const
{
	if (m_bInPlaceEdit)
	{
		return FALSE;
	}

	if (m_bIsValueList)
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_IBEAM));
		return TRUE;
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI2:
	case VT_UI4:
	case VT_UI8:
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_IBEAM));
		return TRUE;
	}

	return FALSE;
}

BOOL CWTFPropertyGridProperty::PushChar(UINT nChar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT(m_pWndList->m_pSel == this);
	ASSERT_VALID(m_pWndInPlace);

	if (m_bIsValueList)
	{
		if (m_bEnabled && m_bAllowEdit && nChar != _T('+') && nChar != _T('-'))
		{
			m_pWndInPlace->SetWindowText(_T(""));
			m_pWndInPlace->SendMessage(WM_CHAR, (WPARAM) nChar);
			return TRUE;
		}
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI2:
	case VT_UI4:
	case VT_UI8:
		if (m_bEnabled && m_bAllowEdit)
		{
			m_pWndInPlace->SetWindowText(_T(""));
			m_pWndInPlace->SendMessage(WM_CHAR, (WPARAM) nChar);
			return TRUE;
		}
	}

	if (!m_bAllowEdit)
	{
		if (nChar == VK_SPACE)
		{
			OnDblClk(CPoint(-1, -1));
		}
		else if (m_lstOptions.GetCount() > 1)
		{
			CString strText;
			m_pWndInPlace->GetWindowText(strText);

			POSITION pos = m_lstOptions.Find(strText);
			if (pos == NULL)
			{
				return FALSE;
			}

			POSITION posSave = pos;
			CString strChar((TCHAR) nChar);
			strChar.MakeUpper();

			m_lstOptions.GetNext(pos);

			while (pos != posSave)
			{
				if (pos == NULL)
				{
					pos = m_lstOptions.GetHeadPosition();
				}

				if (pos == posSave)
				{
					break;
				}

				strText = m_lstOptions.GetAt(pos);

				CString strUpper = strText;
				strUpper.MakeUpper();

				if (strUpper.Left(1) == strChar)
				{
					m_pWndInPlace->SetWindowText(strText);
					OnUpdateValue();
					break;
				}

				m_lstOptions.GetNext(pos);
			}
		}
	}

	OnEndEdit();

	if (::GetCapture() == m_pWndList->GetSafeHwnd())
	{
		ReleaseCapture();
	}

	return FALSE;
}

CString CWTFPropertyGridProperty::GetNameTooltip()
{
	ASSERT_VALID(this);
	return _T("");
}

CString CWTFPropertyGridProperty::GetValueTooltip()
{
	ASSERT_VALID(this);
	return _T("");
}

HBRUSH CWTFPropertyGridProperty::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_pWndList->m_clrText != (COLORREF)-1)
	{
		pDC->SetTextColor(m_pWndList->m_clrText);
	}

	if (m_pWndList->m_brBackground.GetSafeHandle() != NULL)
	{
		if (m_pWndList->m_clrBackground != -1)
		{
			pDC->SetBkColor(m_pWndList->m_clrBackground);
		}

		return(HBRUSH) m_pWndList->m_brBackground.GetSafeHandle();
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_INT:
	case VT_UINT:
	case VT_UI2:
	case VT_UI4:
	case VT_UI8:
	case VT_BOOL:
		if (!m_bEnabled || !m_bAllowEdit)
		{
			pDC->SetBkColor(GetGlobalData()->clrWindow);
			return(HBRUSH) GetGlobalData()->brWindow.GetSafeHandle();
		}
	}

	return NULL;
}

void CWTFPropertyGridProperty::SetModifiedFlag()
{
	ASSERT_VALID(m_pWndList);

	BOOL bIsModified = IsValueChanged();

	if (m_bIsModified == bIsModified && !m_bIsValueList)
	{
		return;
	}

	m_bIsModified = bIsModified;

	if (m_pParent != NULL && m_pParent->m_bIsValueList)
	{
		if (bIsModified)
		{
			m_pParent->m_bIsModified = TRUE;
		}
		else
		{
			m_pParent->m_bIsModified = FALSE;

			for (POSITION pos = m_pParent->m_lstSubItems.GetHeadPosition(); pos != NULL;)
			{
				CWTFPropertyGridProperty* pSubItem = (CWTFPropertyGridProperty*) m_pParent->m_lstSubItems.GetNext(pos);
				ASSERT_VALID(pSubItem);

				if (pSubItem->m_bIsModified)
				{
					m_pParent->m_bIsModified = TRUE;
					break;
				}
			}
		}
	}

	if (m_bIsValueList)
	{
		for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
		{
			CWTFPropertyGridProperty* pSubItem = (CWTFPropertyGridProperty*)m_lstSubItems.GetNext(pos);
			ASSERT_VALID(pSubItem);

			if (pSubItem->m_bIsModified)
			{
				m_bIsModified = TRUE;
				break;
			}
		}
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pSubItem = (CWTFPropertyGridProperty*) m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pSubItem);

		pSubItem->SetModifiedFlag();
	}

	if (m_pWndList->m_bMarkModifiedProperties)
	{
		OnPosSizeChanged(m_Rect);

		if (m_pWndInPlace->GetSafeHwnd() != NULL)
		{
			if (m_bIsModified)
			{
				m_pWndInPlace->SetFont(&m_pWndList->m_fontBold);
			}
			else
			{
				m_pWndInPlace->SetFont(m_pWndList->GetFont());
			}

			CRect rectInPlace;
			m_pWndInPlace->GetWindowRect(&rectInPlace);
			m_pWndList->ScreenToClient(&rectInPlace);

			int nXOffset = m_pWndList->m_nBoldEditLeftMargin - m_pWndList->m_nEditLeftMargin;

			if (m_bIsModified)
			{
				nXOffset = -nXOffset;
			}

			if (HasButton())
			{
				AdjustButtonRect();
				rectInPlace.right = m_rectButton.left;
			}

			m_pWndInPlace->SetWindowPos(NULL, rectInPlace.left + nXOffset, rectInPlace.top, rectInPlace.Width() - nXOffset, rectInPlace.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}

		Redraw();
	}
}

void CWTFPropertyGridProperty::Show(BOOL bShow/* = TRUE*/, BOOL bAdjustLayout/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (m_bIsVisible == bShow)
	{
		return;
	}

	m_bIsVisible = bShow;

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->Show(bShow, FALSE);
	}

	if (bAdjustLayout && m_pWndList != NULL && m_pWndList->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->AdjustLayout();
	}
}

BOOL CWTFPropertyGridProperty::OnActivateByTab()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_bInPlaceEdit &&(m_dwFlags & AFX_PROP_HAS_BUTTON))
	{
		m_bButtonIsFocused = !m_bButtonIsFocused;
		m_pWndList->RedrawWindow(m_rectButton);
		return TRUE;
	}

	if (!m_bInPlaceEdit && m_bEnabled && m_pWndList->EditItem(this))
	{
		CEdit* pEdit = DYNAMIC_DOWNCAST(CEdit, m_pWndInPlace);
		if (::IsWindow(pEdit->GetSafeHwnd()))
		{
			pEdit->SetSel(0, -1);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CWTFPropertyGridProperty::OnRotateListValue(BOOL bForward)
{
	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	CString strText;
	m_pWndInPlace->GetWindowText(strText);

	POSITION pos = m_lstOptions.Find(strText);
	if (pos == NULL)
	{
		return FALSE;
	}

	if (bForward)
	{
		m_lstOptions.GetNext(pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetHeadPosition();
		}
	}
	else
	{
		m_lstOptions.GetPrev(pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetTailPosition();
		}
	}

	if (pos == NULL)
	{
		return FALSE;
	}

	strText = m_lstOptions.GetAt(pos);

	m_pWndInPlace->SetWindowText(strText);
	OnUpdateValue();

	CEdit* pEdit = DYNAMIC_DOWNCAST(CEdit, m_pWndInPlace);
	if (::IsWindow(pEdit->GetSafeHwnd()))
	{
		pEdit->SetSel(0, -1);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridColorProperty object

IMPLEMENT_DYNAMIC(CWTFPropertyGridColorProperty, CWTFPropertyGridProperty)

CWTFPropertyGridColorProperty::CWTFPropertyGridColorProperty(const CString& strName, const COLORREF& color, CPalette* pPalette, LPCTSTR lpszDescr, DWORD_PTR dwData) : CWTFPropertyGridProperty(strName, COleVariant(), lpszDescr, dwData), m_Color(color), m_ColorOrig(color)
{
	CWTFColorBar::InitColors(pPalette, m_Colors);

	m_varValue = (LONG) color;
	m_varValueOrig = (LONG) color;

	m_dwFlags = AFX_PROP_HAS_LIST;

	m_pPopup = NULL;
	m_bStdColorDlg = true;
	m_ColorAutomatic = RGB(0, 0, 0);
	m_nColumnsNumber = 5;
}

CWTFPropertyGridColorProperty::~CWTFPropertyGridColorProperty()
{
}

void CWTFPropertyGridColorProperty::OnDrawValue(CDC* pDC, CRect rect)
{
	CRect rectColor = rect;

	rect.left += rect.Height();
	CWTFPropertyGridProperty::OnDrawValue(pDC, rect);

	rectColor.right = rectColor.left + rectColor.Height();
	rectColor.DeflateRect(1, 1);
	rectColor.top++;
	rectColor.left++;

	CBrush br(m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);
	pDC->FillRect(rectColor, &br);
	pDC->Draw3dRect(rectColor, 0, 0);
}

void CWTFPropertyGridColorProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	m_bButtonIsDown = TRUE;
	Redraw();

	CList<COLORREF,COLORREF> lstDocColors;

	m_pPopup = new CWTFColorPopupMenu(NULL, m_Colors, m_Color, NULL, NULL, NULL, lstDocColors, m_nColumnsNumber, m_ColorAutomatic);
	m_pPopup->SetPropList(m_pWndList);

	if (!m_strOtherColor.IsEmpty()) // Other color button
	{
		m_pPopup->m_wndColorBar.EnableOtherButton(m_strOtherColor, !m_bStdColorDlg);
	}

	if (!m_strAutoColor.IsEmpty()) // Automatic color button
	{
		m_pPopup->m_wndColorBar.EnableAutomaticButton(m_strAutoColor, m_ColorAutomatic);
	}

	CPoint pt(m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth + 1, m_rectButton.bottom + 1);
	m_pWndList->ClientToScreen(&pt);

	if (!m_pPopup->Create(m_pWndList, pt.x, pt.y, NULL, FALSE))
	{
		ASSERT(FALSE);
		m_pPopup = NULL;
	}
	else
	{
		m_pPopup->GetMenuBar()->SetFocus();
	}
}

BOOL CWTFPropertyGridColorProperty::OnEdit(LPPOINT /*lptClick*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	m_pWndInPlace = NULL;

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect(rectEdit, rectSpin);

	CMFCMaskedEdit* pWndEdit = new CMFCMaskedEdit;
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	if (!m_bEnabled)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->SetValidChars(_T("01234567890ABCDEFabcdef"));

	pWndEdit->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	m_pWndInPlace = pWndEdit;

	m_pWndInPlace->SetWindowText(FormatProperty());

	m_pWndInPlace->SetFont(IsModified() && m_pWndList->m_bMarkModifiedProperties ? &m_pWndList->m_fontBold : m_pWndList->GetFont());
	m_pWndInPlace->SetFocus();

	m_bInPlaceEdit = TRUE;
	return TRUE;
}

void CWTFPropertyGridColorProperty::AdjustInPlaceEditRect(CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	rectSpin.SetRectEmpty();

	rectEdit = m_Rect;
	rectEdit.DeflateRect(0, 2);

	int nMargin = m_pWndList->m_bMarkModifiedProperties && m_bIsModified ? m_pWndList->m_nBoldEditLeftMargin : m_pWndList->m_nEditLeftMargin;

	rectEdit.left = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth + m_Rect.Height() + AFX_TEXT_MARGIN - nMargin + 1;

	AdjustButtonRect();
	rectEdit.right = m_rectButton.left;
}

void CWTFPropertyGridColorProperty::SetOriginalValue(const COleVariant& varValue)
{
	CWTFPropertyGridProperty::SetOriginalValue(varValue);
	m_ColorOrig = (COLORREF)varValue.lVal;
}

void CWTFPropertyGridColorProperty::ResetOriginalValue()
{
	CWTFPropertyGridProperty::ResetOriginalValue();
	m_Color = m_ColorOrig;
}

CString CWTFPropertyGridColorProperty::FormatProperty()
{
	ASSERT_VALID(this);

	if (m_Color == (COLORREF)-1)
	{
		return m_strAutoColor;
	}

	CString str;
	str.Format(_T("%02x%02x%02x"), GetRValue(m_Color), GetGValue(m_Color), GetBValue(m_Color));

	return str;
}

void CWTFPropertyGridColorProperty::SetColor(COLORREF color)
{
	ASSERT_VALID(this);

	if ((m_Color == color) && (m_varValue == (LONG)color))
		return;

	m_Color = color;
	m_varValue = (LONG) color;

	if (::IsWindow(m_pWndList->GetSafeHwnd()))
	{
		CRect rect = m_Rect;
		rect.DeflateRect(0, 1);

		m_pWndList->InvalidateRect(rect);
		m_pWndList->UpdateWindow();
	}

	if (m_pWndInPlace != NULL)
	{
		ASSERT_VALID(m_pWndInPlace);
		m_pWndInPlace->SetWindowText(FormatProperty());
	}
}

void CWTFPropertyGridColorProperty::SetColumnsNumber(int nColumnsNumber)
{
	ASSERT_VALID(this);
	ASSERT(nColumnsNumber > 0);

	m_nColumnsNumber = nColumnsNumber;
}

void CWTFPropertyGridColorProperty::EnableAutomaticButton(LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	ASSERT_VALID(this);

	m_ColorAutomatic = colorAutomatic;
	m_strAutoColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}

void CWTFPropertyGridColorProperty::EnableOtherButton(LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	ASSERT_VALID(this);

	m_bStdColorDlg = !bAltColorDlg;
	m_strOtherColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}

BOOL CWTFPropertyGridColorProperty::OnUpdateValue()
{
	ASSERT_VALID(this);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CString strText;
	m_pWndInPlace->GetWindowText(strText);

	COLORREF colorCurr = m_Color;

	if (!strText.IsEmpty())
	{
		int nR = 0, nG = 0, nB = 0;
		_stscanf_s(strText, _T("%2x%2x%2x"), &nR, &nG, &nB);
		m_Color = RGB(nR, nG, nB);
	}

	if (colorCurr != m_Color)
	{
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridFileProperty object

IMPLEMENT_DYNAMIC(CWTFPropertyGridFileProperty, CWTFPropertyGridProperty)

CWTFPropertyGridFileProperty::CWTFPropertyGridFileProperty(const CString& strName, const CString& strFolderName, DWORD_PTR dwData, LPCTSTR lpszDescr) : CWTFPropertyGridProperty(strName, COleVariant((LPCTSTR)strFolderName), lpszDescr, dwData), m_bIsFolder(TRUE)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
}

CWTFPropertyGridFileProperty::CWTFPropertyGridFileProperty( const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt, DWORD dwFileFlags, LPCTSTR lpszFilter, LPCTSTR lpszDescr, DWORD_PTR dwData) : CWTFPropertyGridProperty(strName, COleVariant((LPCTSTR)strFileName), lpszDescr, dwData), m_bOpenFileDialog(bOpenFileDialog), m_dwFileOpenFlags(dwFileFlags)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
	m_strDefExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
	m_strFilter = lpszFilter == NULL ? _T("") : lpszFilter;

	m_bIsFolder  = FALSE;
}

CWTFPropertyGridFileProperty::~CWTFPropertyGridFileProperty()
{
}

void CWTFPropertyGridFileProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	m_bButtonIsDown = TRUE;
	Redraw();

	CString strPath = m_varValue.bstrVal;
	BOOL bUpdate = FALSE;

	if (m_bIsFolder)
	{
		if (afxShellManager == NULL)
		{
			CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
			if (pApp != NULL)
			{
				pApp->InitShellManager();
			}
		}

		if (afxShellManager == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			bUpdate = afxShellManager->BrowseForFolder(strPath, m_pWndList, strPath);
		}
	}
	else
	{
		CFileDialog dlg(m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter, m_pWndList);
		if (dlg.DoModal() == IDOK)
		{
			bUpdate = TRUE;
			strPath = dlg.GetPathName();
		}
	}

	if (bUpdate)
	{
		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText(strPath);
		}

		m_varValue = (LPCTSTR) strPath;
	}

	m_bButtonIsDown = FALSE;
	Redraw();

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridFontProperty object

IMPLEMENT_DYNAMIC(CWTFPropertyGridFontProperty, CWTFPropertyGridProperty)

CWTFPropertyGridFontProperty::CWTFPropertyGridFontProperty(const CString& strName, LOGFONT& lf, DWORD dwFontDialogFlags, LPCTSTR lpszDescr, DWORD_PTR dwData, COLORREF color) : CWTFPropertyGridProperty(strName, COleVariant((LPCTSTR)lf.lfFaceName), lpszDescr, dwData), m_dwFontDialogFlags(dwFontDialogFlags)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
	m_lf = lf;
	m_lfOrig = lf;
	m_bAllowEdit = FALSE;
	m_Color = color;
}

CWTFPropertyGridFontProperty::~CWTFPropertyGridFontProperty()
{
}

void CWTFPropertyGridFontProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	LOGFONT lfPrev = m_lf;
	COLORREF nColorPrev = m_Color;

	m_bButtonIsDown = TRUE;
	Redraw();

#pragma warning(disable : 4244)

	CFontDialog dlg(&m_lf, m_dwFontDialogFlags, NULL, m_pWndList);

#pragma warning(default : 4244)

	if (m_Color != (COLORREF)-1)
	{
		dlg.m_cf.rgbColors = m_Color;
	}

	if (dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&m_lf);
		m_Color = dlg.GetColor();

		if (memcmp(&lfPrev, &m_lf, sizeof(lfPrev) - sizeof(lfPrev.lfFaceName)) || _tcscmp( lfPrev.lfFaceName, m_lf.lfFaceName) || nColorPrev != m_Color)
		{
			m_pWndList->OnPropertyChanged(this);
		}

		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText(FormatProperty());
		}
		else
		{
			m_varValue = (LPCTSTR) FormatProperty();
		}
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}

	m_bButtonIsDown = FALSE;
	Redraw();
}

CString CWTFPropertyGridFontProperty::FormatProperty()
{
	CString str;
	CWindowDC dc(m_pWndList);

	str.Format( _T("%Ts(%i)"), m_lf.lfFaceName, m_lf.lfHeight);

	return str;
}

void CWTFPropertyGridFontProperty::ResetOriginalValue()
{
	CWTFPropertyGridProperty::ResetOriginalValue();
	m_lf = m_lfOrig;
}

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridCtrl

IMPLEMENT_DYNAMIC(CWTFPropertyGridCtrl, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridCtrl notification messages:

UINT CWTFPropertyGridCtrl::AFX_WM_PROPERTY_CHANGED = ::RegisterWindowMessage(_T("AFX_WM_PROPERTYCHANGED"));

CWTFPropertyGridCtrl::CWTFPropertyGridCtrl()
{
	m_hFont = NULL;
	m_nEditLeftMargin = 0;
	m_nBoldEditLeftMargin = 0;
	m_bHeaderCtrl = FALSE;
	m_bDescriptionArea = FALSE;
	m_nDescrHeight = -1;
	m_nDescrRows = 3;
	m_bAlphabeticMode = FALSE;
	m_bVSDotNetLook = FALSE;
	m_rectList.SetRectEmpty();
	m_nLeftColumnWidth = 0;
	m_rectTrackHeader.SetRectEmpty();
	m_rectTrackDescr.SetRectEmpty();
	m_nRowHeight = 0;
	m_nRowVerticalPadding = 12;
	m_nHeaderHeight = 0;
	m_nSubItemIndent = 5;
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_pSel = NULL;
	m_bFocused = FALSE;
	m_nTooltipsCount = 0;
	m_bAlwaysShowUserTT = TRUE;
	m_bTracking = FALSE;
	m_bTrackingDescr = FALSE;

	m_strTrue = _T("True");
	m_strFalse = _T("False");

	m_cListDelimeter = _T(',');

	m_bControlBarColors = FALSE;
	m_bGroupNameFullWidth = TRUE;
	m_bShowDragContext = TRUE;

	m_clrBackground = (COLORREF)-1;
	m_clrText = (COLORREF)-1;
	m_clrGroupBackground = (COLORREF)-1;
	m_clrGroupText = (COLORREF)-1;
	m_clrDescriptionBackground = (COLORREF)-1;
	m_clrDescriptionText = (COLORREF)-1;
	m_clrLine = (COLORREF)-1;

	m_bMarkModifiedProperties = FALSE;

	m_pAccProp = NULL;
	EnableActiveAccessibility();
}

CWTFPropertyGridCtrl::~CWTFPropertyGridCtrl()
{
}

BEGIN_MESSAGE_MAP(CWTFPropertyGridCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_STYLECHANGED()
	ON_WM_SETFONT()
	ON_WM_GETFONT()
	ON_MESSAGE(AFX_UM_UPDATESPIN, &CWTFPropertyGridCtrl::OnUpdateSpin)
	ON_MESSAGE(WM_GETOBJECT, &CWTFPropertyGridCtrl::OnGetObject)
	ON_MESSAGE(WM_MFC_INITCTRL, &CWTFPropertyGridCtrl::OnInitControl)
	ON_NOTIFY(HDN_ITEMCHANGED, AFX_ID_HEADER, &CWTFPropertyGridCtrl::OnHeaderItemChanged)
	ON_NOTIFY(HDN_TRACK, AFX_ID_HEADER, &CWTFPropertyGridCtrl::OnHeaderTrack)
	ON_NOTIFY(HDN_ENDTRACK, AFX_ID_HEADER, &CWTFPropertyGridCtrl::OnHeaderEndTrack)
	ON_NOTIFY(UDN_DELTAPOS, AFX_PROPLIST_ID_INPLACE, &CWTFPropertyGridCtrl::OnSpinDeltaPos)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CWTFPropertyGridCtrl::OnNeedTipText)
	ON_EN_KILLFOCUS(AFX_PROPLIST_ID_INPLACE, &CWTFPropertyGridCtrl::OnEditKillFocus)
	ON_CBN_SELENDOK(AFX_PROPLIST_ID_INPLACE, &CWTFPropertyGridCtrl::OnSelectCombo)
	ON_CBN_CLOSEUP(AFX_PROPLIST_ID_INPLACE, &CWTFPropertyGridCtrl::OnCloseCombo)
	ON_CBN_KILLFOCUS(AFX_PROPLIST_ID_INPLACE, &CWTFPropertyGridCtrl::OnComboKillFocus)
	ON_WM_PRINTCLIENT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWTFPropertyGridCtrl message handlers

void CWTFPropertyGridCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init();
	}
}

int CWTFPropertyGridCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init();
	return 0;
}

void CWTFPropertyGridCtrl::Init()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (GetGlobalData()->m_hcurStretch == NULL)
	{
		GetGlobalData()->m_hcurStretch = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR);
	}

	if (GetGlobalData()->m_hcurStretchVert == NULL)
	{
		GetGlobalData()->m_hcurStretchVert = AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR);
	}

	InitHeader();

	HDITEM hdItem;
	hdItem.mask = HDI_TEXT | HDI_FORMAT;
	hdItem.fmt = HDF_LEFT;
	hdItem.pszText = _T("Property");
	hdItem.cchTextMax = 100;

	GetHeaderCtrl().InsertItem(0, &hdItem);

	hdItem.pszText = _T("Value");
	hdItem.cchTextMax = 100;

	GetHeaderCtrl().InsertItem(1, &hdItem);

	m_wndScrollVert.Create(WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, AFX_ID_SCROLL_VERT);

	m_ToolTip.Create(this, TTS_ALWAYSTIP);
	m_ToolTip.Activate(TRUE);
	if (GetGlobalData()->m_nMaxToolTipWidth != -1)
	{
		m_ToolTip.SetMaxTipWidth(GetGlobalData()->m_nMaxToolTipWidth);
	}

	m_ToolTip.SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);

	m_IPToolTip.Create(this);

	CWnd* pWndParent = GetParent();
	m_bControlBarColors = pWndParent == NULL || !pWndParent->IsKindOf(RUNTIME_CLASS(CDialog));

	AdjustLayout();
	CreateBoldFont();
	CalcEditMargin();
}

void CWTFPropertyGridCtrl::InitHeader()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_wndHeader.Create(WS_CHILD | WS_VISIBLE | HDS_HORZ, rectDummy, this, AFX_ID_HEADER);
}

void CWTFPropertyGridCtrl::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CClientDC dc(this);
	HFONT hfontOld = SetCurrFont(&dc);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_nRowHeight = tm.tmHeight + m_nRowVerticalPadding;

	m_nHeaderHeight = 0;

	CRect rectClient;
	GetClientRect(rectClient);

	if (m_bHeaderCtrl)
	{
		m_nHeaderHeight = m_nRowHeight + 4;

		GetHeaderCtrl().SendMessage(WM_SETFONT, (WPARAM)(m_hFont != NULL ? m_hFont : ::GetStockObject(DEFAULT_GUI_FONT)));
		GetHeaderCtrl().SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), m_nHeaderHeight, SWP_NOZORDER | SWP_NOACTIVATE);

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl().SetItem(0, &hdItem);

		hdItem.cxy = rectClient.Width() + 10;
		GetHeaderCtrl().SetItem(1, &hdItem);

		GetHeaderCtrl().ShowWindow(SW_SHOWNOACTIVATE);
	}
	else
	{
		GetHeaderCtrl().ShowWindow(SW_HIDE);
	}

	::SelectObject(dc.GetSafeHdc(), hfontOld);

	m_rectList = rectClient;
	m_rectList.top += m_nHeaderHeight;

	if (m_bDescriptionArea && m_nDescrHeight != -1 && rectClient.Height() > 0)
	{
		m_nDescrHeight = std::max(m_nDescrHeight, m_nRowHeight);
		m_nDescrHeight = std::min(m_nDescrHeight, rectClient.Height() - m_nRowHeight);
		m_rectList.bottom -= m_nDescrHeight;
	}

	int cxScroll = ::GetSystemMetrics(SM_CXHSCROLL);
	SetScrollSizes();

	if (m_nVertScrollTotal > 0)
	{
		m_rectList.right -= cxScroll;
		m_wndScrollVert.SetWindowPos(NULL, m_rectList.right, m_rectList.top, cxScroll, m_rectList.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		m_wndScrollVert.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	ReposProperties();

	if (m_pSel != NULL && m_pSel->HasButton())
	{
		ASSERT_VALID(m_pSel);
		m_pSel->AdjustButtonRect();
	}

	RedrawWindow();
}

void CWTFPropertyGridCtrl::ReposProperties()
{
	ASSERT_VALID(this);
	m_lstTerminalProps.RemoveAll();

	if (m_ToolTip.GetSafeHwnd() != NULL)
	{
		while (m_nTooltipsCount > 0)
		{
			m_ToolTip.DelTool(this, m_nTooltipsCount);
			m_nTooltipsCount--;
		}
	}

	int y = m_rectList.top - m_nRowHeight * m_nVertScrollOffset - 1;

	if (!m_bAlphabeticMode)
	{
		for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
		{
			CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
			ASSERT_VALID(pProp);

			pProp->Reposition(y);
		}

		return;
	}

	POSITION pos = NULL;

	// Get sorted list of terminal properties:
	for (pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->AddTerminalProp(m_lstTerminalProps);
	}

	for (pos = m_lstTerminalProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstTerminalProps.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->Reposition(y);
	}
}

void CWTFPropertyGridCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	EndEditItem();

	m_nLeftColumnWidth = cx / 2;
	AdjustLayout();
}

void CWTFPropertyGridCtrl::OnSetFont(CFont* pFont, BOOL /*bRedraw*/)
{
	m_hFont = (HFONT)pFont->GetSafeHandle();

	CreateBoldFont();
	CalcEditMargin();
	AdjustLayout();
}

HFONT CWTFPropertyGridCtrl::OnGetFont()
{
	return (m_hFont != NULL) ? m_hFont : (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
}

void CWTFPropertyGridCtrl::CreateBoldFont()
{
	if (m_fontBold.GetSafeHandle() != NULL)
	{
		m_fontBold.DeleteObject();
	}

	CFont* pFont = CFont::FromHandle(m_hFont != NULL ? m_hFont :(HFONT) ::GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pFont);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	pFont->GetLogFont(&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect(&lf);
}

void CWTFPropertyGridCtrl::CalcEditMargin()
{
	CEdit editDummy;
	editDummy.Create(WS_CHILD, CRect(0, 0, 100, 20), this, (UINT)-1);

	editDummy.SetFont(GetFont());
	m_nEditLeftMargin = LOWORD(editDummy.GetMargins());

	editDummy.SetFont(&m_fontBold);
	m_nBoldEditLeftMargin = LOWORD(editDummy.GetMargins());

	editDummy.DestroyWindow();
}

HFONT CWTFPropertyGridCtrl::SetCurrFont(CDC* pDC)
{
	ASSERT_VALID(pDC);

	return(HFONT) ::SelectObject(pDC->GetSafeHdc(), m_hFont != NULL ? m_hFont : ::GetStockObject(DEFAULT_GUI_FONT));
}

void CWTFPropertyGridCtrl::OnPaint()
{
	CPaintDC dcPaint(this); // device context for painting
	OnDraw(&dcPaint);
}

void CWTFPropertyGridCtrl::OnDraw(CDC* pDCSrc)
{
	ASSERT_VALID(pDCSrc);

	CMemDC memDC(*pDCSrc, this);
	CDC* pDC = &memDC.GetDC();

#if 0
	m_clrGray = visualManager->GetPropertyGridGroupColor(this);
#else
	m_clrGray = RGB(128, 128, 128);
#endif

	CRect rectClient;
	GetClientRect(rectClient);

	OnFillBackground(pDC, rectClient);

	HFONT hfontOld = SetCurrFont(pDC);
	pDC->SetTextColor(GetTextColor());
	pDC->SetBkMode(TRANSPARENT);

	OnDrawList(pDC);

	if (m_bDescriptionArea)
	{
		CRect rectDescr = rectClient;
		rectDescr.top = m_rectList.bottom;
		if (rectDescr.Height() > 0)
		{
			rectDescr.DeflateRect(1, 1);
			OnDrawDescription(pDC, rectDescr);
		}
	}

	::SelectObject(pDC->GetSafeHdc(), hfontOld);
}

void CWTFPropertyGridCtrl::OnFillBackground(CDC* pDC, CRect rectClient)
{
	ASSERT_VALID(pDC);

	if (m_brBackground.GetSafeHandle() == NULL)
	{
		pDC->FillRect(rectClient, &(GetGlobalData()->brWindow));
	}
	else
	{
		pDC->FillRect(rectClient, &m_brBackground);
	}
}

void CWTFPropertyGridCtrl::OnDrawBorder(CDC* /*pDC*/)
{
	ASSERT(FALSE); // This method is obsolete
}

void CWTFPropertyGridCtrl::OnDrawList(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	COLORREF clrShadow = m_bControlBarColors ? GetGlobalData()->clrBarShadow : GetGlobalData()->clrBtnShadow;

	CPen penLine(PS_SOLID, 1, m_clrLine != (COLORREF)-1 ? m_clrLine :(m_bVSDotNetLook ? m_clrGray : clrShadow));
	CPen* pOldPen = pDC->SelectObject(&penLine);

	int nXCenter = m_rectList.left + m_nLeftColumnWidth;

	pDC->MoveTo(nXCenter, m_rectList.top - 1);
	pDC->LineTo(nXCenter, m_rectList.bottom);

	const CList<CWTFPropertyGridProperty*, CWTFPropertyGridProperty*>& lst = m_bAlphabeticMode ? m_lstTerminalProps : m_lstProps;

	for (POSITION pos = lst.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = lst.GetNext(pos);
		ASSERT_VALID(pProp);

		if (!OnDrawProperty(pDC, pProp))
		{
			break;
		}
	}

	pDC->SelectObject(pOldPen);
}

void CWTFPropertyGridCtrl::OnDrawDescription(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_clrDescriptionBackground != (COLORREF)-1)
	{
		CBrush br(m_clrDescriptionBackground);
		pDC->FillRect(rect, &br);
	}
	else
	{
		pDC->FillRect(rect, m_bControlBarColors ? &(GetGlobalData()->brBarFace) : &(GetGlobalData()->brBtnFace));
	}

	rect.top += AFX_TEXT_MARGIN;

	COLORREF clrShadow = m_bControlBarColors ? GetGlobalData()->clrBarShadow : GetGlobalData()->clrBtnShadow;

	pDC->Draw3dRect(rect, clrShadow, clrShadow);

	if (m_pSel == NULL)
	{
		return;
	}

	rect.DeflateRect(AFX_TEXT_MARGIN, AFX_TEXT_MARGIN);

	ASSERT_VALID(m_pSel);

	COLORREF clrTextOld = (COLORREF)-1;

	if (m_clrDescriptionText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(m_clrDescriptionText);
	}

	m_pSel->OnDrawDescription(pDC, rect);

	if (clrTextOld == (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}

BOOL CWTFPropertyGridCtrl::OnDrawProperty(CDC* pDC, CWTFPropertyGridProperty* pProp) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pProp);

	if (!pProp->m_Rect.IsRectEmpty())
	{
		if (pProp->m_Rect.top >= m_rectList.bottom)
		{
			return FALSE;
		}

		if (pProp->m_Rect.bottom >= m_rectList.top)
		{
			const int nXCenter = m_rectList.left + m_nLeftColumnWidth;
			COLORREF clrTextOld = (COLORREF)-1;

			if (m_bVSDotNetLook)
			{
				CRect rectLeft = pProp->m_Rect;

				if (!pProp->IsGroup())
				{
					rectLeft.right = std::min((LONG)nXCenter, rectLeft.left);
				}

				if (pProp->m_bIsValueList)
				{
					rectLeft.right = rectLeft.left + rectLeft.Height();
				}

				rectLeft.left = m_rectList.left;
				rectLeft.bottom = std::min(rectLeft.bottom, m_rectList.bottom);

				if (rectLeft.left < rectLeft.right)
				{
					CBrush br(m_clrGroupBackground != (COLORREF)-1 ? m_clrGroupBackground : m_clrGray);
					pDC->FillRect(rectLeft, &br);
				}
			}

			if (!pProp->IsEnabled())
			{
				clrTextOld = pDC->SetTextColor(GetGlobalData()->clrGrayedText);
			}

			CRect rectName = pProp->m_Rect;

			if ((!pProp->IsGroup() || pProp->m_bIsValueList || !m_bGroupNameFullWidth) && pProp->HasValueField())
			{
				rectName.right = nXCenter;
			}

			if (pProp->IsGroup())
			{
				if (m_bGroupNameFullWidth && !m_bVSDotNetLook && !pProp->m_bIsValueList)
				{
					CRect rectFill = rectName;
					rectFill.top++;

					if (m_brBackground.GetSafeHandle() != NULL)
					{
						CBrush& br = ((CWTFPropertyGridCtrl*) this)->m_brBackground;
						pDC->FillRect(rectFill, &br);
					}
					else
					{
						pDC->FillRect(rectFill, &(GetGlobalData()->brWindow));
					}
				}

				CRect rectExpand = rectName;
				rectName.left += m_nRowHeight;
				rectExpand.right = rectName.left;

				CRgn rgnClipExpand;
				CRect rectExpandClip = rectExpand;
				rectExpandClip.bottom = std::min(rectExpandClip.bottom, m_rectList.bottom);

				rgnClipExpand.CreateRectRgnIndirect(&rectExpandClip);
				pDC->SelectClipRgn(&rgnClipExpand);

				pProp->OnDrawExpandBox(pDC, rectExpand);
			}
			else if (!pProp->HasValueField())
			{
				CRect rectFill = rectName;
				rectFill.top++;

				if (m_brBackground.GetSafeHandle() != NULL)
				{
					CBrush& br = ((CWTFPropertyGridCtrl*) this)->m_brBackground;
					pDC->FillRect(rectFill, &br);
				}
				else
				{
					pDC->FillRect(rectFill, &(GetGlobalData()->brWindow));
				}
			}

			if (rectName.right > rectName.left)
			{
				CRgn rgnClipName;
				CRect rectNameClip = rectName;
				rectNameClip.bottom = std::min(rectNameClip.bottom, m_rectList.bottom);

				rgnClipName.CreateRectRgnIndirect(&rectNameClip);
				pDC->SelectClipRgn(&rgnClipName);

				HFONT hOldFont = NULL;
				if (pProp->IsGroup() && !pProp->m_bIsValueList)
				{
					hOldFont = (HFONT) ::SelectObject(pDC->GetSafeHdc(), m_fontBold.GetSafeHandle());
				}

				pProp->OnDrawName(pDC, rectName);

				if (hOldFont != NULL)
				{
					::SelectObject(pDC->GetSafeHdc(), hOldFont);
				}
			}

			CRect rectValue = pProp->m_Rect;
			rectValue.left = nXCenter + 1;

			CRgn rgnClipVal;
			CRect rectValClip = rectValue;
			rectValClip.bottom = std::min(rectValClip.bottom, m_rectList.bottom);

			rgnClipVal.CreateRectRgnIndirect(&rectValClip);
			pDC->SelectClipRgn(&rgnClipVal);

			pProp->OnDrawValue(pDC, rectValue);

			if (!pProp->m_rectButton.IsRectEmpty())
			{
				pProp->OnDrawButton(pDC, pProp->m_rectButton);
			}

			pDC->SelectClipRgn(NULL);

			pDC->MoveTo(m_rectList.left, pProp->m_Rect.bottom);
			pDC->LineTo(m_rectList.right, pProp->m_Rect.bottom);

			if (clrTextOld != (COLORREF)-1)
			{
				pDC->SetTextColor(clrTextOld);
			}
		}
	}

	if (pProp->IsExpanded() || m_bAlphabeticMode)
	{
		for (POSITION pos = pProp->m_lstSubItems.GetHeadPosition(); pos != NULL;)
		{
			if (!OnDrawProperty(pDC, pProp->m_lstSubItems.GetNext(pos)))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void CWTFPropertyGridCtrl::OnPropertyChanged(CWTFPropertyGridProperty* pProp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	pProp->SetModifiedFlag();

	GetOwner()->SendMessage(AFX_WM_PROPERTY_CHANGED, GetDlgCtrlID(), LPARAM(pProp));
}

BOOL CWTFPropertyGridCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CWTFPropertyGridCtrl::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pHeader = (NMHEADER*) pNMHDR;
	ENSURE(pHeader != NULL);

	if (pHeader->iItem == 0)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH;

		GetHeaderCtrl().GetItem(0, &hdItem);

		CRect rectClient;
		GetClientRect(rectClient);

		m_nLeftColumnWidth = std::min(std::max(m_nRowHeight, hdItem.cxy - 2), rectClient.Width() - ::GetSystemMetrics(SM_CXHSCROLL) - 5);

		ReposProperties();

		InvalidateRect(m_rectList);
		UpdateWindow();
	}

	*pResult = 0;
}

void CWTFPropertyGridCtrl::EnableHeaderCtrl(BOOL bEnable, LPCTSTR lpszLeftColumn, LPCTSTR lpszRightColumn)
{
	ASSERT_VALID(this);
	ENSURE(lpszLeftColumn != NULL);
	ENSURE(lpszRightColumn != NULL);

	CWTFPropertyGridProperty* pProp = GetCurSel();
	if (pProp != NULL)
	{
		pProp->OnEndEdit();
	}

	m_bHeaderCtrl = bEnable;

	if (m_bHeaderCtrl)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_TEXT;

		hdItem.pszText = (LPTSTR) lpszLeftColumn;
		hdItem.cchTextMax = static_cast<int>(_tcslen(lpszLeftColumn)) + 1;
		GetHeaderCtrl().SetItem(0, &hdItem);

		hdItem.pszText = (LPTSTR) lpszRightColumn;
		hdItem.cchTextMax = static_cast<int>(_tcslen(lpszRightColumn)) + 1;
		GetHeaderCtrl().SetItem(1, &hdItem);
	}

	AdjustLayout();
	RedrawWindow();
}

void CWTFPropertyGridCtrl::EnableDescriptionArea(BOOL bEnable)
{
	ASSERT_VALID(this);

	m_bDescriptionArea = bEnable;

	AdjustLayout();

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::OnHeaderTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pHeader = (NMHEADER*) pNMHDR;
	ENSURE(pHeader != NULL);

	pHeader->pitem->cxy = std::min(pHeader->pitem->cxy, m_rectList.Width());

	TrackHeader(pHeader->pitem->cxy);
	*pResult = 0;
}

void CWTFPropertyGridCtrl::OnHeaderEndTrack(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	TrackHeader(-1);
	*pResult = 0;
}

void CWTFPropertyGridCtrl::OnSpinDeltaPos(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	PostMessage(AFX_UM_UPDATESPIN);
}

LRESULT CWTFPropertyGridCtrl::OnUpdateSpin(WPARAM, LPARAM)
{
	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit && m_pSel->m_bEnabled)
	{
		m_pSel->OnUpdateValue();
	}

	return 0;
}

void CWTFPropertyGridCtrl::TrackHeader(int nOffset)
{
	CClientDC dc(this);

	if (!m_rectTrackHeader.IsRectEmpty() && !m_bShowDragContext)
	{
		dc.InvertRect(m_rectTrackHeader);
	}

	if (nOffset < 0) // End of track
	{
		m_rectTrackHeader.SetRectEmpty();
	}
	else
	{
		m_rectTrackHeader = m_rectList;
		m_rectTrackHeader.left += nOffset;
		m_rectTrackHeader.right = m_rectTrackHeader.left + 1;

		if (m_bShowDragContext)
		{
			CRect rectClient;
			GetClientRect(rectClient);

			m_nLeftColumnWidth = std::min(std::max(m_nRowHeight, nOffset), rectClient.Width() - ::GetSystemMetrics(SM_CXHSCROLL) - 5);

			HDITEM hdItem;
			hdItem.mask = HDI_WIDTH ;
			hdItem.cxy = m_nLeftColumnWidth + 2;

			GetHeaderCtrl().SetItem(0, &hdItem);

			hdItem.cxy = rectClient.Width() + 10;
			GetHeaderCtrl().SetItem(1, &hdItem);
		}
		else
		{
			dc.InvertRect(m_rectTrackHeader);
		}
	}
}

void CWTFPropertyGridCtrl::TrackDescr(int nOffset)
{
	CClientDC dc(this);

	if (!m_rectTrackDescr.IsRectEmpty())
	{
		dc.InvertRect(m_rectTrackDescr);
	}

	if (nOffset == INT_MIN) // End of track
	{
		m_rectTrackDescr.SetRectEmpty();
	}
	else
	{
		CRect rectClient;
		GetClientRect(rectClient);

		nOffset = std::max((LONG)nOffset, rectClient.top + m_nRowHeight + m_nHeaderHeight);
		nOffset = std::min((LONG)nOffset, rectClient.bottom - m_nRowHeight);

		m_rectTrackDescr = rectClient;
		m_rectTrackDescr.top = nOffset - 1;
		m_rectTrackDescr.bottom = m_rectTrackDescr.top + 2;

		dc.InvertRect(m_rectTrackDescr);
	}
}

void CWTFPropertyGridCtrl::TrackToolTip(CPoint point)
{
	if (m_bTracking || m_bTrackingDescr)
	{
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	CRect rectTT;
	m_IPToolTip.GetWindowRect(&rectTT);

	if (rectTT.PtInRect(ptScreen) && m_IPToolTip.IsWindowVisible())
	{
		return;
	}

	if (!m_IPToolTip.IsWindowVisible())
	{
		rectTT.SetRectEmpty();
	}

	if (::GetCapture() == GetSafeHwnd())
	{
		ReleaseCapture();
	}

	CWTFPropertyGridProperty* pProp = HitTest(point);
	if (pProp == NULL)
	{
		m_IPToolTip.Deactivate();
		return;
	}

	if (abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
	{
		m_IPToolTip.Deactivate();
		return;
	}

	ASSERT_VALID(pProp);

	if (pProp->IsInPlaceEditing())
	{
		return;
	}

	CString strTipText;
	CRect rectToolTip = pProp->m_Rect;

	BOOL bIsValueTT = FALSE;

	if (point.x < m_rectList.left + m_nLeftColumnWidth)
	{
		if (pProp->IsGroup())
		{
			rectToolTip.left += m_nRowHeight;

			if (point.x <= rectToolTip.left)
			{
				m_IPToolTip.Deactivate();
				return;
			}
		}

		if (pProp->m_bNameIsTruncated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetNameTooltip().IsEmpty())
			{
				strTipText = pProp->m_strName;
			}
		}
	}
	else
	{
		if (pProp->m_bValueIsTruncated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetValueTooltip().IsEmpty())
			{
				strTipText = pProp->FormatProperty();
			}
		}

		rectToolTip.left = m_rectList.left + m_nLeftColumnWidth + 1;
		bIsValueTT = TRUE;
	}

	if (!strTipText.IsEmpty())
	{
		ClientToScreen(&rectToolTip);

		if (rectTT.TopLeft() == rectToolTip.TopLeft())
		{
			// Tooltip on the same place, don't show it to prevent flashing
			return;
		}

		m_IPToolTip.SetTextMargin(AFX_TEXT_MARGIN);

		m_IPToolTip.SetFont(bIsValueTT && pProp->IsModified() && m_bMarkModifiedProperties ? &m_fontBold : GetFont());

		m_IPToolTip.Track(rectToolTip, strTipText);
		SetCapture();
	}
	else
	{
		m_IPToolTip.Deactivate();
	}
}

int CWTFPropertyGridCtrl::AddProperty(CWTFPropertyGridProperty* pProp, BOOL bRedraw, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pListProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pListProp);

		if (pListProp == pProp || pListProp->IsSubItem(pProp))
		{
			// Can't ad the same property twice
			ASSERT(FALSE);
			return -1;
		}
	}

	pProp->SetOwnerList(this);

	m_lstProps.AddTail(pProp);
	int nIndex = (int) m_lstProps.GetCount() - 1;

	if (bAdjustLayout)
	{
		AdjustLayout();

		if (bRedraw && GetSafeHwnd() != NULL)
		{
			pProp->Redraw();
		}
	}

	return nIndex;
}

void CWTFPropertyGridCtrl::RemoveAll()
{
	ASSERT_VALID(this);

	while (!m_lstProps.IsEmpty())
	{
		delete m_lstProps.RemoveHead();
	}

	while (m_nTooltipsCount > 0)
	{
		m_ToolTip.DelTool(this, m_nTooltipsCount);
		m_nTooltipsCount--;
	}

	m_lstTerminalProps.RemoveAll();

	m_pSel = NULL;
}

CWTFPropertyGridProperty* CWTFPropertyGridCtrl::GetProperty(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_lstProps.GetCount())
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_lstProps.GetAt(m_lstProps.FindIndex(nIndex));
}

CWTFPropertyGridProperty* CWTFPropertyGridCtrl::FindItemByData(DWORD_PTR dwData, BOOL bSearchSubItems/* = TRUE*/) const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pProp);

		if (pProp->m_dwData == dwData)
		{
			return pProp;
		}

		if (bSearchSubItems)
		{
			pProp = pProp->FindSubItemByData(dwData);

			if (pProp != NULL)
			{
				ASSERT_VALID(pProp);
				return pProp;
			}
		}
	}

	return NULL;
}

CWTFPropertyGridProperty* CWTFPropertyGridCtrl::HitTest(CPoint pt, CWTFPropertyGridProperty::ClickArea* pnArea, BOOL bPropsOnly) const
{
	ASSERT_VALID(this);

	if (!m_rectList.PtInRect(pt) && !bPropsOnly)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectDescr = rectClient;
		rectDescr.top = m_rectList.bottom;

		if (pnArea != NULL && rectDescr.PtInRect(pt))
		{
			*pnArea = CWTFPropertyGridProperty::ClickDescription;
		}

		return NULL;
	}

	const CList<CWTFPropertyGridProperty*, CWTFPropertyGridProperty*>& lst = m_bAlphabeticMode ? m_lstTerminalProps : m_lstProps;

	for (POSITION pos = lst.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = lst.GetNext(pos);
		ASSERT_VALID(pProp);

		CWTFPropertyGridProperty* pHit = pProp->HitTest(pt, pnArea);
		if (pHit != NULL)
		{
			return pHit;
		}
	}

	return NULL;
}

void CWTFPropertyGridCtrl::SetCurSel(CWTFPropertyGridProperty* pProp, BOOL bRedraw)
{
	ASSERT_VALID(this);

	CWTFPropertyGridProperty* pOldSelectedItem = m_pSel;
	if (pOldSelectedItem == pProp)
	{
		NotifyAccessibility(m_pSel);
		return;
	}

	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit)
	{
		EndEditItem();
	}

	m_pSel = pProp;
	OnChangeSelection(m_pSel, pOldSelectedItem);

	if (pOldSelectedItem != NULL)
	{
		pOldSelectedItem->OnKillSelection(pProp);

		CRect rectButton = pOldSelectedItem->m_rectButton;
		pOldSelectedItem->m_rectButton.SetRectEmpty();

		if (bRedraw)
		{
			CRect rectOld = pOldSelectedItem->m_Rect;

			if (!pOldSelectedItem->IsGroup() || !m_bGroupNameFullWidth)
			{
				if (!pOldSelectedItem->IsGroup() && pOldSelectedItem->HasValueField())
				{
					rectOld.right = rectOld.left + m_nLeftColumnWidth;
				}
			}

			rectOld.right = std::max(rectButton.right, rectOld.right);
			InvalidateRect(rectButton);
			InvalidateRect(rectOld);
		}
	}

	if (pProp != NULL)
	{
		pProp->OnSetSelection(pOldSelectedItem);

		if (pProp->HasButton())
		{
			pProp->AdjustButtonRect();
		}

		if (bRedraw)
		{
			CRect rect = pProp->m_Rect;

			if (!pProp->IsGroup() || !m_bGroupNameFullWidth)
			{
				if (!pProp->IsGroup() && pProp->HasValueField())
				{
					rect.right = rect.left + m_nLeftColumnWidth;
				}
			}

			rect.right = std::max(pProp->m_rectButton.right, rect.right);
			InvalidateRect(rect);
			InvalidateRect(pProp->m_rectButton);
		}
	}

	if (bRedraw)
	{
		if (m_bDescriptionArea)
		{
			CRect rectClient;
			GetClientRect(rectClient);

			CRect rectDescr = rectClient;
			rectDescr.top = m_rectList.bottom;
			rectDescr.DeflateRect(AFX_TEXT_MARGIN, AFX_TEXT_MARGIN);
			InvalidateRect(rectDescr);
		}

		UpdateWindow();
	}

	NotifyAccessibility(m_pSel);
}

void CWTFPropertyGridCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);

	SetFocus();

	CRect rectClient;
	GetClientRect(rectClient);

	if (m_bDescriptionArea)
	{
		if (abs(point.y -(m_rectList.bottom + AFX_TEXT_MARGIN)) <= AFX_STRETCH_DELTA)
		{
			SetCapture();
			TrackDescr(point.y);
			m_bTrackingDescr = TRUE;
			return;
		}

		if (point.y > m_rectList.bottom)
		{
			return;
		}
	}

	if (abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
	{
		SetCapture();
		TrackHeader(point.x);
		m_bTracking = TRUE;
		return;
	}

	CWTFPropertyGridProperty::ClickArea clickArea;
	CWTFPropertyGridProperty* pHit = HitTest(point, &clickArea);

	BOOL bSelChanged = pHit != GetCurSel();

	SetCurSel(pHit);
	if (pHit == NULL)
	{
		return;
	}

	if (m_rectList.Height () > pHit->GetRect ().Height ())
	{
		EnsureVisible(pHit);
	}

	switch (clickArea)
	{
	case CWTFPropertyGridProperty::ClickExpandBox:
		pHit->Expand(!pHit->IsExpanded());
		break;

	case CWTFPropertyGridProperty::ClickName:
		pHit->OnClickName(point);
		break;

	case CWTFPropertyGridProperty::ClickValue:
		if (pHit->m_bEnabled)
		{
			if (EditItem(pHit, &point) && pHit->m_pWndInPlace != NULL)
			{
				if (pHit->m_rectButton.PtInRect(point))
				{
					CString strPrevVal = pHit->FormatProperty();

					if (::GetCapture() == GetSafeHwnd())
					{
						ReleaseCapture();
					}

					pHit->OnClickButton(point);

					if (strPrevVal != pHit->FormatProperty())
					{
						OnPropertyChanged(pHit);
					}
				}
				else if (!bSelChanged || pHit->IsProcessFirstClick())
				{
					pHit->OnClickValue(WM_LBUTTONDOWN, point);
				}
			}
		}
		break;

	default:
		break;
	}
}

void CWTFPropertyGridCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonDown(nFlags, point);

	SetFocus();

	CRect rectClient;
	GetClientRect(rectClient);

	if (m_bDescriptionArea)
	{
		if (abs(point.y -(m_rectList.bottom + AFX_TEXT_MARGIN)) <= AFX_STRETCH_DELTA)
		{
			return;
		}

		if (point.y > m_rectList.bottom)
		{
			return;
		}
	}

	if (abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
	{
		return;
	}

	CWTFPropertyGridProperty::ClickArea clickArea;
	CWTFPropertyGridProperty* pHit = HitTest(point, &clickArea);

	BOOL bSelChanged = pHit != GetCurSel();

	SetCurSel(pHit);
	if (pHit == NULL)
	{
		return;
	}

	EnsureVisible(pHit);

	switch (clickArea)
	{
	case CWTFPropertyGridProperty::ClickExpandBox:
		break;

	case CWTFPropertyGridProperty::ClickName:
		pHit->OnRClickName(point);
		break;

	case CWTFPropertyGridProperty::ClickValue:
		pHit->OnRClickValue(point, bSelChanged);

		if (pHit->m_bEnabled && !bSelChanged)
		{
			if (EditItem(pHit, &point) && pHit->m_pWndInPlace != NULL)
			{
				if (pHit->m_rectButton.PtInRect(point))
				{
					return;
				}
				else if (pHit->IsProcessFirstClick())
				{
					pHit->OnClickValue(WM_RBUTTONDOWN, point);
				}
			}
		}
		break;

	default:
		break;
	}
}

BOOL CWTFPropertyGridCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create(GetGlobalData()->RegisterWindowClass(_T("Wtf:PropList")), _T(""), dwStyle, rect, pParentWnd, nID, NULL);
}

BOOL CWTFPropertyGridCtrl::EditItem(CWTFPropertyGridProperty* pProp, LPPOINT lptClick)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	if (!EndEditItem())
	{
		return FALSE;
	}

	if (pProp->IsGroup() && !pProp->m_bIsValueList)
	{
		return FALSE;
	}

	if (pProp->OnEdit(lptClick))
	{
		pProp->Redraw();
		SetCurSel(pProp);
		SetCapture();
	}

	return TRUE;
}

void CWTFPropertyGridCtrl::OnClickButton(CPoint point)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pSel);

	if (m_pSel->OnUpdateValue())
	{
		CString strPrevVal = m_pSel->FormatProperty();

		CWaitCursor wait;
		m_pSel->OnClickButton(point);

		if (strPrevVal != m_pSel->FormatProperty())
		{
			OnPropertyChanged(m_pSel);
		}
	}
}

BOOL CWTFPropertyGridCtrl::EndEditItem(BOOL bUpdateData/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (m_pSel == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID(m_pSel);

	if (!m_pSel->m_bInPlaceEdit)
	{
		return TRUE;
	}

	if (bUpdateData)
	{
		if (!ValidateItemData(m_pSel) || !m_pSel->OnUpdateValue())
		{
			return FALSE;
		}
	}

	if (m_pSel != NULL && !m_pSel->OnEndEdit())
	{
		return FALSE;
	}

	if (::GetCapture() == GetSafeHwnd())
	{
		ReleaseCapture();
	}

	if (m_pSel != NULL)
	{
		m_pSel->Redraw();
	}

	return TRUE;
}

BOOL CWTFPropertyGridCtrl::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		m_ToolTip.RelayEvent(pMsg);
		m_IPToolTip.Hide();
		break;

	case WM_MOUSEMOVE:
		m_ToolTip.RelayEvent(pMsg);

		if (pMsg->wParam == 0) // No buttons pressed
		{
			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);

			TrackToolTip(ptCursor);
		}
		break;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && m_pSel != NULL && m_pSel->OnActivateByTab())
	{
		return TRUE;
	}

	if (pMsg->message == WM_SYSKEYDOWN && (pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RIGHT) && m_pSel != NULL
		&& m_pSel->m_bEnabled && ((m_pSel->m_dwFlags) & AFX_PROP_HAS_BUTTON) && EditItem(m_pSel))
	{
		CString strPrevVal = m_pSel->FormatProperty();

		CWaitCursor wait;
		m_pSel->OnClickButton(CPoint(-1, -1));

		if (strPrevVal != m_pSel->FormatProperty())
		{
			OnPropertyChanged(m_pSel);
		}

		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && m_pSel != NULL && m_pSel->IsGroup() && !m_pSel->IsInPlaceEditing())
	{
		m_pSel->Expand(!m_pSel->IsExpanded());
		return TRUE;
	}

	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit && m_pSel->m_bEnabled)
	{
		ASSERT_VALID(m_pSel);

		const BOOL bIsDroppedDown = m_pSel->m_pWndCombo != NULL && m_pSel->m_pWndCombo->GetDroppedState();

		if (pMsg->message == WM_KEYDOWN && (!bIsDroppedDown || pMsg->wParam == VK_RETURN))
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				if (m_pSel->m_bButtonIsFocused)
				{
					CString strPrevVal = m_pSel->FormatProperty();

					CWaitCursor wait;
					m_pSel->OnClickButton(CPoint(-1, -1));

					if (strPrevVal != m_pSel->FormatProperty())
					{
						OnPropertyChanged(m_pSel);
					}
					return TRUE;
				}

				if (bIsDroppedDown)
				{
					HWND hwndInplace = m_pSel->m_pWndInPlace->GetSafeHwnd();
					m_pSel->OnSelectCombo();

					if (::IsWindow(hwndInplace))
					{
						m_pSel->m_pWndInPlace->SetFocus();
					}
				}

				if (!EndEditItem())
				{
					MessageBeep((UINT)-1);
				}

				SetFocus();
				break;

			case VK_ESCAPE:
				EndEditItem(FALSE);
				SetFocus();
				break;

			case VK_DOWN:
			case VK_UP:
				if (m_pSel->m_lstOptions.GetCount() > 1)
				{
					if (m_pSel->OnRotateListValue(pMsg->wParam != VK_UP))
					{
						return TRUE;
					}
				}
				else if (::IsWindow(m_pSel->m_pWndInPlace->GetSafeHwnd()))
				{
					m_pSel->m_pWndInPlace->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
					return TRUE;
				}
				break;

			default:
				if (!m_pSel->m_bAllowEdit)
				{
					m_pSel->PushChar((UINT) pMsg->wParam);
					return TRUE;
				}

				if (ProcessClipboardAccelerators((UINT) pMsg->wParam))
				{
					return TRUE;
				}

				return FALSE;
			}

			return TRUE;
		}
		else if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
		{
			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);

			if (m_pSel->m_pWndSpin != NULL)
			{
				ASSERT_VALID(m_pSel->m_pWndSpin);
				ASSERT(m_pSel->m_pWndSpin->GetSafeHwnd() != NULL);

				CRect rectSpin;
				m_pSel->m_pWndSpin->GetClientRect(rectSpin);
				m_pSel->m_pWndSpin->MapWindowPoints(this, rectSpin);

				if (rectSpin.PtInRect(ptCursor))
				{
					MapWindowPoints(m_pSel->m_pWndSpin, &ptCursor, 1);

					m_pSel->m_pWndSpin->SendMessage(pMsg->message, pMsg->wParam, MAKELPARAM(ptCursor.x, ptCursor.y));
					return TRUE;
				}
			}

			CWnd* pWndInPlaceEdit = m_pSel->m_pWndInPlace;
			ASSERT_VALID(pWndInPlaceEdit);

			if (!m_pSel->m_bAllowEdit)
			{
				m_pSel->m_pWndInPlace->HideCaret();
			}

			CRect rectEdit;
			pWndInPlaceEdit->GetClientRect(rectEdit);
			pWndInPlaceEdit->MapWindowPoints(this, rectEdit);

			if (rectEdit.PtInRect(ptCursor) && pMsg->message == WM_LBUTTONDBLCLK)
			{
				if (m_pSel->OnDblClk(ptCursor))
				{
					return TRUE;
				}
			}

			if (rectEdit.PtInRect(ptCursor) && pMsg->message == WM_RBUTTONDOWN && !m_pSel->m_bAllowEdit)
			{
				return TRUE;
			}

			if (!rectEdit.PtInRect(ptCursor) &&
				(pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_NCLBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_MBUTTONDOWN))
			{
				if (m_pSel->m_rectButton.PtInRect(ptCursor))
				{
					CString strPrevVal = m_pSel->FormatProperty();

					CWaitCursor wait;
					OnClickButton(ptCursor);

					if (strPrevVal != m_pSel->FormatProperty())
					{
						OnPropertyChanged(m_pSel);
					}
					return TRUE;
				}

				if (!EndEditItem())
				{
					return TRUE;
				}
			}
			else
			{
				MapWindowPoints(pWndInPlaceEdit, &ptCursor, 1);
				pWndInPlaceEdit->SendMessage(pMsg->message, pMsg->wParam, MAKELPARAM(ptCursor.x, ptCursor.y));
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CWTFPropertyGridCtrl::OnCancelMode()
{
	if (m_bTracking)
	{
		TrackHeader(-1);
		m_bTracking = FALSE;

		if (::GetCapture() == GetSafeHwnd())
		{
			ReleaseCapture();
		}
	}

	if (m_bTrackingDescr)
	{
		TrackDescr(INT_MIN);
		m_bTrackingDescr = FALSE;

		if (::GetCapture() == GetSafeHwnd())
		{
			ReleaseCapture();
		}
	}

	// Tooltip:
	if (::GetCapture() == GetSafeHwnd())
	{
		ReleaseCapture();
	}

	m_IPToolTip.Deactivate();

	EndEditItem();

	CWnd::OnCancelMode();
}

void CWTFPropertyGridCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	m_bFocused = TRUE;

	if (m_pSel != NULL)
	{
		RedrawWindow(m_pSel->m_Rect);
	}
}

void CWTFPropertyGridCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (!IsChild(pNewWnd))
	{
		if (m_pSel == NULL || m_pSel->OnKillFocus(pNewWnd))
		{
			EndEditItem();
			m_bFocused = FALSE;

			if (m_pSel != NULL)
			{
				m_pSel->Redraw();
			}
		}
	}

	CWnd::OnKillFocus(pNewWnd);
}

void CWTFPropertyGridCtrl::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CWnd::OnStyleChanged(nStyleType, lpStyleStruct);
	AdjustLayout();
}

UINT CWTFPropertyGridCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

void CWTFPropertyGridCtrl::SetScrollSizes()
{
	ASSERT_VALID(this);

	if (m_wndScrollVert.GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
		m_nVertScrollOffset = 0;
	}
	else
	{
		m_nVertScrollPage = m_rectList.Height() / m_nRowHeight - 1;
		m_nVertScrollTotal = GetTotalItems(FALSE /* Visible only */);

		if (m_nVertScrollTotal <= m_nVertScrollPage)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
		}

		m_nVertScrollOffset = std::min(m_nVertScrollOffset, m_nVertScrollTotal);
	}

	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);

	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_nVertScrollTotal;
	si.nPage = m_nVertScrollPage;
	si.nPos = m_nVertScrollOffset;

	SetScrollInfo(SB_VERT, &si, TRUE);
	m_wndScrollVert.EnableScrollBar(m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	m_wndScrollVert.EnableWindow();
}

int CWTFPropertyGridCtrl::GetTotalItems(BOOL bIncludeHidden) const
{
	ASSERT_VALID(this);

	int nCount = 0;

	if (m_bAlphabeticMode)
	{
		if (bIncludeHidden)
		{
			return(int) m_lstTerminalProps.GetCount();
		}

		for (POSITION pos = m_lstTerminalProps.GetHeadPosition(); pos != NULL;)
		{
			CWTFPropertyGridProperty* pProp = m_lstTerminalProps.GetNext(pos);
			ASSERT_VALID(pProp);

			if (pProp->IsVisible())
			{
				nCount++;

				if (pProp->IsExpanded())
				{
					nCount += pProp->GetExpandedSubItems();
				}
			}
		}

		return nCount;
	}

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pProp);

		nCount += pProp->GetExpandedSubItems(bIncludeHidden) + 1;
	}

	return nCount;
}

void CWTFPropertyGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_pSel != NULL && pScrollBar->GetSafeHwnd() != NULL && m_pSel->m_pWndSpin->GetSafeHwnd() == pScrollBar->GetSafeHwnd())
	{
		return;
	}

	m_IPToolTip.Hide();
	EndEditItem();

	int nPrevOffset = m_nVertScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset--;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset++;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= m_nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += m_nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = std::min(std::max(0, m_nVertScrollOffset), m_nVertScrollTotal - m_nVertScrollPage + 1);

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	SetScrollPos(SB_VERT, m_nVertScrollOffset);

	ReposProperties();

	int dy = m_nRowHeight *(nPrevOffset - m_nVertScrollOffset);
	ScrollWindow(0, dy, m_rectList, m_rectList);

	if (m_pSel != NULL)
	{
		ASSERT_VALID(m_pSel);
		RedrawWindow(m_pSel->m_rectButton);
	}
}

CScrollBar* CWTFPropertyGridCtrl::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	return(CScrollBar* ) &m_wndScrollVert;
}

BOOL CWTFPropertyGridCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return TRUE;
	}

	if (m_nVertScrollTotal > 0)
	{
		const int nSteps = abs(zDelta) / WHEEL_DELTA;

		for (int i = 0; i < nSteps; i++)
		{
			OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
		}
	}

	return TRUE;
}

void CWTFPropertyGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDblClk(nFlags, point);

	if (point.y <= m_rectList.bottom && abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
	{
		// Double click on vertical splitter, make splitter 50/50:
		CRect rectClient;
		GetClientRect(rectClient);

		m_nLeftColumnWidth = rectClient.Width() / 2;

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl().SetItem(0, &hdItem);

		hdItem.cxy = rectClient.Width() + 10;
		GetHeaderCtrl().SetItem(1, &hdItem);
		return;
	}

	if (m_pSel == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pSel);

	if (m_pSel->IsGroup() &&
		(!m_pSel->m_bIsValueList || point.x < m_rectList.left + m_nLeftColumnWidth))
	{
		m_pSel->Expand(!m_pSel->IsExpanded());
	}
	else if (m_pSel->m_bEnabled)
	{
		if (EditItem(m_pSel) && m_pSel->m_pWndInPlace != NULL)
		{
			m_pSel->m_pWndInPlace->SendMessage(WM_LBUTTONDOWN);
			m_pSel->m_pWndInPlace->SendMessage(WM_LBUTTONUP);
		}

		if (m_pSel->GetRect().PtInRect(point))
		{
			m_pSel->OnDblClk(point);
		}
	}
}

BOOL CWTFPropertyGridCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		CPoint point;

		::GetCursorPos(&point);
		ScreenToClient(&point);

		if (m_bDescriptionArea && abs(point.y -(m_rectList.bottom + AFX_TEXT_MARGIN)) <= AFX_STRETCH_DELTA)
		{
			::SetCursor(GetGlobalData()->m_hcurStretchVert);
			return TRUE;
		}

		if (point.y <= m_rectList.bottom)
		{
			if (abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
			{
				::SetCursor(GetGlobalData()->m_hcurStretch);
				return TRUE;
			}

			CWTFPropertyGridProperty::ClickArea clickArea;
			CWTFPropertyGridProperty* pHit = HitTest(point, &clickArea);

			if (pHit != NULL && pHit == m_pSel && clickArea == CWTFPropertyGridProperty::ClickValue && !pHit->m_rectButton.PtInRect(point) && pHit->OnSetCursor())
			{
				return TRUE;
			}
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CWTFPropertyGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_lstProps.IsEmpty())
	{
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	switch (nChar)
	{
	case VK_F4:
		if (m_pSel != NULL && m_pSel->m_bEnabled && EditItem(m_pSel))
		{
			if ((m_pSel->m_dwFlags) & AFX_PROP_HAS_BUTTON)
			{
				CString strPrevVal = m_pSel->FormatProperty();

				CWaitCursor wait;
				m_pSel->OnClickButton(CPoint(-1, -1));

				if (strPrevVal != m_pSel->FormatProperty())
				{
					OnPropertyChanged(m_pSel);
				}
			}
			return;
		}
		break;

	case VK_LEFT:
		if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			BOOL bShowDragContext = m_bShowDragContext;
			m_bShowDragContext = TRUE;

			TrackHeader(m_nLeftColumnWidth - 5);

			m_bShowDragContext = bShowDragContext;
			return;
		}
		else if (m_pSel != NULL && m_pSel->IsGroup() && m_pSel->IsExpanded())
		{
			m_pSel->Expand(FALSE);
			return;
		}

		// else ==> act as VK_UP!

	case VK_UP:
		{
			if (m_pSel == NULL)
			{
				SetCurSel(m_bAlphabeticMode ? m_lstTerminalProps.GetHead() : m_lstProps.GetHead());
				OnVScroll(SB_TOP, 0, NULL);
				return;
			}

			// Select prev. item:
			CPoint point(m_pSel->m_Rect.right - 1, m_pSel->m_Rect.top - 2);

			CWTFPropertyGridProperty* pHit = HitTest(point, NULL, TRUE);
			if (pHit != NULL)
			{
				SetCurSel(pHit);
				EnsureVisible(pHit);
			}
		}
		return;

	case VK_RIGHT:
		if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			BOOL bShowDragContext = m_bShowDragContext;
			m_bShowDragContext = TRUE;

			TrackHeader(m_nLeftColumnWidth + 5);

			m_bShowDragContext = bShowDragContext;
			return;
		}
		else if (m_pSel != NULL && m_pSel->IsGroup() && !m_pSel->IsExpanded())
		{
			m_pSel->Expand();
			return;
		}

		// else ==> act as VK_DOWN!

	case VK_DOWN:
		{
			if (m_pSel == NULL)
			{
				SetCurSel(m_bAlphabeticMode ? m_lstTerminalProps.GetHead() : m_lstProps.GetHead());
				OnVScroll(SB_TOP, 0, NULL);
				return;
			}

			if ((::GetAsyncKeyState(VK_MENU) & 0x8000) && nChar == VK_DOWN)
			{
				CString strPrevVal = m_pSel->FormatProperty();

				CWaitCursor wait;
				m_pSel->OnClickButton(CPoint(-1, -1));

				if (strPrevVal != m_pSel->FormatProperty())
				{
					OnPropertyChanged(m_pSel);
				}

				return;
			}

			// Select next item:
			CPoint point(m_pSel->m_Rect.right - 1, m_pSel->m_Rect.bottom + 2);

			CWTFPropertyGridProperty* pHit = HitTest(point, NULL, TRUE);
			if (pHit != NULL)
			{
				SetCurSel(pHit);
				EnsureVisible(pHit);
			}
		}
		return;

	case VK_NEXT:
		{
			if (m_pSel == NULL)
			{
				SetCurSel(m_bAlphabeticMode ? m_lstTerminalProps.GetHead() : m_lstProps.GetHead());
				OnVScroll(SB_TOP, 0, NULL);
				return;
			}

			if (m_nVertScrollPage != 0)
			{
				EnsureVisible(m_pSel);

				CPoint point(m_pSel->m_Rect.right - 1, m_pSel->m_Rect.top + m_nVertScrollPage * m_nRowHeight);

				CWTFPropertyGridProperty* pHit = HitTest(point, NULL, TRUE);
				if (pHit != NULL)
				{
					SetCurSel(pHit);
					OnVScroll(SB_PAGEDOWN, 0, NULL);
					return;
				}
			}
		}

	case VK_END:
		{
			CWTFPropertyGridProperty* pLastProp = NULL;

			if (m_bAlphabeticMode)
			{
				pLastProp = m_lstTerminalProps.GetTail();
				ASSERT_VALID(pLastProp);
			}
			else
			{
				pLastProp = m_lstProps.GetTail();
				ASSERT_VALID(pLastProp);
			}

			while (!pLastProp->m_lstSubItems.IsEmpty() && pLastProp->IsExpanded())
			{
				pLastProp = pLastProp->m_lstSubItems.GetTail();
			}

			SetCurSel(pLastProp);
			OnVScroll(SB_BOTTOM, 0, NULL);
		}
		return;

	case VK_PRIOR:
		{
			if (m_pSel != NULL && m_nVertScrollPage != 0)
			{
				EnsureVisible(m_pSel);

				CPoint point(m_pSel->m_Rect.right - 1, m_pSel->m_Rect.top - m_nVertScrollPage * m_nRowHeight);

				CWTFPropertyGridProperty* pHit = HitTest(point, NULL, TRUE);
				if (pHit != NULL)
				{
					SetCurSel(pHit);
					OnVScroll(SB_PAGEUP, 0, NULL);
					return;
				}
			}
		}

	case VK_HOME:
		SetCurSel(m_bAlphabeticMode ? m_lstTerminalProps.GetHead() : m_lstProps.GetHead());
		OnVScroll(SB_TOP, 0, NULL);
		return;

	case VK_ADD:
		if (m_pSel != NULL && m_pSel->IsGroup() && !m_pSel->IsExpanded() && !m_pSel->IsInPlaceEditing())
		{
			m_pSel->Expand();
		}
		return;

	case VK_SUBTRACT:
		if (m_pSel != NULL && m_pSel->IsGroup() && m_pSel->IsExpanded() && !m_pSel->IsInPlaceEditing())
		{
			m_pSel->Expand(FALSE);
		}
		return;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CWTFPropertyGridCtrl::EnsureVisible(CWTFPropertyGridProperty* pProp, BOOL bExpandParents/* = FALSE*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	if (m_nRowHeight == 0)
	{
		return;
	}

	if (bExpandParents && pProp->GetParent() != NULL)
	{
		CWTFPropertyGridProperty* pParent = pProp;

		while ((pParent = pParent->GetParent()) != NULL)
		{
			ASSERT_VALID(pParent);
			pParent->Expand(TRUE);
		}
	}

	CRect rect = pProp->m_Rect;

	if (rect.top >= m_rectList.top - 1 && rect.bottom <= m_rectList.bottom)
	{
		return;
	}

	CRect rectButton = pProp->m_rectButton;
	pProp->m_rectButton.SetRectEmpty();
	RedrawWindow(rectButton);

	if (rect.top < m_rectList.top - 1 && rect.bottom >= m_rectList.top - 1)
	{
		OnVScroll(SB_LINEUP, 0, NULL);
	}
	else if (rect.bottom > m_rectList.bottom && rect.top <= m_rectList.bottom)
	{
		OnVScroll(SB_LINEDOWN, 0, NULL);
	}
	else
	{
		OnVScroll(SB_THUMBPOSITION, rect.top / m_nRowHeight - m_nVertScrollOffset, NULL);
	}

	if (!rectButton.IsRectEmpty())
	{
		pProp->AdjustButtonRect();
		RedrawWindow(pProp->m_rectButton);
	}
}

void CWTFPropertyGridCtrl::ExpandAll(BOOL bExpand/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (m_bAlphabeticMode)
	{
		return;
	}

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->ExpandDeep(bExpand);
	}

	AdjustLayout();

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnChar(nChar, nRepCnt, nFlags);

	if (m_pSel == NULL || !m_pSel->m_bEnabled)
	{
		return;
	}

	ASSERT_VALID(m_pSel);

	if (!EditItem(m_pSel))
	{
		return;
	}

	m_pSel->PushChar(nChar);
}

HBRUSH CWTFPropertyGridCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (m_pSel != NULL && pWnd->GetSafeHwnd() == m_pSel->m_pWndInPlace->GetSafeHwnd())
	{
		HBRUSH hbrProp = m_pSel->OnCtlColor(pDC, nCtlColor);
		if (hbrProp != NULL)
		{
			return hbrProp;
		}
	}

	return hbr;
}

void CWTFPropertyGridCtrl::UpdateColor(COLORREF color)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pSel);

	CWTFPropertyGridColorProperty* pColorProp = DYNAMIC_DOWNCAST(CWTFPropertyGridColorProperty, m_pSel);
	if (pColorProp)
	{
		BOOL bChanged = color != pColorProp->GetColor();
		pColorProp->SetColor(color);

		if (bChanged)
		{
			OnPropertyChanged(pColorProp);
		}

		if (color == (COLORREF)-1 && pColorProp->m_pWndInPlace != NULL && ::IsWindow(pColorProp->m_pWndInPlace->GetSafeHwnd()))
		{
			pColorProp->m_pWndInPlace->SetWindowText(_T(""));
		}

		pColorProp->OnUpdateValue();
	}
}

void CWTFPropertyGridCtrl::CloseColorPopup()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pSel);

	CWTFPropertyGridColorProperty* pColorProp = DYNAMIC_DOWNCAST(CWTFPropertyGridColorProperty, m_pSel);
	if (pColorProp)
	{
		pColorProp->m_pPopup = NULL;

		pColorProp->m_bButtonIsDown = FALSE;
		pColorProp->Redraw();

		if (pColorProp->m_pWndInPlace != NULL)
		{
			pColorProp->m_pWndInPlace->SetFocus();
		}
	}
}

void CWTFPropertyGridCtrl::OnSelectCombo()
{
	ASSERT_VALID(this);

	if (m_pSel == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pSel);
	m_pSel->OnSelectCombo();
}

void CWTFPropertyGridCtrl::OnCloseCombo()
{
	ASSERT_VALID(this);

	if (m_pSel == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pSel);
	m_pSel->OnCloseCombo();
}

void CWTFPropertyGridCtrl::OnEditKillFocus()
{
	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit != NULL && m_pSel->m_bEnabled)
	{
		ASSERT_VALID(m_pSel);

		if (!IsChild(GetFocus()) && m_pSel->OnEditKillFocus())
		{
			if (!EndEditItem())
			{
				m_pSel->m_pWndInPlace->SetFocus();
			}
			else
			{
				OnKillFocus(GetFocus());
			}
		}
	}
}

void CWTFPropertyGridCtrl::OnComboKillFocus()
{
	if (m_pSel != NULL && m_pSel->m_pWndCombo != NULL && m_pSel->m_bEnabled)
	{
		ASSERT_VALID(m_pSel);

		if (!IsChild(GetFocus()))
		{
			if (!EndEditItem())
			{
				m_pSel->m_pWndCombo->SetFocus();
			}
			else
			{
				OnKillFocus(GetFocus());
			}
		}
	}
}

void CWTFPropertyGridCtrl::SetBoolLabels(LPCTSTR lpszTrue, LPCTSTR lpszFalse)
{
	ASSERT_VALID(this);
	ASSERT(lpszTrue != NULL);
	ASSERT(lpszFalse != NULL);

	m_strTrue = lpszTrue;
	m_strFalse = lpszFalse;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::SetListDelimiter(TCHAR c)
{
	ASSERT_VALID(this);
	ASSERT(GetSafeHwnd() == NULL); // Should be called before window create

	m_cListDelimeter = c;
}

void CWTFPropertyGridCtrl::SetDescriptionRows(int nDescRows)
{
	ASSERT_VALID(this);

	m_nDescrRows = nDescRows;

	if (GetSafeHwnd() != NULL)
	{
		AdjustLayout();
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::SetAlphabeticMode(BOOL bSet)
{
	ASSERT_VALID(this);

	if (m_bAlphabeticMode == bSet)
	{
		return;
	}

	m_bAlphabeticMode = bSet;
	m_nVertScrollOffset = 0;

	SetCurSel(NULL);

	if (GetSafeHwnd() != NULL)
	{
		if (m_bAlphabeticMode)
		{
			ReposProperties();
		}

		AdjustLayout();
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::SetVSDotNetLook(BOOL bSet)
{
	ASSERT_VALID(this);
	m_bVSDotNetLook = bSet;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

BOOL CWTFPropertyGridCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CWnd::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ENSURE(pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		m_ToolTip.SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

void CWTFPropertyGridCtrl::OnDestroy()
{
	while (!m_lstProps.IsEmpty())
	{
		delete m_lstProps.RemoveHead();
	}

	m_pSel = NULL;

	m_IPToolTip.DestroyWindow();
	m_ToolTip.DestroyWindow();

	CWnd::OnDestroy();
}

BOOL CWTFPropertyGridCtrl::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_ToolTip.GetSafeHwnd() == NULL || pNMH->hwndFrom != m_ToolTip.GetSafeHwnd())
	{
		return FALSE;
	}

	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	CWTFPropertyGridProperty* pProp = HitTest(point);
	if (pProp == NULL)
	{
		return FALSE;
	}

	if (point.x < m_rectList.left + m_nLeftColumnWidth)
	{
		if (!pProp->m_bNameIsTruncated || m_bAlwaysShowUserTT)
		{
			// User-defined tooltip
			strTipText = pProp->GetNameTooltip();
		}
	}
	else
	{
		if (!pProp->m_bValueIsTruncated || m_bAlwaysShowUserTT)
		{
			// User-defined tooltip
			strTipText = pProp->GetValueTooltip();
		}
	}

	if (strTipText.IsEmpty())
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);

	m_ToolTip.SetFont(GetFont(), FALSE);
	return TRUE;
}

void CWTFPropertyGridCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	if (m_bTracking)
	{
		TrackHeader(point.x);
		return;
	}

	if (m_bTrackingDescr)
	{
		TrackDescr(point.y);
		return;
	}
}

void CWTFPropertyGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);

	if (m_bTracking)
	{
		TrackHeader(-1);
		m_bTracking = FALSE;

		if (::GetCapture() == GetSafeHwnd())
		{
			ReleaseCapture();
		}

		CRect rectClient;
		GetClientRect(rectClient);

		m_nLeftColumnWidth = std::min(std::max(m_nRowHeight, (int)point.x), rectClient.Width() - ::GetSystemMetrics(SM_CXHSCROLL) - 5);

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl().SetItem(0, &hdItem);

		hdItem.cxy = rectClient.Width() + 10;
		GetHeaderCtrl().SetItem(1, &hdItem);
	}

	if (m_bTrackingDescr)
	{
		TrackDescr(INT_MIN);
		m_bTrackingDescr = FALSE;

		if (::GetCapture() == GetSafeHwnd())
		{
			ReleaseCapture();
		}

		CRect rectClient;
		GetClientRect(rectClient);

		point.y = std::max((int)point.y, m_nRowHeight + m_nHeaderHeight);
		m_nDescrHeight = rectClient.Height() - point.y + 2;
		m_nDescrHeight = std::max(m_nRowHeight, m_nDescrHeight);

		AdjustLayout();
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (GetStyle() & WS_BORDER)
	{
		lpncsp->rgrc[0].left++;
		lpncsp->rgrc[0].top++ ;
		lpncsp->rgrc[0].right--;
		lpncsp->rgrc[0].bottom--;
	}
}

void CWTFPropertyGridCtrl::OnNcPaint()
{
	if (m_nDescrHeight == -1)
	{
		m_nDescrHeight = m_nRowHeight * m_nDescrRows + m_nRowHeight / 2;
		AdjustLayout();
	}

	if (GetStyle() & WS_BORDER)
	{
		visualManager->OnDrawControlBorder(this);
	}
}

void CWTFPropertyGridCtrl::SetGroupNameFullWidth(BOOL bGroupNameFullWidth, BOOL bRedraw/* = TRUE*/)
{
	m_bGroupNameFullWidth = bGroupNameFullWidth;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::SetCustomColors(COLORREF clrBackground, COLORREF clrText, COLORREF clrGroupBackground,
	COLORREF clrGroupText, COLORREF clrDescriptionBackground, COLORREF clrDescriptionText, COLORREF clrLine)
{
	m_clrBackground = clrBackground;
	m_clrText = clrText;
	m_clrGroupBackground = clrGroupBackground;
	m_clrGroupText = clrGroupText;
	m_clrDescriptionBackground = clrDescriptionBackground;
	m_clrDescriptionText = clrDescriptionText;
	m_clrLine = clrLine;

	m_brBackground.DeleteObject();

	if (m_clrBackground != (COLORREF)-1)
	{
		m_brBackground.CreateSolidBrush(m_clrBackground);
	}
}

void CWTFPropertyGridCtrl::GetCustomColors(COLORREF& clrBackground, COLORREF& clrText, COLORREF& clrGroupBackground,
	COLORREF& clrGroupText, COLORREF& clrDescriptionBackground, COLORREF& clrDescriptionText, COLORREF& clrLine)
{
	clrBackground = m_clrBackground;
	clrText = m_clrText;
	clrGroupBackground = m_clrGroupBackground;
	clrGroupText = m_clrGroupText;
	clrDescriptionBackground = m_clrDescriptionBackground;
	clrDescriptionText = m_clrDescriptionText;
	clrLine = m_clrLine;
}

BOOL CWTFPropertyGridCtrl::ProcessClipboardAccelerators(UINT nChar)
{
	if (m_pSel == NULL || m_pSel->m_pWndInPlace->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pSel);

	BOOL bIsCtrl = (::GetAsyncKeyState(VK_CONTROL) & 0x8000);
	BOOL bIsShift = (::GetAsyncKeyState(VK_SHIFT) & 0x8000);

	if (bIsCtrl &&(nChar == _T('C') || nChar == VK_INSERT))
	{
		m_pSel->m_pWndInPlace->SendMessage(WM_COPY);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('V') ||(bIsShift && nChar == VK_INSERT))
	{
		m_pSel->m_pWndInPlace->SendMessage(WM_PASTE);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('X') ||(bIsShift && nChar == VK_DELETE))
	{
		m_pSel->m_pWndInPlace->SendMessage(WM_CUT);
		return TRUE;
	}

	return FALSE;
}

int CWTFPropertyGridCtrl::CompareProps(const CWTFPropertyGridProperty* pProp1, const CWTFPropertyGridProperty* pProp2) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp1);
	ASSERT_VALID(pProp2);

	return pProp1->m_strName.Compare(pProp2->m_strName);
}

LRESULT CWTFPropertyGridCtrl::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	if (GetGlobalData()->IsAccessibilitySupport())
	{
		return CWnd::OnGetObject(wParam, lParam);
	}

	return(LRESULT)0L;
}

void CWTFPropertyGridCtrl::NotifyAccessibility(CWTFPropertyGridProperty* pProp)
{
	if (!GetGlobalData()->IsAccessibilitySupport() || pProp == NULL)
	{
		return;
	}

	m_pAccProp = pProp;

	CPoint pt(pProp->m_Rect.left, pProp->m_Rect.top);
	ClientToScreen(&pt);
	LPARAM lParam = MAKELPARAM(pt.x, pt.y);

	::NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
}

void CWTFPropertyGridCtrl::MarkModifiedProperties(BOOL bMark/* = TRUE*/, BOOL bRedraw/* = TRUE*/)
{
	m_bMarkModifiedProperties = bMark;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CWTFPropertyGridCtrl::ResetOriginalValues(BOOL bRedraw)
{
	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		CWTFPropertyGridProperty* pProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pProp);

		pProp->ResetOriginalValue();
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

HRESULT CWTFPropertyGridCtrl::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = 0;
	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accChild(VARIANT /*varChild*/, IDispatch **ppdispChild)
{
	if (!(*ppdispChild))
	{
		return E_INVALIDARG;
	}

	if (m_pStdObject != NULL)
	{
		*ppdispChild = m_pStdObject;
	}
	else
	{
		*ppdispChild = NULL;
	}
	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accName(VARIANT varChild, BSTR *pszName)
{
	if (pszName == NULL)
	{
		return E_INVALIDARG;
	}

	if (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;
		GetWindowText(strText);
		if (strText.GetLength() == 0)
		{
			*pszName  = SysAllocString(L"PropertyList");
			return S_OK;
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if (m_pAccProp != NULL)
	{
		CString strName = m_pAccProp->IsInPlaceEditing() ? m_pAccProp->FormatProperty() : m_pAccProp->GetName();
		*pszName = strName.AllocSysString();
		return S_OK;
	}

	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (m_pAccProp != NULL)
	{
		BOOL bGroup = (m_pAccProp->IsGroup() && !m_pAccProp->m_bIsValueList);
		if (!bGroup)
		{
			CString strValue = m_pAccProp->FormatProperty();
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}

HRESULT CWTFPropertyGridCtrl::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if ((((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF)) || (NULL == pszDescription))
	{
		return E_INVALIDARG;
	}

	if (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF))
	{
		*pszDescription = SysAllocString(L"PropertyList");
		return S_OK;
	}

	if (m_pAccProp != NULL)
	{
		CString strName = m_pAccProp->GetName();
		*pszDescription = strName.AllocSysString();
	}

	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = varChild.vt;
		pvarRole->lVal = ROLE_SYSTEM_LIST;
		return S_OK;
	}

	pvarRole->vt = VT_INT;
	pvarRole->lVal = ROLE_SYSTEM_ROW;

	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (((varChild.vt == VT_I4) || (varChild.vt == VT_I8) || (varChild.vt == VT_INT)) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = varChild.vt;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	pvarState->vt = VT_INT;
	pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

	if (m_pAccProp != NULL)
	{
		if (m_pAccProp->IsSelected())
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}

		BOOL bGroup = (m_pAccProp->IsGroup() && !m_pAccProp->m_bIsValueList);
		if (!m_pAccProp->IsEnabled() || bGroup)
		{
			pvarState->lVal |= STATE_SYSTEM_READONLY;
		}
	}

	return S_OK;
}

HRESULT CWTFPropertyGridCtrl::get_accHelp(VARIANT /*varChild*/, BSTR * /*pszHelp*/)
{
	return S_FALSE;
}

HRESULT CWTFPropertyGridCtrl::get_accHelpTopic(BSTR * /*pszHelpFile*/, VARIANT /*varChild*/, long * /*pidTopic*/)
{
	return S_FALSE;
}

HRESULT CWTFPropertyGridCtrl::get_accKeyboardShortcut(VARIANT /*varChild*/, BSTR* /*pszKeyboardShortcut*/)
{
	return S_FALSE;
}

HRESULT CWTFPropertyGridCtrl::get_accFocus(VARIANT *pvarChild)
{
	if (NULL == pvarChild)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND;
}

HRESULT CWTFPropertyGridCtrl::get_accSelection(VARIANT *pvarChildren)
{
	if (NULL == pvarChildren)
	{
		return E_INVALIDARG;
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT CWTFPropertyGridCtrl::get_accDefaultAction(VARIANT /*varChild*/, BSTR* /*pszDefaultAction*/)
{
	return DISP_E_MEMBERNOTFOUND; 
}

HRESULT CWTFPropertyGridCtrl::accSelect(long flagsSelect, VARIANT varChild)
{
	if (m_pStdObject != NULL)
	{
		return m_pStdObject->accSelect(flagsSelect, varChild);
	}
	return E_INVALIDARG;
}

HRESULT CWTFPropertyGridCtrl::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	HRESULT hr = S_OK;

	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}
	else
	{
		if (m_pAccProp != NULL)
		{
			CRect rcProp = m_pAccProp->m_Rect;
			ClientToScreen(&rcProp);
			*pxLeft = rcProp.left;
			*pyTop = rcProp.top;
			*pcxWidth = rcProp.Width();
			*pcyHeight = rcProp.Height();
		}
	}

	return hr;
}

HRESULT CWTFPropertyGridCtrl::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	CPoint pt(xLeft, yTop);
	ScreenToClient(&pt);

	CWTFPropertyGridProperty* pProp = HitTest(pt);
	if (pProp != NULL)
	{
		LPARAM lParam = MAKELPARAM((WORD)xLeft, (WORD)yTop);
		pvarChild->vt = VT_INT;
		pvarChild->lVal = (LONG)lParam;
	}
	else
	{
		pvarChild->vt = VT_INT;
		pvarChild->lVal = CHILDID_SELF;
	}

	m_pAccProp = pProp;
	return S_OK;
}

LRESULT CWTFPropertyGridCtrl::OnInitControl(WPARAM wParam, LPARAM lParam)
{
#if 1
	DWORD dwSize = (DWORD)wParam;
	BYTE* pbInitData = (BYTE*)lParam;

	CString strDst;
	CMFCControlContainer::UTF8ToString((LPSTR)pbInitData, strDst, dwSize);

	CTagManager tagManager(strDst);

	BOOL bDescriptionArea = TRUE;
	if (ReadBoolProp(tagManager, PS_MFCPropertyGrid_DescriptionArea, bDescriptionArea))
	{
		EnableDescriptionArea(bDescriptionArea);
	}

	CString strDescriptionRows;
	if (tagManager.ExcludeTag(PS_MFCPropertyGrid_DescriptionRows, strDescriptionRows))
	{
		if (!strDescriptionRows.IsEmpty())
		{
			int nDescriptionRows = _ttoi((LPCTSTR)strDescriptionRows);
			if (nDescriptionRows >= 0)
			{
				SetDescriptionRows(nDescriptionRows);
			}
		}
	}

	BOOL bHeaderCtrl = TRUE;
	if (ReadBoolProp(tagManager, PS_MFCPropertyGrid_HeaderCtrl, bHeaderCtrl))
	{
		EnableHeaderCtrl(bHeaderCtrl);
	}

	BOOL bAlphabeticMode = FALSE;
	if (ReadBoolProp(tagManager, PS_MFCPropertyGrid_AlphabeticMode, bAlphabeticMode))
	{
		SetAlphabeticMode(bAlphabeticMode);
	}

	BOOL bModifiedProperties = TRUE;
	if (ReadBoolProp(tagManager, PS_MFCPropertyGrid_ModifiedProperties, bModifiedProperties))
	{
		MarkModifiedProperties(bModifiedProperties);
	}

	BOOL bVSDotNetLook = TRUE;
	if (ReadBoolProp(tagManager, PS_MFCPropertyGrid_VSDotNetLook, bVSDotNetLook))
	{
		SetVSDotNetLook(bVSDotNetLook);
	}
#endif

	return 0;
}

LRESULT CWTFPropertyGridCtrl::OnPrintClient(CDC* pDC, UINT nFlags)
{
	ASSERT_VALID(pDC);

	if (nFlags & PRF_CLIENT)
	{
		OnDraw(pDC);
	}

	return 0L;
}
