// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

namespace c3
{

	class Configuration
	{

	public:

		virtual void Release() = NULL;

		virtual bool GetBool(const TCHAR *path, bool def) = NULL;
		virtual void SetBool(const TCHAR *path, bool val) = NULL;

		virtual COLORREF GetColor(const TCHAR *path, COLORREF def) = NULL;
		virtual void SetColor(const TCHAR *path, COLORREF val) = NULL;

		virtual float GetFloat(const TCHAR *path, float def) = NULL;
		virtual void SetFloat(const TCHAR *path, float val) = NULL;

		virtual int64_t GetInt(const TCHAR *path, int64_t def) = NULL;
		virtual void SetInt(const TCHAR *path, int64_t val) = NULL;

		virtual bool GetPoint(const TCHAR *path, POINT &val, const POINT *def = NULL) = NULL;
		virtual void SetPoint(const TCHAR *path, const POINT &val) = NULL;

		virtual bool GetRect(const TCHAR *path, RECT &val, const RECT *def = NULL) = NULL;
		virtual void SetRect(const TCHAR *path, const RECT &def) = NULL;

		virtual const TCHAR *GetString(const TCHAR *path, const TCHAR *def) = NULL;
		virtual void SetString(const TCHAR *path, const TCHAR *val) = NULL;

		virtual size_t GetNumSubKeys(const TCHAR *path) = NULL;
		virtual bool GetSubKeyName(const TCHAR *path, size_t subkeyindex, TCHAR *retbuf, size_t retbuflen) = NULL;

		virtual bool RemoveKey(const TCHAR *path) = NULL;
		virtual bool RemoveSubKey(const TCHAR *path, size_t subkeyindex) = NULL;

	};

};