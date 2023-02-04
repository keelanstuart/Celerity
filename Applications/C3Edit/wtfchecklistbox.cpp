#include "pch.h"
#include "wtfchecklistbox.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CHECKLIST_STATE
class CHECKLIST_STATE : public CNoTrackObject
{
public:
	CHECKLIST_STATE();
	virtual ~CHECKLIST_STATE();

	HBITMAP m_hbitmapCheck;
	CSize m_sizeCheck;
};

CHECKLIST_STATE::CHECKLIST_STATE()
{
	CBitmap bitmap;

	bitmap.LoadBitmap(IDB_CHECKBOX);

	BITMAP bm;
	bitmap.GetObject(sizeof (BITMAP), &bm);
	m_sizeCheck.cx = bm.bmWidth / 3;
	m_sizeCheck.cy = bm.bmHeight;
	m_hbitmapCheck = (HBITMAP)bitmap.Detach();
}

CHECKLIST_STATE::~CHECKLIST_STATE()
{
	if (m_hbitmapCheck != NULL)
		::DeleteObject(m_hbitmapCheck);
}

EXTERN_PROCESS_LOCAL(CHECKLIST_STATE, _checklistState)

/////////////////////////////////////////////////////////////////////////////
// CHECK_DATA

struct CHECK_DATA
{
public:
	int m_nCheck;
	BOOL m_bEnabled;
	DWORD m_dwUserData;

	CHECK_DATA()
	{
		m_nCheck = 0;
		m_bEnabled = TRUE;
		m_dwUserData = 0;
	};
};


/////////////////////////////////////////////////////////////////////////////
// CWtfCheckListBox

BEGIN_MESSAGE_MAP(CWtfCheckListBox, CListBox)
	//{{AFX_MSG_MAP(CWtfCheckListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(LB_ADDSTRING, OnLBAddString)
	ON_MESSAGE(LB_FINDSTRING, OnLBFindString)
	ON_MESSAGE(LB_FINDSTRINGEXACT, OnLBFindStringExact)
	ON_MESSAGE(LB_GETITEMDATA, OnLBGetItemData)
	ON_MESSAGE(LB_GETTEXT, OnLBGetText)
	ON_MESSAGE(LB_INSERTSTRING, OnLBInsertString)
	ON_MESSAGE(LB_SELECTSTRING, OnLBSelectString)
	ON_MESSAGE(LB_SETITEMDATA, OnLBSetItemData)
	ON_MESSAGE(LB_SETITEMHEIGHT, OnLBSetItemHeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWtfCheckListBox::CWtfCheckListBox()
{

}

BOOL CWtfCheckListBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!(dwStyle & LBS_OWNERDRAWVARIABLE)) //must be one or the other
		dwStyle |= LBS_OWNERDRAWFIXED;
	return CListBox::Create(dwStyle, rect, pParentWnd, nID);
}

void CWtfCheckListBox::SetCheckStyle(UINT nStyle)
{
	ASSERT(nStyle == 0 || nStyle == BS_CHECKBOX ||
		nStyle == BS_AUTOCHECKBOX || nStyle == BS_AUTO3STATE ||
		nStyle == BS_3STATE);

	m_nStyle = nStyle;
}

void CWtfCheckListBox::SetCheck(int nIndex, int nCheck)
{
	ASSERT(::IsWindow(m_hWnd));

	if (nCheck == 2)
	{
		if (m_nStyle == BS_CHECKBOX || m_nStyle == BS_AUTOCHECKBOX)
			return;
	}

	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, nIndex, 0);
	if (lResult != LB_ERR)
	{

		CHECK_DATA* pState = (CHECK_DATA*)lResult;

		if (pState == NULL)
			pState = new CHECK_DATA;

		pState->m_nCheck = nCheck;
		VERIFY(DefWindowProc(LB_SETITEMDATA, nIndex, (LPARAM)pState) != LB_ERR);

		InvalidateCheck(nIndex);
	}
}

int CWtfCheckListBox::GetCheck(int nIndex)
{
	ASSERT(::IsWindow(m_hWnd));

	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, nIndex, 0);
	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lResult;
		if (pState != NULL)
			return pState->m_nCheck;
	}
	return 0; // The default
}

void CWtfCheckListBox::Enable(int nIndex, BOOL bEnabled)
{
	ASSERT(::IsWindow(m_hWnd));

	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, nIndex, 0);
	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lResult;

		if (pState == NULL)
			pState = new CHECK_DATA;

		pState->m_bEnabled = bEnabled;
		VERIFY(DefWindowProc(LB_SETITEMDATA, nIndex, (LPARAM)pState) != LB_ERR);

		InvalidateItem(nIndex);
	}
}

int CWtfCheckListBox::IsEnabled(int nIndex)
{
	ASSERT(::IsWindow(m_hWnd));

	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, nIndex, 0);
	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lResult;
		if (pState != NULL)
			return pState->m_bEnabled;
	}
	return TRUE; // The default
}

CRect CWtfCheckListBox::OnGetCheckPosition(CRect, CRect rectCheckBox)
{
	return rectCheckBox;
}

void CWtfCheckListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) ==
		(LBS_OWNERDRAWFIXED | LBS_HASSTRINGS));

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (((LONG)(lpDrawItemStruct->itemID) >= 0) &&
		(lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
	{
		int cyItem = GetItemHeight(lpDrawItemStruct->itemID);
		BOOL fDisabled = !IsWindowEnabled() || !IsEnabled(lpDrawItemStruct->itemID);

		COLORREF newTextColor = fDisabled ?
			RGB(0x80, 0x80, 0x80) : RGB(0xFF, 0xFF, 0xFF);//GetSysColor(COLOR_WINDOWTEXT);  // light gray
		COLORREF oldTextColor = pDC->SetTextColor(newTextColor);

		COLORREF newBkColor = RGB(64, 64, 64);//GetSysColor(COLOR_WINDOW);
		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xFF, 0xFF, 0xFF);   // dark gray

		if (!fDisabled && ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0))
		{
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkColor(RGB(80, 80, 96));
		}

		if (m_cyText == 0)
			VERIFY(cyItem >= CalcMinimumItemHeight());

		CString strText;
		GetText(lpDrawItemStruct->itemID, strText);

		pDC->ExtTextOut(lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top + std::max(0, (cyItem - m_cyText) / 2),
			ETO_OPAQUE, &(lpDrawItemStruct->rcItem), strText, strText.GetLength(), NULL);

		pDC->SetTextColor(oldTextColor);
		pDC->SetBkColor(oldBkColor);
	}

	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) != 0)
		pDC->DrawFocusRect(&(lpDrawItemStruct->rcItem));
}

void CWtfCheckListBox::MeasureItem(LPMEASUREITEMSTRUCT)
{
	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) ==
		(LBS_OWNERDRAWFIXED | LBS_HASSTRINGS));
}

void CWtfCheckListBox::PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CHECKLIST_STATE* pChecklistState = _checklistState;

	DRAWITEMSTRUCT drawItem;
	memcpy(&drawItem, lpDrawItemStruct, sizeof(DRAWITEMSTRUCT));

	if ((((LONG)drawItem.itemID) >= 0) &&
	   ((drawItem.itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) != 0))
	{
		int cyItem = GetItemHeight(drawItem.itemID);

		CDC* pDC = CDC::FromHandle(drawItem.hDC);

		COLORREF newBkColor = RGB(64, 64, 64);//GetSysColor(COLOR_WINDOW);

		BOOL fDisabled = !IsWindowEnabled() || !IsEnabled(drawItem.itemID);
		if ((drawItem.itemState & ODS_SELECTED) && !fDisabled)
			newBkColor = RGB(80, 80, 96);//GetSysColor(COLOR_HIGHLIGHT);

		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		CDC bitmapDC;
		if (bitmapDC.CreateCompatibleDC(pDC))
		{
			int nCheck = GetCheck(drawItem.itemID);
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(bitmapDC.m_hDC, pChecklistState->m_hbitmapCheck);

			CRect rectCheck = drawItem.rcItem;
			rectCheck.left += 1;
			rectCheck.top += 1 + std::max<int>(0, (cyItem - pChecklistState->m_sizeCheck.cy) / 2);
			rectCheck.right = rectCheck.left + pChecklistState->m_sizeCheck.cx;
			rectCheck.bottom = rectCheck.top + pChecklistState->m_sizeCheck.cy;

			CRect rectItem = drawItem.rcItem;
			rectItem.right = rectItem.left + pChecklistState->m_sizeCheck.cx + 2;

			CRect rectCheckBox = OnGetCheckPosition(rectItem, rectCheck);

			ASSERT(rectCheck.IntersectRect(rectItem, rectCheckBox));
			ASSERT((rectCheck == rectCheckBox) && (rectCheckBox.Size() == pChecklistState->m_sizeCheck));

			CBrush brush(newBkColor);
			pDC->FillRect(rectItem, &brush);

			pDC->BitBlt(rectCheckBox.left, rectCheckBox.top,
				pChecklistState->m_sizeCheck.cx, pChecklistState->m_sizeCheck.cy, &bitmapDC,
				pChecklistState->m_sizeCheck.cx  * nCheck, 0, SRCCOPY);

			::SelectObject(bitmapDC.m_hDC, hOldBitmap);
		}
		pDC->SetBkColor(oldBkColor);
	}

	if (drawItem.itemData != 0 && drawItem.itemData != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)drawItem.itemData;
		drawItem.itemData = pState->m_dwUserData;
	}
	drawItem.rcItem.left = drawItem.rcItem.left + pChecklistState->m_sizeCheck.cx + 2;

	DrawItem(&drawItem);
}

void CWtfCheckListBox::PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	int cyItem = CalcMinimumItemHeight();

	MEASUREITEMSTRUCT measureItem;
	memcpy(&measureItem, lpMeasureItemStruct, sizeof(MEASUREITEMSTRUCT));

	measureItem.itemHeight = cyItem;
	measureItem.itemWidth  = (UINT)-1;

	// WINBUG: Windows95 and Windows NT disagree on what this value
	// should be.  According to the docs, they are both wrong
	if (GetStyle() & LBS_OWNERDRAWVARIABLE)
	{
		LRESULT lResult = DefWindowProc(LB_GETITEMDATA, measureItem.itemID, 0);
		if (lResult != LB_ERR)
			measureItem.itemData = (UINT)lResult;
		else
			measureItem.itemData = 0;

		// WINBUG: This is only done in the LBS_OWNERDRAWVARIABLE case
		// because Windows 95 does not initialize itemData to zero in the
		// case of LBS_OWNERDRAWFIXED list boxes (it is stack garbage).
		if (measureItem.itemData != 0 && measureItem.itemData != LB_ERR)
		{
			CHECK_DATA* pState = (CHECK_DATA*)measureItem.itemData;
			measureItem.itemData = pState->m_dwUserData;
		}
	}

	MeasureItem(&measureItem);

	lpMeasureItemStruct->itemHeight = std::max(measureItem.itemHeight,(UINT) cyItem);
	lpMeasureItemStruct->itemWidth = measureItem.itemWidth;
}

int CWtfCheckListBox::PreCompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	COMPAREITEMSTRUCT compareItem;
	memcpy(&compareItem, lpCompareItemStruct, sizeof(COMPAREITEMSTRUCT));

	if (compareItem.itemData1 != 0 && compareItem.itemData1 != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)compareItem.itemData1;
		compareItem.itemData1 = pState->m_dwUserData;
	}
	if (compareItem.itemData2 != 0 && compareItem.itemData2 != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)compareItem.itemData2;
		compareItem.itemData2 = pState->m_dwUserData;
	}
	return CompareItem(&compareItem);
}

void CWtfCheckListBox::PreDeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	DELETEITEMSTRUCT deleteItem;
	memcpy(&deleteItem, lpDeleteItemStruct, sizeof(DELETEITEMSTRUCT));

	// WINBUG: The following if block is required because Windows NT
	// version 3.51 does not properly fill out the LPDELETEITEMSTRUCT.
	if (deleteItem.itemData == 0)
	{
		LRESULT lResult = DefWindowProc(LB_GETITEMDATA, deleteItem.itemID, 0);
		if (lResult != LB_ERR)
			deleteItem.itemData = (UINT)lResult;
	}

	if (deleteItem.itemData != 0 && deleteItem.itemData != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)deleteItem.itemData;
		deleteItem.itemData = pState->m_dwUserData;
		delete pState;
	}
	DeleteItem(&deleteItem);
}

BOOL CWtfCheckListBox::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
	LRESULT* pResult)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(pResult == NULL);       // no return value expected
		PreDrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(pResult == NULL);       // no return value expected
		PreMeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(pResult != NULL);       // return value expected
		*pResult = PreCompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(pResult == NULL);       // no return value expected
		PreDeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	default:
		return CListBox::OnChildNotify(message, wParam, lParam, pResult);
	}
	return TRUE;
}

#ifdef _DEBUG
void CWtfCheckListBox::PreSubclassWindow()
{
	CListBox::PreSubclassWindow();

	// CWtfCheckListBoxes must be owner drawn
	ASSERT(GetStyle() & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE));
}
#endif

int CWtfCheckListBox::CalcMinimumItemHeight()
{
	int nResult;

	CHECKLIST_STATE* pChecklistState = _checklistState;

	if ((GetStyle() & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) ==
		(LBS_HASSTRINGS | LBS_OWNERDRAWFIXED))
	{
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(GetFont());
		TEXTMETRIC tm;
		VERIFY (dc.GetTextMetrics ( &tm ));
		dc.SelectObject(pOldFont);

		m_cyText = tm.tmHeight;
		nResult = std::max<int>(pChecklistState->m_sizeCheck.cy + 1, m_cyText);
	}
	else
	{
		nResult = pChecklistState->m_sizeCheck.cy + 1;
	}

	return nResult;
}

void CWtfCheckListBox::InvalidateCheck(int nIndex)
{
	CRect rect;
	CHECKLIST_STATE* pChecklistState = _checklistState;

	GetItemRect(nIndex, rect);
	rect.right = rect.left + pChecklistState->m_sizeCheck.cx + 2;
	InvalidateRect(rect, FALSE);
}

void CWtfCheckListBox::InvalidateItem(int nIndex)
{
	CRect rect;
	GetItemRect(nIndex, rect);
	InvalidateRect(rect, FALSE);
}

int CWtfCheckListBox::CheckFromPoint(CPoint point, BOOL& bInCheck)
{
	// assume did not hit anything
	bInCheck = FALSE;
	int nIndex = -1;

	CHECKLIST_STATE* pChecklistState = _checklistState;
	if ((GetStyle() & (LBS_OWNERDRAWFIXED|LBS_MULTICOLUMN)) == LBS_OWNERDRAWFIXED)
	{
		// optimized case for ownerdraw fixed, single column
		int cyItem = GetItemHeight(0);
		if (point.y < cyItem * GetCount())
		{
			nIndex = GetTopIndex() + point.y / cyItem;
			if (point.x < pChecklistState->m_sizeCheck.cx + 2)
				++bInCheck;
		}
	}
	else
	{
		// general case for ownerdraw variable or multiple column
		for (int i = GetTopIndex(); i < GetCount(); i++)
		{
			CRect itemRect;
			GetItemRect(i, &itemRect);
			if (itemRect.PtInRect(point))
			{
				nIndex = i;
				if (point.x < itemRect.left + pChecklistState->m_sizeCheck.cx + 2)
					++bInCheck;
				break;
			}
		}
	}
	return nIndex;
}

void CWtfCheckListBox::SetSelectionCheck( int nCheck )
{
   int* piSelectedItems;
   int nSelectedItems;
   int iSelectedItem;

   nSelectedItems = GetSelCount();
   if( nSelectedItems > 0 )
   {
	  piSelectedItems = (int*)_alloca( nSelectedItems*sizeof( int ) );
	  GetSelItems( nSelectedItems, piSelectedItems );
	  for( iSelectedItem = 0; iSelectedItem < nSelectedItems; iSelectedItem++ )
	  {
		 if( IsEnabled( piSelectedItems[iSelectedItem] ) )
		 {
			SetCheck( piSelectedItems[iSelectedItem], nCheck );
			InvalidateCheck( piSelectedItems[iSelectedItem] );
		 }
	  }
   }
}

void CWtfCheckListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
   SetFocus();

   // determine where the click is
   BOOL bInCheck;
   int nIndex = CheckFromPoint(point, bInCheck);

   // if the item is disabled, then eat the click
   if (!IsEnabled(nIndex))
	  return;

   if (m_nStyle != BS_CHECKBOX && m_nStyle != BS_3STATE)
   {
	  // toggle the check mark automatically if the check mark was hit
	  if (bInCheck)
	  {
		 CWnd* pParent = GetParent();
		 ASSERT_VALID( pParent );

			int nModulo = (m_nStyle == BS_AUTO3STATE) ? 3 : 2;
		 int nCheck;
		 int nNewCheck;

		 nCheck = GetCheck( nIndex );
		 nCheck = (nCheck == nModulo) ? nCheck-1 : nCheck;
		 nNewCheck = (nCheck+1)%nModulo;
		 SetCheck( nIndex, nNewCheck );
		 InvalidateCheck( nIndex );

		 if( (GetStyle()&(LBS_EXTENDEDSEL|LBS_MULTIPLESEL)) && GetSel(
			nIndex ) )
		 {
			// The listbox is a multi-select listbox, and the user clicked on
			// a selected check, so change the check on all of the selected
			// items.
			SetSelectionCheck( nNewCheck );
		 }
		 else
		 {
			CListBox::OnLButtonDown( nFlags, point );
		 }

		 // Inform parent of check
		 pParent->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(),
			CLBN_CHKCHANGE ), (LPARAM)m_hWnd );
		 return;
	  }
   }

	// do default listbox selection logic
   CListBox::OnLButtonDown( nFlags, point );
}

void CWtfCheckListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BOOL bInCheck;
	CheckFromPoint(point, bInCheck);

	if (bInCheck)
	{
		// Double and single clicks act the same on the check box!
		OnLButtonDown(nFlags, point);
		return;
	}

	CListBox::OnLButtonDblClk(nFlags, point);
}

void CWtfCheckListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		int nIndex = GetCaretIndex();
		CWnd* pParent = GetParent();
		ASSERT_VALID(pParent);

		if (nIndex != LB_ERR)
		{
			if (m_nStyle != BS_CHECKBOX && m_nStyle != BS_3STATE)
			{
				if ((GetStyle() & LBS_MULTIPLESEL) != 0)
				{
					if (IsEnabled(nIndex))
					{
						BOOL bSelected = GetSel(nIndex);
						if (bSelected)
						{
							int nModulo = (m_nStyle == BS_AUTO3STATE) ? 3 : 2;
							int nCheck = GetCheck(nIndex);
							nCheck = (nCheck == nModulo) ? nCheck - 1 : nCheck;
							SetCheck(nIndex, (nCheck + 1) % nModulo);

							// Inform of check
							pParent->SendMessage(WM_COMMAND,
								MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE),
								(LPARAM)m_hWnd);
						}
						SetSel(nIndex, !bSelected);
					}
					else
						SetSel(nIndex, FALSE); // unselect disabled items

					return;
				}
				else
				{
					// If there is a selection, the space bar toggles that check,
					// all other keys are the same as a standard listbox.

					if (IsEnabled(nIndex))
					{
						int nModulo = (m_nStyle == BS_AUTO3STATE) ? 3 : 2;
						int nCheck = GetCheck(nIndex);
						nCheck = (nCheck == nModulo) ? nCheck - 1 : nCheck;
				  int nNewCheck = (nCheck+1)%nModulo;
						SetCheck(nIndex, nNewCheck);

						InvalidateCheck(nIndex);

				  if( GetStyle()&LBS_EXTENDEDSEL )
				  {
					 // The listbox is a multi-select listbox, and the user
					 // clicked on a selected check, so change the check on all
					 // of the selected items.
					 SetSelectionCheck( nNewCheck );
				  }

						// Inform of check
						pParent->SendMessage(WM_COMMAND,
							MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE),
							(LPARAM)m_hWnd);
					}
					else
						SetSel(nIndex, FALSE); // unselect disabled items

					return;
				}
			}
		}
	}
	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

int CWtfCheckListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
		== (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
		SetItemHeight(0, CalcMinimumItemHeight());

	return 0;
}

LRESULT CWtfCheckListBox::OnSetFont(WPARAM , LPARAM)
{
	Default();

	if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
		== (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
		SetItemHeight(0, CalcMinimumItemHeight());

	return 0;
}

LRESULT CWtfCheckListBox::OnLBAddString(WPARAM wParam, LPARAM lParam)
{
	CHECK_DATA* pState = NULL;

	if (!(GetStyle() & LBS_HASSTRINGS))
	{
		pState = new CHECK_DATA;

		pState->m_dwUserData = (DWORD)lParam;
		lParam = (LPARAM)pState;
	}

	LRESULT lResult = DefWindowProc(LB_ADDSTRING, wParam, lParam);

	if (lResult == LB_ERR && pState != NULL)
		delete pState;

	return lResult;
}

LRESULT CWtfCheckListBox::OnLBFindString(WPARAM wParam, LPARAM lParam)
{
	if (GetStyle() & LBS_HASSTRINGS)
		return DefWindowProc(LB_FINDSTRING, wParam, lParam);

	int nIndex = (int)wParam;
	if (nIndex == -1) nIndex = 0;

	for(; nIndex < GetCount(); nIndex++)
		if ((DWORD)lParam == GetItemData(nIndex))
			return nIndex;

	return LB_ERR;
}

LRESULT CWtfCheckListBox::OnLBFindStringExact(WPARAM wParam, LPARAM lParam)
{
	if (GetStyle() & (LBS_HASSTRINGS | LBS_SORT))
		return DefWindowProc(LB_FINDSTRINGEXACT, wParam, lParam);

	int nIndex = (int)wParam;
	if (nIndex == -1) nIndex = 0;

	for(; nIndex < GetCount(); nIndex++)
		if ((DWORD)lParam == GetItemData(nIndex))
			return nIndex;

	return LB_ERR;
}

LRESULT CWtfCheckListBox::OnLBGetItemData(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, wParam, lParam);

	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lResult;

		if (pState == NULL)
			return 0; // default

		lResult = pState->m_dwUserData;
	}
	return lResult;
}

LRESULT CWtfCheckListBox::OnLBGetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_GETTEXT, wParam, lParam);

	if (GetStyle() & LBS_HASSTRINGS)
		return lResult;

	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lParam;

		if (pState != NULL)
			lParam = pState->m_dwUserData;
	}
	return lResult;
}

LRESULT CWtfCheckListBox::OnLBInsertString(WPARAM wParam, LPARAM lParam)
{
	CHECK_DATA* pState = NULL;

	if (!(GetStyle() & LBS_HASSTRINGS))
	{
		pState = new CHECK_DATA;
		pState->m_dwUserData = (DWORD)lParam;
		lParam = (LPARAM)pState;
	}

	LRESULT lResult = DefWindowProc(LB_INSERTSTRING, wParam, lParam);

	if (lResult == LB_ERR && pState != NULL)
		delete pState;

	return lResult;
}

LRESULT CWtfCheckListBox::OnLBSelectString(WPARAM wParam, LPARAM lParam)
{
	if (GetStyle() & LBS_HASSTRINGS)
		return DefWindowProc(LB_SELECTSTRING, wParam, lParam);

	int nIndex = (int)wParam;
	if (nIndex == -1) nIndex = 0;

	for(; nIndex < GetCount(); nIndex++)
		if ((DWORD)lParam == GetItemData(nIndex))
		{
			SetCurSel(nIndex);
			return nIndex;
		}

	return LB_ERR;
}

LRESULT CWtfCheckListBox::OnLBSetItemData(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, wParam, 0);

	if (lResult != LB_ERR)
	{
		CHECK_DATA* pState = (CHECK_DATA*)lResult;

		if (pState == NULL)
			pState = new CHECK_DATA;

		pState->m_dwUserData = (DWORD)lParam;
		lResult = DefWindowProc(LB_SETITEMDATA, wParam, (LPARAM)pState);

		if (lResult == LB_ERR)
			delete pState;
	}
	return lResult;
}

LRESULT CWtfCheckListBox::OnLBSetItemHeight(WPARAM wParam, LPARAM lParam)
{
	int nHeight = std::max(CalcMinimumItemHeight(),(int)LOWORD(lParam));
	return DefWindowProc(LB_SETITEMHEIGHT, wParam, MAKELPARAM(nHeight,0));
}

IMPLEMENT_DYNAMIC(CWtfCheckListBox, CListBox)

PROCESS_LOCAL(CHECKLIST_STATE, _checklistState)

/////////////////////////////////////////////////////////////////////////////