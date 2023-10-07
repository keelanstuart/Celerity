// GraphNode.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "GraphNode.h"
#include "afxdialogex.h"


// CGraphNode dialog

IMPLEMENT_DYNAMIC(CGraphNode, CDialog)

CGraphNode *CGraphNode::DoModeless(CWnd *pParent)
{
	CGraphNode *ret = new CGraphNode(pParent);
	if (ret)
	{
		ret->Create(IDD_GRAPHNODE, pParent);
	}

	return ret;
}

CGraphNode::CGraphNode(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_GRAPHNODE, pParent)
{

}

CGraphNode::~CGraphNode()
{
}

void CGraphNode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphNode, CDialog)
    ON_WM_MOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CGraphNode message handlers


void CGraphNode::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


void CGraphNode::OnMove(int x, int y)
{
    CDialog::OnMove(x, y);

	GetParent()->RedrawWindow();
	Invalidate();
}


void CGraphNode::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	GetParent()->RedrawWindow();
	Invalidate();
}
