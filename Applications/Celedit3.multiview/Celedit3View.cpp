
// Celedit3View.cpp : implementation of the CCeledit3View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Celedit3.h"
#endif

#include "Celedit3Doc.h"
#include "CntrItem.h"
#include "resource.h"
#include "Celedit3View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCeledit3View

IMPLEMENT_DYNCREATE(CCeledit3View, CView)

BEGIN_MESSAGE_MAP(CCeledit3View, CView)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_COMMAND(ID_OLE_INSERT_NEW, &CCeledit3View::OnInsertObject)
	ON_COMMAND(ID_CANCEL_EDIT_CNTR, &CCeledit3View::OnCancelEditCntr)
	ON_COMMAND(ID_FILE_PRINT, &CCeledit3View::OnFilePrint)
END_MESSAGE_MAP()

// CCeledit3View construction/destruction

CCeledit3View::CCeledit3View() noexcept
{
	m_pSelection = nullptr;

	m_Rend = nullptr;

	m_ClearColor = glm::fvec4(0, 0, 0, 1);
}

CCeledit3View::~CCeledit3View()
{
}

BOOL CCeledit3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCeledit3View drawing

void CCeledit3View::OnDraw(CDC* pDC)
{
	if (!pDC)
		return;

	CCeledit3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (m_Rend && m_Rend->Initialized())
	{
		m_Rend->SetOverrideHwnd(GetSafeHwnd());

		CWnd *po = GetOwner();
		CMFCTabCtrl *ptc = dynamic_cast<CMFCTabCtrl *>(po);
		if (ptc)
		{
			COLORREF c = ptc->GetActiveTabColor();
			m_ClearColor.r = (float)(c & 0xff) / 255.0f;
			m_ClearColor.g = (float)((c >> 8) & 0xff) / 255.0f;
			m_ClearColor.b = (float)((c >> 16) & 0xff) / 255.0f;
		}
		m_Rend->SetClearColor(&m_ClearColor);
		m_Rend->SetClearDepth(1.0f);

		if (m_Rend->BeginScene(0))
		{
			m_Rend->EndScene(0);
		}

		m_Rend->SetOverrideHwnd(NULL);
	}

	// TODO: remove this code when final draw code is complete.
	if (m_pSelection != nullptr)
	{
		CSize size;
		CRect rect(10, 10, 210, 210);

		if (m_pSelection->GetExtent(&size, m_pSelection->m_nDrawAspect))
		{
			pDC->HIMETRICtoLP(&size);
			rect.right = size.cx + 10;
			rect.bottom = size.cy + 10;
		}
		m_pSelection->Draw(pDC, rect);
	}
}

void CCeledit3View::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pSelection = nullptr;    // initialize selection

	m_Rend = theApp.m_C3->GetRenderer();
	if (!m_Rend)
		return;

	if (!m_Rend->Initialized())
	{
		CRect r;
		GetClientRect(r);

		if (m_Rend->Initialize(r.Width(), r.Height(), theApp.GetMainWnd()->GetSafeHwnd(), 0))
		{
		}
	}
}

void CCeledit3View::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != nullptr && pActiveItem->GetActiveView() == this)
   {
      pActiveItem->Deactivate();
      ASSERT(GetDocument()->GetInPlaceActiveItem(this) == nullptr);
   }
   CView::OnDestroy();
}



// OLE Client support and commands

BOOL CCeledit3View::IsSelected(const CObject* pDocItem) const
{
	// The implementation below is adequate if your selection consists of
	//  only CCeledit3CntrItem objects.  To handle different selection
	//  mechanisms, the implementation here should be replaced

	// TODO: implement this function that tests for a selected OLE client item

	return pDocItem == m_pSelection;
}

void CCeledit3View::OnInsertObject()
{
	// Invoke the standard Insert Object dialog box to obtain information
	//  for new CCeledit3CntrItem object
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	BeginWaitCursor();

	CCeledit3CntrItem* pItem = nullptr;
	TRY
	{
		// Create new item connected to this document
		CCeledit3Doc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pItem = new CCeledit3CntrItem(pDoc);
		ASSERT_VALID(pItem);

		// Initialize the item from the dialog data
		if (!dlg.CreateItem(pItem))
			AfxThrowMemoryException();  // any exception will do
		ASSERT_VALID(pItem);

        if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
			pItem->DoVerb(OLEIVERB_SHOW, this);

		ASSERT_VALID(pItem);
		// As an arbitrary user interface design, this sets the selection
		//  to the last item inserted

		// TODO: reimplement selection as appropriate for your application
		m_pSelection = pItem;   // set selection to last inserted item
		pDoc->UpdateAllViews(nullptr);
	}
	CATCH(CException, e)
	{
		if (pItem != nullptr)
		{
			ASSERT_VALID(pItem);
			pItem->Delete();
		}
		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH

	EndWaitCursor();
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the container (not the server) causes the deactivation
void CCeledit3View::OnCancelEditCntr()
{
	// Close any in-place active item on this view.
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != nullptr)
	{
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == nullptr);
}

// Special handling of OnSetFocus and OnSize are required for a container
//  when an object is being edited in-place
void CCeledit3View::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != nullptr &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != nullptr)
		{
			pWnd->SetFocus();   // don't call the base class
			return;
		}
	}

	CView::OnSetFocus(pOldWnd);
}

void CCeledit3View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != nullptr)
		pActiveItem->SetItemRects();
}

void CCeledit3View::OnFilePrint()
{
	//By default, we ask the Active document to print itself
	//using IOleCommandTarget. If you don't want this behavior
	//remove the call to COleDocObjectItem::DoDefaultPrinting.
	//If the call fails for some reason, we will try printing
	//the docobject using the IPrint interface.
	CPrintInfo printInfo;
	ASSERT(printInfo.m_pPD != nullptr);
	if (S_OK == COleDocObjectItem::DoDefaultPrinting(this, &printInfo))
		return;

	CView::OnFilePrint();

}



// CCeledit3View diagnostics

#ifdef _DEBUG
void CCeledit3View::AssertValid() const
{
	CView::AssertValid();
}

void CCeledit3View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCeledit3Doc* CCeledit3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCeledit3Doc)));
	return (CCeledit3Doc*)m_pDocument;
}
#endif //_DEBUG


// CCeledit3View message handlers
