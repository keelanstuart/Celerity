// CreatePropertyDlg.cpp : implementation file
//

#include "pch.h"
#include "C3Edit.h"
#include "afxdialogex.h"
#include "CreatePropertyDlg.h"
#include "resource.h"


// CCreatePropertyDlg dialog

IMPLEMENT_DYNAMIC(CCreatePropertyDlg, CDialog)

CCreatePropertyDlg::CCreatePropertyDlg(props::IPropertySet *pprops, CWnd* pParent) : CDialog(IDD, pParent)
{
	m_pProps = pprops;
}

CCreatePropertyDlg::~CCreatePropertyDlg()
{
}

void CCreatePropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCreatePropertyDlg, CDialog)
END_MESSAGE_MAP()


// CCreatePropertyDlg message handlers

void AddComboOption(CComboBox &cb, DWORD_PTR val, const TCHAR *str)
{
	int i = cb.AddString(str);
	cb.SetItemData(i, val);
}



BOOL CCreatePropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_wndEditName.SubclassDlgItem(IDC_EDIT_PROPNAME, this))
		return FALSE;

	if (!m_wndEditFCC.SubclassDlgItem(IDC_EDIT_PROPFCC, this))
		return FALSE;

	if (!m_wndComboType.SubclassDlgItem(IDC_COMBO_PROPTYPE, this))
		return FALSE;

	if (!m_wndComboAspect.SubclassDlgItem(IDC_COMBO_PROPASPECT, this))
		return FALSE;

	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_STRING,		_T("String"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_INT,			_T("Integer (int64_t)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_INT_V2,		_T("Integer[2] (Vector type)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_INT_V3,		_T("Integer[3] (Vector type)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_INT_V4,		_T("Integer[4] (Vector type)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_FLOAT,		_T("Float"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2,	_T("Float[2] (glm::fvec2)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3,	_T("Float[3] (glm::fvec3)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4,	_T("Float[4] (glm::fvec4)"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_GUID,		_T("GUID"));
	AddComboOption(m_wndComboType, props::IProperty::PROPERTY_TYPE::PT_BOOLEAN,		_T("Boolean"));
	m_wndComboType.SetCurSel(0);

	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_GENERIC,			_T("Generic"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_FILENAME,		_T("Filename                          (String)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_DIRECTORY,		_T("Directory                         (String)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB,		_T("RGB Color                         (Numeric types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA,		_T("RGBA Color                        (Numeric types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_LATLON,			_T("Lattitude / Longitude / Altitude  (Vector types"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_ELEVAZIM,		_T("Elevation / Azimuth               (Vector types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_RASCDEC,			_T("Right-Ascension, Declination      (Vector types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_QUATERNION,		_T("Quaternion                        (Float[4] types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_BOOL_ONOFF,		_T("Off / On                          (Boolean types display \"off\" or \"on\")"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_BOOL_YESNO,		_T("No / Yes                          (Boolean types display \"no\" or \"yes\")"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_BOOL_TRUEFALSE,	_T("False / True                      (Boolean types display \"false\" or \"true\")"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_BOOL_ABLED,		_T("Disabled / Enabled                (Boolean types display \"disabled\" or \"enabled\")"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_FONT_DESC,		_T("Font Description                  (String; describes a font)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_DATE,			_T("Date                              (String / Integer; holds a time_t)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_TIME,			_T("Time                              (String / Integer; holds a time_t)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_IPADDRESS,		_T("IP Address                        (String)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR,	_T("Ambient Color                     (Numeric types - ambient)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE,	_T("Diffuse Color                     (Numeric types - diffuse)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE,	_T("Emissive Color                    (Numeric types - emissive)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR,	_T("Specular Color                    (Numeric types - specular)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_ALPHAPASS,		_T("Alpha Pass                        (Float or Vec2 for a range)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_EYE_POSITION,	_T("Eye Position                      (Vector types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_EYE_DIRECTION,	_T("Eye Direction                     (Vector types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_SUN_DIRECTION,	_T("Sun Direction                     (Vector types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR,		_T("Sun Color                         (Numeric types)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_PERCENTAGE,		_T("Percentage                        (Values are percentages)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG,	_T("Rotation in Degrees               (Numeric values are rotations in degrees)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD,	_T("Rotation in Radians               (Numeric values are rotations in radians)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS,	_T("Time                              (in Seconds)"));
	AddComboOption(m_wndComboAspect, props::IProperty::PROPERTY_ASPECT::PA_DIMENSIONS,		_T("Dimensions                        (Length, Width, Height - based on Type)"));
	m_wndComboAspect.SetCurSel(0);

	m_wndEditName.SetFocus();

	return FALSE;
}


void CCreatePropertyDlg::OnOK()
{
	m_wndEditName.GetWindowText(m_PropName);

	CString fcctmp;
	m_wndEditFCC.GetWindowText(fcctmp);
	if (fcctmp.GetLength() > 4)
	{
		if (MessageBox(_T("FCC is short for \"FOUR CHARACTER CODE\" - the FCC entered here will be truncated to 4 characters unless you cancel now."), _T("FCC Warning"), MB_OKCANCEL) == IDCANCEL)
			return;
	}

	m_PropFCC = 0;
	uint8_t *b = (uint8_t *)&m_PropFCC + 3;
	for (size_t i = 0; (i < fcctmp.GetLength()) && (i < 4); i++)
		*(b--) = (uint8_t)fcctmp[(int)i];

	m_PropType = (props::IProperty::PROPERTY_TYPE)m_wndComboType.GetItemData(m_wndComboType.GetCurSel());
	m_PropAspect = (props::IProperty::PROPERTY_ASPECT)m_wndComboAspect.GetItemData(m_wndComboAspect.GetCurSel());

	if (m_pProps->GetPropertyById(m_PropFCC))
	{
		MessageBox(_T("A property with that FCC already exists!"), _T("Create Property Failed"), MB_OK);
		return;
	}

	if (m_pProps->GetPropertyByName(m_PropName))
	{
		MessageBox(_T("A property with that name already exists!"), _T("Create Property Failed"), MB_OK);
		return;
	}

	props::IProperty *pp = m_pProps->CreateProperty(m_PropName, m_PropFCC);
	if (pp)
	{
		pp->SetAspect(m_PropAspect);

		switch (m_PropType)
		{
			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:	pp->SetBool(false);							break;
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:		pp->SetFloat(0);							break;
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:	pp->SetVec2F(props::TVec2F(0, 0));			break;
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:	pp->SetVec3F(props::TVec3F(0, 0, 0));		break;
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:	pp->SetVec4F(props::TVec4F(0, 0, 0, 0));	break;
			case props::IProperty::PROPERTY_TYPE::PT_INT:		pp->SetInt(0);								break;
			case props::IProperty::PROPERTY_TYPE::PT_INT_V2:	pp->SetVec2I(props::TVec2I(0, 0));			break;
			case props::IProperty::PROPERTY_TYPE::PT_INT_V3:	pp->SetVec3I(props::TVec3I(0, 0, 0));		break;
			case props::IProperty::PROPERTY_TYPE::PT_INT_V4:	pp->SetVec4I(props::TVec4I(0, 0, 0, 0));	break;
			case props::IProperty::PROPERTY_TYPE::PT_GUID:		pp->SetGUID(GUID());						break;
			case props::IProperty::PROPERTY_TYPE::PT_STRING:	pp->SetString(_T(""));						break;
		}
	}

	CDialog::OnOK();
}
