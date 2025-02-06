#pragma once
#include "afxdialogex.h"


// CBrushSettingsDlg dialog

class CBrushSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CBrushSettingsDlg)

public:
	CBrushSettingsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBrushSettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BRUSHSETTINGS };
#endif

	using PlacementMode = enum
	{
		PLACEMENT_RAYCAST = 0,
		PLACEMENT_XYCAM,
		PLACEMENT_YZCAM,
		PLACEMENT_XZCAM,
		PLACEMENT_XYORIG,
		PLACEMENT_YZORIG,
		PLACEMENT_XZORIG,
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
//	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedCheckbox();

	void UpdateEnabled(bool live);
};
