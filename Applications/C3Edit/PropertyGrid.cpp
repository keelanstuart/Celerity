/*
	Copyright © 2013-2026, Keelan Stuart (hereafter referenced as AUTHOR). All Rights Reserved.
	Permission to use, copy, modify, and distribute this software is hereby granted, without fee and without a signed licensing agreement,
	provided that the above copyright notice appears in all copies, modifications, and distributions.
	Furthermore, AUTHOR assumes no responsibility for any damages caused either directly or indirectly by the use of this software, nor vouches for
	any fitness of purpose of this software.
	All other copyrighted material contained herein is noted and rights attributed to individual copyright holders.
	
	For inquiries, contact: keelanstuart@gmail.com
*/

// PropertyGrid.cpp : implementation file
//

#include "pch.h"
#include "PropertyGrid.h"
#include "C3EditFrame.h"
#include "C3EditDoc.h"
#include "C3Edit.h"
#include "C3EditView.h"
#include <C3Font.h>

// CPropertyGrid

IMPLEMENT_DYNAMIC(CPropertyGrid, CMFCPropertyGridCtrl)

CPropertyGrid::CPropertyGrid()
{
	m_bLocked = false;
	m_Props = nullptr;
	m_bUnfocus = true;
}


CPropertyGrid::~CPropertyGrid()
{
}


CWTFPropertyGridProperty *CPropertyGrid::FindItemByName(const TCHAR *name, CWTFPropertyGridProperty *top)
{
	CWTFPropertyGridProperty *ret = nullptr;

	if (!top)
	{
		for (int i = 0, maxi = GetPropertyCount(); i < maxi; i++)
		{
			CWTFPropertyGridProperty *p = GetProperty(i);
			if (!_tcsicmp(p->GetName(), name))
				return p;

			if (p->GetSubItemsCount() && (nullptr != (ret = FindItemByName(name, p))))
				break;
		}
	}
	else
	{
		for (int i = 0, maxi = top->GetSubItemsCount(); i < maxi; i++)
		{
			CWTFPropertyGridProperty *p = top->GetSubItem(i);
			if (!_tcsicmp(p->GetName(), name))
				return p;

			if (p->GetSubItemsCount() && (nullptr != (ret = FindItemByName(name, p))))
				break;
		}
	}

	return ret;
}

class CWTFPropertyGridDateProperty : public CWTFPropertyGridProperty
{
	DECLARE_DYNAMIC(CWTFPropertyGridDateProperty)

	// Construction
public:

	CWTFPropertyGridDateProperty(const CString& strName, struct tm &datetime) :
		CWTFPropertyGridProperty(strName, _T("1900/01/01"), NULL, NULL, NULL, NULL, _T("0123456789/-")),
		m_Date(datetime)
	{
		TCHAR buf[64];
		_stprintf_s(buf, _T("%04d-%02d-%02d"), m_Date.tm_year + 1900, m_Date.tm_mon + 1, m_Date.tm_mday);
		SetValue(buf);
		AllowEdit(false);
	}

	virtual ~CWTFPropertyGridDateProperty()
	{
	}

	// Overrides
public:
	virtual void OnClickButton(CPoint point)
	{
	}

	virtual BOOL HasButton() { return TRUE; }

	// Attributes
protected:
	struct tm m_Date;
};

IMPLEMENT_DYNAMIC(CWTFPropertyGridDateProperty, CWTFPropertyGridProperty)

class CWTFPropertyGridTimeProperty : public CWTFPropertyGridProperty
{
	DECLARE_DYNAMIC(CWTFPropertyGridTimeProperty)

	// Construction
public:

	CWTFPropertyGridTimeProperty(const CString& strName, struct tm &time) :
		CWTFPropertyGridProperty(strName, _T("00:00:00"), NULL, NULL, NULL, NULL, _T("0123456789:")),
		m_Time(time)
	{
		TCHAR buf[64];
		_stprintf_s(buf, _T("%02d:%02d:%02d"), m_Time.tm_hour, m_Time.tm_min, m_Time.tm_sec);
		SetValue(buf);
		AllowEdit(false);
	}

	virtual ~CWTFPropertyGridTimeProperty()
	{
	}

	// Overrides
public:
	virtual void OnClickButton(CPoint point)
	{
	}

	virtual BOOL HasButton() { return TRUE; }

	// Attributes
protected:
	struct tm m_Time;
};

IMPLEMENT_DYNAMIC(CWTFPropertyGridTimeProperty, CWTFPropertyGridProperty)

CWTFPropertyGridProperty *CPropertyGrid::FindItemByID(props::FOURCHARCODE id, CWTFPropertyGridProperty *top)
{
	CWTFPropertyGridProperty *ret = nullptr;

	if (!top)
	{
		for (int i = 0, maxi = GetPropertyCount(); i < maxi; i++)
		{
			CWTFPropertyGridProperty *p = GetProperty(i);
			if (p->GetData() == id)
				return p;

			if (p->GetSubItemsCount() && (nullptr != (ret = FindItemByID(id, p))))
				break;
		}
	}
	else
	{
		for (int i = 0, maxi = top->GetSubItemsCount(); i < maxi; i++)
		{
			CWTFPropertyGridProperty *p = top->GetSubItem(i);
			if (p->GetData() == id)
				return p;

			if (p->GetSubItemsCount() && (nullptr != (ret = FindItemByID(id, p))))
				break;
		}
	}

	return ret;
}


void CPropertyGrid::SetActiveProperties(props::IPropertySet *pprops, PROPERTY_DESCRIPTION_CB prop_desc, FILE_FILTER_CB file_filter, bool reset)
{
	if (reset)
	{
		if (pprops == m_Props)
			return;

		if (m_Props)
			RemoveAll();

		m_Props = pprops;
	}

	if (!pprops)
	{
		AdjustLayout();
		RedrawWindow();
		return;
	}

	TCHAR tempname[256];
	SetGroupNameFullWidth(TRUE, FALSE);

	for (size_t i = 0, maxi = m_Props ? pprops->GetPropertyCount() : 0; i < maxi; i++)
	{
		props::IProperty *p = pprops->GetProperty(i);
		if (p->Flags().IsSet(props::IProperty::PROPFLAG(props::IProperty::HIDDEN)))
			if (!theApp.m_Config->GetBool(_T("properties.showhidden"), false))
				continue;

		CWTFPropertyGridProperty *pwp = nullptr, *parent_prop = nullptr;

		_tcsncpy_s(tempname, 255, p->GetName(), 255);
		TCHAR *name_start = tempname, *name_end = name_start;
		CString propname;

		while (nullptr != (name_end  = _tcschr(name_start, _T('\\'))))
		{
			*name_end = _T('\0');

			propname = name_start;

			CWTFPropertyGridProperty *tmpprop = FindItemByName(propname);

			if (reset)
			{
				// create a collapsible parent property
				if (!tmpprop)
				{
					tmpprop = new CWTFPropertyGridProperty(propname, 0Ui64, FALSE);

					if (parent_prop)
						parent_prop->AddSubItem(tmpprop);
					else
						AddProperty(tmpprop);
				}
			}

			parent_prop = tmpprop;

			// move past it
			name_end++;
			name_start = name_end;
		}

		propname = name_start;

		if (!reset)
		{
			props::IProperty *op = m_Props->GetPropertyById(p->GetID());
			if (op && !p->IsSameAs(op))
			{
				CWTFPropertyGridProperty *tmpprop = FindItemByName(propname);
				if (tmpprop)
				{
					tmpprop->SetValue(_T(""));
				}
			}

			continue;
		}

		switch (p->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_STRING:
			case props::IProperty::PROPERTY_TYPE::PT_GUID:
			{
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_FILENAME:
					{
						if (reset)
						{
							pwp = new CWTFPropertyGridFileProperty(propname, TRUE, CString(p->AsString()), _T(""), 0, file_filter ? file_filter(p->GetID()) : _T("*.*"));
							pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A File"));
						}
						else
						{
						}
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_DIRECTORY:
					{
						pwp = new CWTFPropertyGridFileProperty(propname, CString(p->AsString()), 0);
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Directory"));
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_DATE:
					{
						time_t tt = p->AsInt();
						struct tm _tt;
						gmtime_s(&_tt, &tt);
						pwp = new CWTFPropertyGridDateProperty(propname, _tt);
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Date"));
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_TIME:
					{
						time_t tt = p->AsInt();
						struct tm _tt;
						gmtime_s(&_tt, &tt);
						pwp = new CWTFPropertyGridTimeProperty(propname, _tt);
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Time"));
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_FONT_DESC:
					{
						static LOGFONT lf;
						c3::Font::ParseFontDescIntoLOGFONT((const TCHAR * const)p->AsString(), lf);
						pwp = new CWTFPropertyGridFontProperty(propname, lf, CF_SCREENFONTS | CF_EFFECTS);
						break;
					}

					default:
					{
						pwp = new CWTFPropertyGridProperty(propname, p->AsString());
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A String"));
						break;
					}
				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
			{
				const TCHAR *truestr, *falsestr;
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_BOOL_YESNO:
						truestr = _T("Yes"); falsestr = _T("No");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_BOOL_ONOFF:
						truestr = _T("On"); falsestr = _T("Off");
						break;

					default:
						truestr = _T("True"); falsestr = _T("False");
						break;
				}

				pwp = new CWTFPropertyGridProperty(propname, p->AsBool() ? truestr : falsestr, _T(""));
				if (pwp)
				{
					pwp->AddOption(falsestr);
					pwp->AddOption(truestr);
					pwp->AllowEdit(false);
					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Boolean"));

				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_ENUM:
			{
				pwp = new CWTFPropertyGridProperty(propname, p->AsString());
				if (pwp)
				{
					for (size_t i = 0, maxi = p->GetMaxEnumVal(); i < maxi; i++)
						pwp->AddOption(p->GetEnumString(i));

					pwp->AllowEdit(false);
					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("An Enumerated Type"));
				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT:
			{
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
					{
						UINT r = GetRValue(p->AsInt());
						UINT g = GetGValue(p->AsInt());
						UINT b = GetBValue(p->AsInt());
						pwp = new CWTFPropertyGridColorProperty(propname, RGB(r, g, b));
						((CWTFPropertyGridColorProperty *)pwp)->EnableOtherButton(_T("Other..."));
						((CWTFPropertyGridColorProperty *)pwp)->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Color"));
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_DATE:
					{
						time_t tt = p->AsInt();
						struct tm _tt;
						gmtime_s(&_tt, &tt);

						pwp = new CWTFPropertyGridDateProperty(propname, _tt);
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Date"));
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_TIME:
					{
						time_t tt = p->AsInt();
						struct tm _tt;
						gmtime_s(&_tt, &tt);

						pwp = new CWTFPropertyGridTimeProperty(propname, _tt);
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Time"));
						break;
					}

					default:
					{
						pwp = new CWTFPropertyGridProperty(propname, (LONG)(p->AsInt()), NULL, NULL, NULL, NULL, _T("0123456789-"));
						pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("An Integer"));
						break;
					}
				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			{
				pwp = new CWTFPropertyGridProperty(propname, 0, TRUE);

				const TCHAR *xname = _T("x"), *yname = _T("y"), *zname = _T("z"), *wname = _T("w");
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_DIMENSIONS:
						xname = _T("Width");
						yname = _T("Length");
						zname = _T("Height");
						wname = _T("Time? Really?");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_RECT:
						xname = _T("X1");
						yname = _T("Y1");
						zname = _T("X2");
						wname = _T("Y2");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_RECT_TLBR:
						xname = _T("Top");
						yname = _T("Left");
						zname = _T("Bottom");
						wname = _T("Right");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG:
					case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD:
						xname = _T("Pitch");
						yname = _T("Roll");
						zname = _T("Yaw");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_LATLON:
						xname = _T("Longitude");
						yname = _T("Lattitude");
						zname = _T("Altitude");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
						xname = _T("Red");
						yname = _T("Green");
						zname = _T("Blue");
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
						xname = _T("Red");
						yname = _T("Green");
						zname = _T("Blue");
						wname = _T("Alpha");
						break;
				}

				if (p->Flags().IsSet(props::IProperty::PROPFLAG(props::IProperty::MINMAX)))
				{
					xname = _T("Minimum");
					yname = _T("Maximum");
				}
				else if (p->Flags().IsSet(props::IProperty::PROPFLAG(props::IProperty::INNEROUTER)))
				{
					xname = _T("Inner");
					yname = _T("Outer");
				}

				if (p->GetType() == props::IProperty::PROPERTY_TYPE::PT_INT_V2)
				{
					props::TVec2I v2i;
					p->AsVec2I(&v2i);
					pwp->AddSubItem(new CWTFPropertyGridProperty(xname, (LONG)(v2i.x), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(yname, (LONG)(v2i.y), NULL, NULL, NULL, NULL, _T("0123456789-")));
				}
				else if (p->GetType() == props::IProperty::PROPERTY_TYPE::PT_INT_V3)
				{
					props::TVec3I v3i;
					p->AsVec3I(&v3i);
					pwp->AddSubItem(new CWTFPropertyGridProperty(xname, (LONG)(v3i.x), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(yname, (LONG)(v3i.y), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(zname, (LONG)(v3i.z), NULL, NULL, NULL, NULL, _T("0123456789-")));
				}
				else if (p->GetType() == props::IProperty::PROPERTY_TYPE::PT_INT_V4)
				{
					props::TVec4I v4i;
					p->AsVec4I(&v4i);
					pwp->AddSubItem(new CWTFPropertyGridProperty(xname, (LONG)(v4i.x), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(yname, (LONG)(v4i.y), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(zname, (LONG)(v4i.z), NULL, NULL, NULL, NULL, _T("0123456789-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(wname, (LONG)(v4i.w), NULL, NULL, NULL, NULL, _T("0123456789-")));
				}

				pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T(""));
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
			{
				pwp = new CWTFPropertyGridProperty(propname, p->AsFloat(), NULL, NULL, NULL, NULL, _T("0123456789.-"));
				pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("A Real Number"));
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
			{
				bool iscolor =
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR) ||
					(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR);

				if ((props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3 == p->GetType()) && iscolor)
				{
					UINT r = UINT(std::clamp<float>(p->AsVec3F()->x, 0.0f, 1.0f) * 255.0f);
					UINT g = UINT(std::clamp<float>(p->AsVec3F()->y, 0.0f, 1.0f) * 255.0f);
					UINT b = UINT(std::clamp<float>(p->AsVec3F()->z, 0.0f, 1.0f) * 255.0f);
					pwp = new CWTFPropertyGridColorProperty(propname, RGB(r, g, b));
					((CWTFPropertyGridColorProperty *)pwp)->EnableOtherButton(_T("Other..."));
					((CWTFPropertyGridColorProperty *)pwp)->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));

					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("RGB Color"));
				}
				else if ((props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4 == p->GetType()) && iscolor)
				{
					UINT r = UINT(std::clamp<float>(p->AsVec4F()->x, 0.0f, 1.0f) * 255.0f);
					UINT g = UINT(std::clamp<float>(p->AsVec4F()->y, 0.0f, 1.0f) * 255.0f);
					UINT b = UINT(std::clamp<float>(p->AsVec4F()->z, 0.0f, 1.0f) * 255.0f);
					UINT a = UINT(std::clamp<float>(p->AsVec4F()->w, 0.0f, 1.0f) * 255.0f);
					pwp = new CWTFPropertyGridColorProperty(propname, RGB(r, g, b));
					((CWTFPropertyGridColorProperty *)pwp)->EnableOtherButton(_T("Other..."));
					((CWTFPropertyGridColorProperty *)pwp)->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));

					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T("RGBA Color"));
				}
				else if ((props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4 == p->GetType()) && (props::IProperty::PROPERTY_ASPECT::PA_QUATERNION == p->GetAspect()))
				{
					const TCHAR *xname = _T("Pitch"), *yname = _T("Roll"), *zname = _T("Yaw");
					glm::fquat q = *((glm::fquat *)p->AsVec4F());

					float _x = glm::degrees(glm::pitch(q));
					if (_x == -0.0f)
						_x = 0.0f;

					float _y = glm::degrees(glm::yaw(q));
					if (_y == -0.0f)
						_y = 0.0f;

					float _z = glm::degrees(glm::roll(q));
					if (_z == -0.0f)
						_z = 0.0f;

					pwp = new CWTFPropertyGridProperty(propname, 0, TRUE);

					pwp->AddSubItem(new CWTFPropertyGridProperty(xname, _x, NULL, NULL, NULL, NULL, _T("0123456789.-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(yname, _y, NULL, NULL, NULL, NULL, _T("0123456789.-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(zname, _z, NULL, NULL, NULL, NULL, _T("0123456789.-")));

					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T(""));
				}
				else
				{
					pwp = new CWTFPropertyGridProperty(propname, 0, TRUE);

					const TCHAR *xname = _T("x"), *yname = _T("y"), *zname = _T("z"), *wname = _T("w");
					switch (p->GetAspect())
					{
						case props::IProperty::PROPERTY_ASPECT::PA_DIMENSIONS:
							xname = _T("Width");
							yname = _T("Length");
							zname = _T("Height");
							wname = _T("Time? Really?");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_RECT:
							xname = _T("X1");
							yname = _T("Y1");
							zname = _T("X2");
							wname = _T("Y2");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_RECT_TLBR:
							xname = _T("Top");
							yname = _T("Left");
							zname = _T("Bottom");
							wname = _T("Right");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG:
						case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD:
							xname = _T("Pitch");
							yname = _T("Roll");
							zname = _T("Yaw");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_LATLON:
							xname = _T("Longitude");
							yname = _T("Lattitude");
							zname = _T("Altitude");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_ELEVAZIM:
							xname = _T("Azimuth");
							yname = _T("Elevation");
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_RASCDEC:
							xname = _T("Right Asecnsion");
							yname = _T("Declination");
							break;
					}

					if (p->Flags().IsSet(props::IProperty::PROPFLAG(props::IProperty::MINMAX)))
					{
						xname = _T("Minimum");
						yname = _T("Maximum");
					}
					else if (p->Flags().IsSet(props::IProperty::PROPFLAG(props::IProperty::INNEROUTER)))
					{
						xname = _T("Inner");
						yname = _T("Outer");
					}


					pwp->AddSubItem(new CWTFPropertyGridProperty(xname, p->AsVec2F()->x, NULL, NULL, NULL, NULL, _T("0123456789.-")));
					pwp->AddSubItem(new CWTFPropertyGridProperty(yname, p->AsVec2F()->y, NULL, NULL, NULL, NULL, _T("0123456789.-")));
					if (p->GetType() >= props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3)
						pwp->AddSubItem(new CWTFPropertyGridProperty(zname, p->AsVec3F()->z, NULL, NULL, NULL, NULL, _T("0123456789.-")));
					if (p->GetType() == props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4)
						pwp->AddSubItem(new CWTFPropertyGridProperty(wname, p->AsVec4F()->w, NULL, NULL, NULL, NULL, _T("0123456789.-")));

					pwp->SetDescription(prop_desc ? prop_desc(p->GetID()) : _T(""));
				}

				break;
			}
		}

		if (reset && pwp)
		{
			pwp->SetData(p->GetID());
			if (!parent_prop)
				AddProperty(pwp, FALSE, FALSE);
			else
				parent_prop->AddSubItem(pwp);
		}
	}

	if (reset)
	{
		AdjustLayout();
	}
}

void CPropertyGrid::OnClickButton(CPoint point)
{
	m_bLocked = true;

	__super::OnClickButton(point);

	m_bLocked = false;
}

BEGIN_MESSAGE_MAP(CPropertyGrid, CWTFPropertyGridCtrl)
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// CPropertyGrid message handlers

void CPropertyGrid::UpdateCurrentProperties()
{
	if (m_bFocused)
		return;

	if (!m_Props)
		return;

	for (int i = 0, maxi = GetPropertyCount(); i < maxi; i++)
	{
		CWTFPropertyGridProperty *pgp = this->GetProperty(i);
		uint32_t id = (uint32_t)pgp->GetData();

		props::IProperty *p = m_Props->GetPropertyById(id);

		if (p)
		{
			bool iscolor =
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR) ||
				(p->GetAspect() == props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR);

			switch (p->GetType())
			{
				case props::IProperty::PROPERTY_TYPE::PT_STRING:
				{
					pgp->UpdateValue(p->AsString());
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_INT:
				{
					switch (p->GetAspect())
					{
						// add other cases if necessary

						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
						case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
						case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:

						default:
							pgp->UpdateValue(p->AsInt());
							break;
					}
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
				{
					pgp->UpdateValue(p->AsFloat());
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_GUID:
				{
					TCHAR guidstr[64];
					p->AsString(guidstr);
					pgp->UpdateValue(guidstr);
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
				{
					pgp->UpdateValue((long)(p->AsBool()));
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
				{
					pgp->GetSubItem(0)->UpdateValue(p->AsVec2I()->x);
					pgp->GetSubItem(1)->UpdateValue(p->AsVec2I()->y);
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
				{
					pgp->GetSubItem(0)->UpdateValue(p->AsVec3I()->x);
					pgp->GetSubItem(1)->UpdateValue(p->AsVec3I()->y);
					pgp->GetSubItem(2)->UpdateValue(p->AsVec3I()->z);
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
				{
					pgp->GetSubItem(0)->UpdateValue(p->AsVec4I()->x);
					pgp->GetSubItem(1)->UpdateValue(p->AsVec4I()->y);
					pgp->GetSubItem(2)->UpdateValue(p->AsVec4I()->z);
					pgp->GetSubItem(3)->UpdateValue(p->AsVec4I()->w);
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
				{
					pgp->GetSubItem(0)->UpdateValue(p->AsVec2F()->x);
					pgp->GetSubItem(1)->UpdateValue(p->AsVec2F()->y);
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
				{
					switch (p->GetAspect())
					{
						case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG:
							pgp->GetSubItem(0)->UpdateValue(glm::degrees(p->AsVec3F()->x));
							pgp->GetSubItem(1)->UpdateValue(glm::degrees(p->AsVec3F()->y));
							pgp->GetSubItem(2)->UpdateValue(glm::degrees(p->AsVec3F()->z));
							break;

						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
						case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
						case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
						{
							UINT r = UINT(std::clamp<float>(p->AsVec3F()->x, 0.0f, 1.0f) * 255.0f);
							UINT g = UINT(std::clamp<float>(p->AsVec3F()->y, 0.0f, 1.0f) * 255.0f);
							UINT b = UINT(std::clamp<float>(p->AsVec3F()->z, 0.0f, 1.0f) * 255.0f);
							((CWTFPropertyGridColorProperty *)pgp)->SetColor(RGB(r, g, b));
							break;
						}

						default:
							pgp->GetSubItem(0)->UpdateValue(p->AsVec3F()->x);
							pgp->GetSubItem(1)->UpdateValue(p->AsVec3F()->y);
							pgp->GetSubItem(2)->UpdateValue(p->AsVec3F()->z);
							break;
					}
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
				{
					switch (p->GetAspect())
					{
						case props::IProperty::PROPERTY_ASPECT::PA_QUATERNION:
						{
							glm::fquat q = *((glm::fquat *)p->AsVec4F());
							float _x = glm::degrees(glm::pitch(q));
							if (_x == -0.0f)
								_x = 0.0f;

							float _y = glm::degrees(glm::yaw(q));
							if (_y == -0.0f)
								_y = 0.0f;

							float _z = glm::degrees(glm::roll(q));
							if (_z == -0.0f)
								_z = 0.0f;

							pgp->GetSubItem(0)->UpdateValue(_x);
							pgp->GetSubItem(1)->UpdateValue(_y);
							pgp->GetSubItem(2)->UpdateValue(_z);
							break;
						}

						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
						case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
						case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
						case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
						{
							UINT r = UINT(std::clamp<float>(p->AsVec4F()->x, 0.0f, 1.0f) * 255.0f);
							UINT g = UINT(std::clamp<float>(p->AsVec4F()->y, 0.0f, 1.0f) * 255.0f);
							UINT b = UINT(std::clamp<float>(p->AsVec4F()->z, 0.0f, 1.0f) * 255.0f);
							UINT a = UINT(std::clamp<float>(p->AsVec4F()->w, 0.0f, 1.0f) * 255.0f);
							((CWTFPropertyGridColorProperty *)pgp)->SetColor(RGBA(r, g, b, a));
							break;
						}

						default:
							pgp->GetSubItem(0)->UpdateValue(p->AsVec4F()->x);
							pgp->GetSubItem(1)->UpdateValue(p->AsVec4F()->y);
							pgp->GetSubItem(2)->UpdateValue(p->AsVec4F()->z);
							pgp->GetSubItem(3)->UpdateValue(p->AsVec4F()->w);
							break;
					}
					break;
				}

				case props::IProperty::PROPERTY_TYPE::PT_ENUM:
				{
					pgp->UpdateValue(p->GetEnumString(p->AsInt()));
					break;
				}
			}
		}
	}
}



void CPropertyGrid::OnPropertyChanged(CWTFPropertyGridProperty* pProp)
{
	if (!m_Props)
		return;

	CWTFPropertyGridCtrl::OnPropertyChanged(pProp);

	uint32_t id = (uint32_t)pProp->GetData();

	props::IProperty *p = m_Props->GetPropertyById(id);
	if (!p && pProp->GetParent())
	{
		pProp = pProp->GetParent();
		id = (uint32_t)pProp->GetData();
		p = m_Props->GetPropertyById(id);
	}

	if (p)
	{
		COleVariant var = pProp->GetValue();

		switch (p->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_STRING:
			{
				CString propstr = var;
				p->SetString(propstr);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT:
			{
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
					{
						COLORREF c = ((CWTFPropertyGridColorProperty *)pProp)->GetColor();
						p->SetInt(c);
						break;
					}

					case props::IProperty::PA_DATE:
					{
						CString propstr = var;
						struct tm t;
						memset(&t, 0, sizeof(struct tm));
						_stscanf_s((LPCTSTR)propstr, _T("%04d/%02d/%02d"), &t.tm_year, &t.tm_mon, &t.tm_mday);
						t.tm_mon--;
						p->SetInt(mktime(&t));
						break;
					}

					case props::IProperty::PA_TIME:
					{
						CString propstr = var;
						struct tm t;
						memset(&t, 0, sizeof(struct tm));
						_stscanf_s((LPCTSTR)propstr, _T("%02d:%02d:%02d"), &t.tm_hour, &t.tm_min, &t.tm_sec);
						p->SetInt(mktime(&t));
						break;
					}

					default:
						p->SetInt(var.llVal);
						break;
				}
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
			{
				p->SetFloat(var.fltVal);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_GUID:
			{
				CString propstr = var;
				p->SetString(propstr);
				p->ConvertTo(props::IProperty::PROPERTY_TYPE::PT_GUID);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
			{
				const TCHAR *truestr;
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_BOOL_YESNO: truestr = _T("Yes"); break;
					case props::IProperty::PROPERTY_ASPECT::PA_BOOL_ONOFF: truestr = _T("On"); break;
					default: truestr = _T("True"); break;
				}
				CString propstr = var;
				p->SetBool(!_tcsicmp(propstr, truestr) ? true : false);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT_V2:
			{
				props::TVec2I v2;
				for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
					v2.v[i] = pProp->GetSubItem(i)->GetValue().llVal;
				p->SetVec2I(v2);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT_V3:
			{
				props::TVec3I v3;
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
					{
						LONG lval = pProp->GetValue().lVal;
						v3.x = lval & 0xff;
						v3.y = (lval >> 8) & 0xff;
						v3.z = (lval >> 16) & 0xff;
						break;
					}

					default:
					{
						for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
							v3.v[i] = pProp->GetSubItem(i)->GetValue().llVal;
						break;
					}
				}
				p->SetVec3I(v3);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_INT_V4:
			{
				props::TVec4I v4;
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
					{
						COLORREF c = ((CWTFPropertyGridColorProperty *)pProp)->GetColor();
						v4.w = (c >> 24) & 0xff;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					{
						COLORREF c = ((CWTFPropertyGridColorProperty *)pProp)->GetColor();
						v4.x = c & 0xff;
						v4.y = (c >> 8) & 0xff;
						v4.z = (c >> 16) & 0xff;
						break;
					}

					default:
					{
						for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
							v4.v[i] = pProp->GetSubItem(i)->GetValue().llVal;
						break;
					}
				}
				p->SetVec4I(v4);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
			{
				props::TVec2F v2;
				for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
					v2.v[i] = pProp->GetSubItem(i)->GetValue().fltVal;
				p->SetVec2F(v2);
				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
			{
				props::TVec3F v3;

				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR:
					case props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR:
					case props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR:
					{
						COLORREF c = ((CWTFPropertyGridColorProperty *)pProp)->GetColor();
						v3.x = (float)(c & 0xFF) / 255.0f;
						v3.y = (float)((c >> 8) & 0xFF) / 255.0f;
						v3.z = (float)((c >> 16) & 0xFF) / 255.0f;
						p->SetVec3F(v3);
						break;
					}

					case props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG:
					{
						for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
							v3.v[i] = glm::radians(pProp->GetSubItem(i)->GetValue().fltVal);
						p->SetVec3F(v3);
						break;
					}

					default:
					{
						for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
							v3.v[i] = pProp->GetSubItem(i)->GetValue().fltVal;
						p->SetVec3F(v3);
						break;
					}
				}

				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
			{
				switch (p->GetAspect())
				{
					case props::IProperty::PA_QUATERNION:
					{
						float _p = glm::radians(pProp->GetSubItem(0)->GetValue().fltVal);
						float _y = glm::radians(pProp->GetSubItem(1)->GetValue().fltVal);
						float _r = glm::radians(pProp->GetSubItem(2)->GetValue().fltVal);

						glm::fquat q(glm::fvec3(_p, _y, -_r));
						q = glm::normalize(q);

						p->SetVec4F(props::TVec4F(q.x, q.y, q.z, q.w));
						break;
					}

					case props::IProperty::PA_COLOR_RGBA:
					case props::IProperty::PA_COLOR_DIFFUSE:
					case props::IProperty::PA_COLOR_EMISSIVE:
					case props::IProperty::PA_COLOR_SPECULAR:
					case props::IProperty::PA_AMBIENT_COLOR:
					case props::IProperty::PA_SUN_COLOR:
					{
						COLORREF c = ((CWTFPropertyGridColorProperty *)pProp)->GetColor();
						props::TVec4F v4;
						v4.x = (float)(c & 0xFF) / 255.0f;
						v4.y = (float)((c >> 8) & 0xFF) / 255.0f;
						v4.z = (float)((c >> 16) & 0xFF) / 255.0f;
						v4.w = (float)((c >> 24) & 0xFF) / 255.0f;
						p->SetVec4F(v4);
						break;
					}

					default:
					{
						props::TVec4F v4;
						for (int i = 0, maxi = pProp->GetSubItemsCount(); i < maxi; i++)
							v4.v[i] = pProp->GetSubItem(i)->GetValue().fltVal;
						p->SetVec4F(v4);
						break;
					}
				}

				break;
			}

			case props::IProperty::PROPERTY_TYPE::PT_ENUM:
			{
				CString propstr = var;
				p->SetEnumValByString(propstr);

				// if the property has a dynamic data provider, then we need to refresh...
				// some properties are dependant upon others. :/
				if (p->GetEnumProvider())
				{
					SetActiveProperties(m_Props);
				}
				break;
			}
		}
	}

	//theApp.GetActiveDocument()->SetModifiedFlag();
}


void CPropertyGrid::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	C3EditFrame *pfrm = (C3EditFrame *)(theApp.GetMainWnd());
	C3EditDoc *pdoc = (C3EditDoc *)(pfrm->GetActiveDocument());
	POSITION vp = pdoc->GetFirstViewPosition();
	C3EditView *pv = (C3EditView *)pdoc->GetNextView(vp);

	switch (nChar)
	{
		case VK_ESCAPE:
			if (m_pSel)
			{
				if (m_bUnfocus)
					pv->SetFocus();
				else
					m_bUnfocus = true;
			}
			break;

		default:
			break;
	}

	CWTFPropertyGridCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CPropertyGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWTFPropertyGridCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CPropertyGrid::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
	{
		if (m_pSel)
		{
			if (m_pSel->IsInPlaceEditing())
			{
				EndEditItem(FALSE);
				SetFocus();
				m_bUnfocus = false;
			}

			return TRUE;
		}
	}

	return CWTFPropertyGridCtrl::PreTranslateMessage(pMsg);
}
