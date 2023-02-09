
#pragma once

#include "PropertyGrid.h"
#include "wtfchecklistbox.h"

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPropertiesCheckListBox : public CCheckListBox
{
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT pdi);
	virtual void PreDrawItem(LPDRAWITEMSTRUCT pdi);
};

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd() noexcept;

	void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

	void SetActivePrototype(c3::Prototype *pproto);
	void SetActiveObject(c3::Object *pobj);
	void SetActiveProperties(props::IPropertySet* props, bool readonly, const TCHAR* title);
	void UpdateCurrentProperties() { m_wndPropList.UpdateCurrentProperties(); }

	void FillOutFlags();
	void FillOutComponents();

protected:
	CFont m_fntPropList;
	CPropertiesToolBar m_wndToolBar;
	CEdit m_wndNameEdit;
	CWtfCheckListBox m_wndCompList;
	CWtfCheckListBox m_wndFlagList;
	CPropertyGrid m_wndPropList;

	c3::Prototype *m_pProto;
	c3::Object *m_pObj;
	props::IPropertySet *m_pProps;
	bool m_bExpanded;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnCtlcolorlistbox(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCheckChangeFlags();
	afx_msg void OnCheckChangeComponents();

	DECLARE_MESSAGE_MAP()

	void InitPropList();

	int m_nComboHeight;
public:
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
};

