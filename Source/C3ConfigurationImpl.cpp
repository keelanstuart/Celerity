// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3ConfigurationImpl.h>
#include <tinyxml2.h>

using namespace c3;


ConfigurationImpl::ConfigurationImpl(const TCHAR *filename) : m_StringMap()
{
	m_sFilename = filename;

	FILE *f = nullptr;
	if ((_tfopen_s(&f, m_sFilename.c_str(), _T("rb, ccs=UTF-8")) != EINVAL) && f)
	{
		m_ConfigDoc.LoadFile(f);
		fclose(f);
	}
}


ConfigurationImpl::~ConfigurationImpl()
{
	FILE *f = nullptr;
	errno_t err = _tfopen_s(&f, m_sFilename.c_str(), _T("wb, ccs=UTF-8"));
	if ((err != EINVAL) && f)
	{
		m_ConfigDoc.SaveFile(f, false);
		fclose(f);
	}
}


void ConfigurationImpl::Release()
{
	delete this;
}


int64_t ConfigurationImpl::GetInt(const TCHAR *path, int64_t def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("value", def);
			pel->SetAttribute("type", "int");
			return def;
		}

		if (IsType(pel, _T("int")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				return pa->Int64Value();
			}
		}
	}

	return def;
}


void ConfigurationImpl::SetInt(const TCHAR *path, int64_t val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("type", "int");
			pel->SetAttribute("value", val);
			return;
		}

		if (IsType(pel, _T("int")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				pa->SetAttribute(val);
			}
		}
	}
}


COLORREF ConfigurationImpl::GetColor(const TCHAR *path, COLORREF def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			char s[32];
			def = (def & 0xff00ff00) | ((def & 0x00ff0000) >> 16) | ((def & 0x000000ff) << 16);
			sprintf_s(s, sizeof(s), "#%08x", def);

			pel->SetAttribute("value", s);

			pel->SetAttribute("type", "color");
		}

		if (IsType(pel, _T("color")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				COLORREF ret;
				sscanf_s(pa->Value(), "#%08x", &ret);

				ret = (ret & 0xff00ff00) | ((ret & 0x00ff0000) >> 16) | ((ret & 0x000000ff) << 16);
				return ret;
			}
		}
	}

	return def;
}


void ConfigurationImpl::SetColor(const TCHAR *path, COLORREF val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("type", "color");
			pel->SetAttribute("value", "#00000000");
		}

		if (IsType(pel, _T("color")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				char s[32];
				val = (val & 0xff00ff00) | ((val & 0x00ff0000) >> 16) | ((val & 0x000000ff) << 16);
				sprintf_s(s, sizeof(s), "#%08x", val);

				pa->SetAttribute(s);
			}
		}
	}
}


float ConfigurationImpl::GetFloat(const TCHAR *path, float def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("type", "float");
			pel->SetAttribute("value", def);
			return def;
		}

		if (IsType(pel, _T("float")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				return pa->FloatValue();
			}
		}
	}

	return def;
}


void ConfigurationImpl::SetFloat(const TCHAR *path, float val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("value", "0.0");
			pel->SetAttribute("type", "float");
			return;
		}

		if (IsType(pel, _T("float")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				pa->SetAttribute(val);
			}
		}
	}
}


bool ConfigurationImpl::GetBool(const TCHAR *path, bool def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("value", def ? "true" : "false");
			pel->SetAttribute("type", "bool");
			return def;
		}

		if (IsType(pel, _T("bool")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				return (_stricmp(pa->Value(), "true") ? false : true);
			}
		}
	}

	return def;
}


void ConfigurationImpl::SetBool(const TCHAR *path, bool val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("value", "false");
			pel->SetAttribute("type", "bool");
		}

		if (IsType(pel, _T("bool")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
			if (pa)
			{
				pa->SetAttribute(val ? "true" : "false");
			}
		}
	}
}


bool ConfigurationImpl::GetRect(const TCHAR *path, RECT &val, const RECT *def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			char s[32];

			sprintf_s(s, sizeof(s), "%d", def ? def->left : 0);
			pel->SetAttribute("left", s);

			sprintf_s(s, sizeof(s), "%d", def ? def->top : 0);
			pel->SetAttribute("top", s);

			sprintf_s(s, sizeof(s), "%d", def ? def->right : 0);
			pel->SetAttribute("right", s);

			sprintf_s(s, sizeof(s), "%d", def ? def->bottom : 0);
			pel->SetAttribute("bottom", s);

			pel->SetAttribute("type", "rect");
		}

		if (IsType(pel, _T("rect")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("left"));
			if (pa)
			{
				val.left = pa->IntValue();
			}
			pa = GetAttribute(pel, _T("top"));
			if (pa)
			{
				val.top = pa->IntValue();
			}
			pa = GetAttribute(pel, _T("right"));
			if (pa)
			{
				val.right = pa->IntValue();
			}
			pa = GetAttribute(pel, _T("bottom"));
			if (pa)
			{
				val.bottom = pa->IntValue();
			}

			return true;
		}
	}

	return false;
}


void ConfigurationImpl::SetRect(const TCHAR *path, const RECT &val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("left", "0");
			pel->SetAttribute("top", "0");
			pel->SetAttribute("right", "0");
			pel->SetAttribute("bottom", "0");
			pel->SetAttribute("type", "rect");
		}

		if (IsType(pel, _T("rect")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("left"));
			if (pa)
			{
				pa->SetAttribute((int)val.left);
			}
			pa = GetAttribute(pel, _T("top"));
			if (pa)
			{
				pa->SetAttribute((int)val.top);
			}
			pa = GetAttribute(pel, _T("right"));
			if (pa)
			{
				pa->SetAttribute((int)val.right);
			}
			pa = GetAttribute(pel, _T("bottom"));
			if (pa)
			{
				pa->SetAttribute((int)val.bottom);
			}
		}
	}
}


bool ConfigurationImpl::GetPoint(const TCHAR *path, POINT &val, const POINT *def)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			char s[32];

			sprintf_s(s, sizeof(s), "%d", def ? def->x : 0);
			pel->SetAttribute("x", s);

			sprintf_s(s, sizeof(s), "%d", def ? def->y : 0);
			pel->SetAttribute("y", s);

			pel->SetAttribute("type", "point");
		}

		if (IsType(pel, _T("point")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("x"));
			if (pa)
			{
				val.x = pa->IntValue();
			}
			pa = GetAttribute(pel, _T("y"));
			if (pa)
			{
				val.y = pa->IntValue();
			}

			return true;
		}
	}

	return false;
}


void ConfigurationImpl::SetPoint(const TCHAR *path, const POINT &val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("x", "0");
			pel->SetAttribute("y", "0");
			pel->SetAttribute("type", "point");
		}

		if (IsType(pel, _T("point")))
		{
			tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("x"));
			if (pa)
			{
				pa->SetAttribute((int)val.x);
			}
			pa = GetAttribute(pel, _T("y"));
			if (pa)
			{
				pa->SetAttribute((int)val.y);
			}
		}
	}
}


const TCHAR *ConfigurationImpl::GetString(const TCHAR *path, const TCHAR *def)
{
	bool created;
	bool should_create = true;
	if (_tcschr(path, _T('#')) && !def)
		should_create = false;

	tinyxml2::XMLElement *pel = EvaluatePath(path, should_create, created);

	if (pel)
	{
		TXmlEl2StrMap::iterator it = m_StringMap.find(pel);
		if (it == m_StringMap.end())
		{
			m_StringMap.insert(TXmlEl2StrPair(pel, tstring(def ? def : _T(""))));
			it = m_StringMap.find(pel);
		}

		if (created)
		{
			pel->SetAttribute("type", "string");

			tinyxml2::XMLText *pt = (tinyxml2::XMLText *)pel->FirstChild();
			if (!pt)
			{
				char *tmpdef;
				CONVERT_TCS2MBCS(def, tmpdef);
				tinyxml2::XMLText *tmp = m_ConfigDoc.NewText(tmpdef);
				pt = (tinyxml2::XMLText *)pel->InsertEndChild(tmp);
			}
		}

		if (IsType(pel, _T("string")))
		{
			const char *tmpret = pel->GetText();
			if (!tmpret)
			{
				tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("value"));
				if (pa)
				{
					tmpret = pa->Value();
				}
			}

			if (tmpret)
			{
				TCHAR *ret;
				CONVERT_MBCS2TCS(tmpret, ret);
				assert(it != m_StringMap.end());
				it->second = ret;

				return it->second.c_str();
			}
		}
	}

	return def;
}


void ConfigurationImpl::SetString(const TCHAR *path, const TCHAR *val)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		if (created)
		{
			pel->SetAttribute("type", "string");
		}

		if (IsType(pel, _T("string")))
		{
			tinyxml2::XMLText *pt = (tinyxml2::XMLText *)pel->FirstChild();
			if (!pt)
			{
				tinyxml2::XMLText *tmp = m_ConfigDoc.NewText("");
				pt = (tinyxml2::XMLText *)pel->InsertEndChild(tmp);
			}

			if (pt)
			{
				char *tmpval;
				CONVERT_TCS2MBCS(val, tmpval);
				pt->SetValue(tmpval);

				TXmlEl2StrMap::iterator it = m_StringMap.find(pel);
				if (it == m_StringMap.end())
				{
					m_StringMap.insert(TXmlEl2StrPair(pel, tstring(val ? val : _T(""))));
				}
				else
				{
					it->second = val ? val : _T("");
				}
			}
		}
	}
}


tinyxml2::XMLElement *ConfigurationImpl::EvaluatePath(const TCHAR *path, bool create, bool &didcreate)
{
	didcreate = false;

	tinyxml2::XMLElement *ret = m_ConfigDoc.RootElement();
	if (!ret)
	{
		tinyxml2::XMLElement *root = m_ConfigDoc.NewElement("configuration");
		ret = (tinyxml2::XMLElement *)m_ConfigDoc.InsertEndChild(root);

		if (!ret)
		{
			return NULL;
		}
	}

	const TCHAR *allowed_chars = _T("_-:{}");

	bool first_char = true;
	tstring pathcomp;
	while (path && *path && ret)
	{
		if ((!first_char && isalnum(*path)) || isalpha(*path) || _tcschr(allowed_chars, *path))
		{
			first_char = false;
			pathcomp += *path;
		}

		path++;

		size_t tagidx = 0;

		if (*path == _T('#'))
		{
			tstring idxstr;
			path++;
			while (*path && isalnum(*path) && !isalpha(*path))
			{
				idxstr += *path;
				path++;
			}

			tagidx = _ttoi(idxstr.c_str());
		}

		if ((!*path || !(isalnum(*path) || _tcschr(allowed_chars, *path))) && !pathcomp.empty())
		{
			char *elstr;
			CONVERT_TCS2MBCS(pathcomp.c_str(), elstr);

			tinyxml2::XMLElement *tmpret = ret->FirstChildElement(elstr);
			while ((tagidx > 0) && tmpret)
			{
				tinyxml2::XMLElement *next = tmpret->NextSiblingElement(elstr);
				if (!next)
				{
					tinyxml2::XMLElement *dummy = m_ConfigDoc.NewElement(elstr);
					next = (tinyxml2::XMLElement *)ret->InsertAfterChild(tmpret, dummy);
				}

				tagidx--;
				tmpret = next;
			}

			if (!tmpret && create)
			{
				std::transform(pathcomp.begin(), pathcomp.end(), pathcomp.begin(), tolower);

				tinyxml2::XMLElement *dummy = m_ConfigDoc.NewElement(elstr);
				if (!dummy)
					return nullptr;

				tmpret = (tinyxml2::XMLElement *)ret->InsertEndChild(dummy);
				didcreate = true;
			}

			ret = tmpret;

			pathcomp.clear();
			first_char = true;
		}
	}

	return ret;
}


tinyxml2::XMLAttribute *ConfigurationImpl::GetAttribute(const tinyxml2::XMLElement *pel, const TCHAR *name)
{
	char *_name;
	CONVERT_TCS2MBCS(name, _name);

	tinyxml2::XMLAttribute *pa = (tinyxml2::XMLAttribute *)pel->FirstAttribute();
	while (pa)
	{
		if (!_stricmp(pa->Name(), _name))
		{
			return pa;
		}

		pa = (tinyxml2::XMLAttribute *)pa->Next();
	}

	return NULL;
}

bool ConfigurationImpl::IsType(const tinyxml2::XMLElement *pel, const TCHAR *type)
{
	char *_type;
	CONVERT_TCS2MBCS(type, _type);

	tinyxml2::XMLAttribute *pa = GetAttribute(pel, _T("type"));
	if (pa)
	{
		if (!_stricmp(pa->Value(), _type))
			return true;
	}

	return false;
}

size_t ConfigurationImpl::GetNumSubKeys(const TCHAR *path)
{
	size_t ret = 0;

	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		tinyxml2::XMLElement *pchild = pel->FirstChildElement();
		while (pchild)
		{
			ret++;

			pchild = pchild->NextSiblingElement();
		}
	}

	return ret;
}

bool ConfigurationImpl::GetSubKeyName(const TCHAR *path, size_t subkeyindex, TCHAR *retbuf, size_t retbuflen)
{
	bool ret = false;

	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, true, created);

	if (pel)
	{
		tinyxml2::XMLElement *pchild = pel->FirstChildElement();

		for (size_t i = 0; i < subkeyindex; i++)
		{
			if (pchild)
				pchild = pel->NextSiblingElement();
			else
				return false;
		}

		if (pchild)
		{
			TCHAR *val;
			CONVERT_MBCS2TCS(pchild->Value(), val);

			_tcsncpy_s(retbuf, retbuflen, val, retbuflen);
			ret = true;
		}
	}

	return ret;
}

bool ConfigurationImpl::RemoveKey(const TCHAR *path)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, false, created);
	if (pel)
	{
		TXmlEl2StrMap::const_iterator it = m_StringMap.find(pel);
		if (it != m_StringMap.end())
			m_StringMap.erase(it);

		m_ConfigDoc.DeleteNode(pel);
	}

	return (pel != nullptr);
}

bool ConfigurationImpl::RemoveSubKey(const TCHAR *path, size_t subkeyindex)
{
	bool created;
	tinyxml2::XMLElement *pel = EvaluatePath(path, false, created);

	if (pel)
	{
		tinyxml2::XMLElement *pchild = pel->FirstChildElement();

		for (size_t i = 0; i < subkeyindex; i++)
		{
			if (pchild)
				pchild = pel->NextSiblingElement();
			else
				return false;
		}

		if (pchild)
		{
			TXmlEl2StrMap::const_iterator it = m_StringMap.find(pel);
			if (it != m_StringMap.end())
				m_StringMap.erase(it);

			m_ConfigDoc.DeleteNode(pel);

			return true;
		}
	}

	return false;
}
