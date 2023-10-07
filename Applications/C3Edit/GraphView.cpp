// GraphView.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "GraphView.h"
#include "GraphNode.h"


// CGraphView

IMPLEMENT_DYNCREATE(CGraphView, CScrollPanel)

CGraphView::CGraphView()
{

}

CGraphView::~CGraphView()
{
}


BEGIN_MESSAGE_MAP(CGraphView, CScrollPanel)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GRAPH_ADDNODE, &CGraphView::OnGraphAddNode)
	ON_UPDATE_COMMAND_UI(ID_GRAPH_ADDNODE, &CGraphView::OnUpdateGraphAddNode)
	ON_COMMAND(ID_GRAPH_DELETENODE, &CGraphView::OnGraphDeleteNode)
	ON_UPDATE_COMMAND_UI(ID_GRAPH_DELETENODE, &CGraphView::OnUpdateGraphDeleteNode)
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CGraphView drawing

void CGraphView::OnInitialUpdate()
{
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CGraphView::OnDraw(CDC* pDC)
{
//	pDC->
}



// CGraphView message handlers


void CGraphView::OnContextMenu(CWnd *pWnd, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_GRAPH, point.x, point.y, this, TRUE);
#endif
}


void CGraphView::OnGraphAddNode()
{
	CGraphNode *pgn = CGraphNode::DoModeless(this);
	pgn->SetParent(this);
}


void CGraphView::OnUpdateGraphAddNode(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}


void CGraphView::OnGraphDeleteNode()
{
	// TODO: Add your command handler code here
}


void CGraphView::OnUpdateGraphDeleteNode(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


BOOL CGraphView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags & MK_CONTROL)
	{
		XFORM mat;
		GetDC()->GetWorldTransform(&mat);

		float d = 1.0f + (0.1f / (float)zDelta);
		mat.eM11 *= d;
		mat.eM12 *= d;
		mat.eM21 *= d;
		mat.eM22 *= d;

		GetDC()->SetWorldTransform(&mat);
	}

	return CScrollPanel::OnMouseWheel(nFlags, zDelta, pt);
}


void CGraphView::OnPaint()
{
	CPaintDC dc(this); // Create a device context for the parent dialog

	// Define the scale factor
	double scaleFactor = 0.5; // Adjust this value as needed

	// Loop through your child dialogs and draw them onto the parent's DC
	CWnd* pChild = GetWindow(GW_CHILD);
	while (pChild != nullptr)
	{
		// Check if the child is a CDialog or any other CWnd-derived class that you want to draw
		if (pChild->IsKindOf(RUNTIME_CLASS(CGraphNode)))
		{
			// Hide the original child window
			pChild->ShowWindow(SW_HIDE);

			// Get the position of the child in the parent's client coordinates
			CRect rc;
			pChild->GetWindowRect(&rc);
			ScreenToClient(&rc);

			// Scale the child's position and size
			rc.left = static_cast<int>(rc.left * scaleFactor);
			rc.top = static_cast<int>(rc.top * scaleFactor);
			rc.right = static_cast<int>(rc.right * scaleFactor);
			rc.bottom = static_cast<int>(rc.bottom * scaleFactor);

			// Create a memory DC for the scaled child dialog
			CDC memDC;
			memDC.CreateCompatibleDC(&dc);

			// Create a bitmap to hold the scaled child dialog's drawing
			CBitmap bmp;
			bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
			CBitmap* pOldBitmap = memDC.SelectObject(&bmp);

			// Scale the child dialog's content to fit the new size
			memDC.SetMapMode(MM_ANISOTROPIC);
			memDC.SetWindowExt(rc.Width(), rc.Height());
			memDC.SetViewportExt(static_cast<int>(rc.Width() / scaleFactor), static_cast<int>(rc.Height() / scaleFactor));

			// Paint the scaled child dialog onto the memory DC
			pChild->SendMessage(WM_PRINT, (WPARAM)memDC.GetSafeHdc(), PRF_CLIENT | PRF_NONCLIENT | PRF_CHILDREN | PRF_ERASEBKGND);

#if 1
			int mx = memDC.GetDeviceCaps(LOGPIXELSX);
			int my = memDC.GetDeviceCaps(LOGPIXELSY);

			int dx = memDC.GetDeviceCaps(LOGPIXELSX);
			int dy = memDC.GetDeviceCaps(LOGPIXELSY);

			int aw = dx * rc.Width() / mx;
			int ah = dy * rc.Height() / my;

			dc.StretchBlt(rc.left, rc.top, rc.Width(), rc.Height(), &memDC, 0, 0, aw, ah, SRCCOPY);
#else
			dc.BitBlt(rectChild.left, rectChild.top, rectChild.Width(), rectChild.Height(), &memDC, 0, 0, SRCCOPY);
#endif

			// Show the original child window again
			pChild->ShowWindow(SW_SHOW);
		}

		// Move to the next child window
		pChild = pChild->GetNextWindow(GW_HWNDNEXT);
	}
}
