// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3Configuration.h>

namespace c3
{

	class ConfigurationImpl : public Configuration
	{

	protected:
		tstring m_sFilename;

		tinyxml2::XMLDocument m_ConfigDoc;

		tinyxml2::XMLElement *EvaluatePath(const TCHAR *path, bool create, bool &didcreate);

		tinyxml2::XMLAttribute *GetAttribute(const tinyxml2::XMLElement *pel, const TCHAR *name);

		bool IsType(const tinyxml2::XMLElement *pel, const TCHAR *type);

		typedef std::map<tinyxml2::XMLElement *, tstring> TXmlEl2StrMap;
		typedef std::pair<tinyxml2::XMLElement *, tstring> TXmlEl2StrPair;
		TXmlEl2StrMap m_StringMap;

	public:

		ConfigurationImpl(const TCHAR *filename);

		virtual ~ConfigurationImpl();

		virtual void Release();

		virtual bool GetBool(const TCHAR *path, bool def);
		virtual void SetBool(const TCHAR *path, bool val);

		virtual COLORREF GetColor(const TCHAR *path, COLORREF def);
		virtual void SetColor(const TCHAR *path, COLORREF val);

		virtual float GetFloat(const TCHAR *path, float def);
		virtual void SetFloat(const TCHAR *path, float val);

		virtual int64_t GetInt(const TCHAR *path, int64_t def);
		virtual void SetInt(const TCHAR *path, int64_t val);

		virtual bool GetPoint(const TCHAR *path, POINT &val, const POINT *def);
		virtual void SetPoint(const TCHAR *path, const POINT &val);

		virtual bool GetRect(const TCHAR *path, RECT &val, const RECT *def);
		virtual void SetRect(const TCHAR *path, const RECT &def);

		virtual const TCHAR *GetString(const TCHAR *path, const TCHAR *def);
		virtual void SetString(const TCHAR *path, const TCHAR *val);

		virtual size_t GetNumSubKeys(const TCHAR *path);
		virtual bool GetSubKeyName(const TCHAR *path, size_t subkeyindex, TCHAR *retbuf, size_t retbuflen);

		virtual bool RemoveKey(const TCHAR *path);
		virtual bool RemoveSubKey(const TCHAR *path, size_t subkeyindex);

	};

};
