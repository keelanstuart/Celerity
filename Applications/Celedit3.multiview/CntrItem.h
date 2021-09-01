
// CntrItem.h : interface of the CCeledit3CntrItem class
//

#pragma once

class CCeledit3Doc;
class CCeledit3View;

class CCeledit3CntrItem : public COleClientItem
{
	DECLARE_SERIAL(CCeledit3CntrItem)

// Constructors
public:
	CCeledit3CntrItem(CCeledit3Doc* pContainer = nullptr);
		// Note: pContainer is allowed to be null to enable IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-null document pointer

// Attributes
public:
	CCeledit3Doc* GetDocument()
		{ return reinterpret_cast<CCeledit3Doc*>(COleClientItem::GetDocument()); }
	CCeledit3View* GetActiveView()
		{ return reinterpret_cast<CCeledit3View*>(COleClientItem::GetActiveView()); }

public:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	virtual void OnActivate();

protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
	virtual BOOL OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);

// Implementation
public:
	~CCeledit3CntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
};

