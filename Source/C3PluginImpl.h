// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3Plugin.h>

namespace c3
{

	class PluginImpl : public Plugin
	{

	protected:

		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETNAME)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETSHORTNAME)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETAUTHOR)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETWEBSITE)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETDESC)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETSHORTDESC)();
		typedef const TCHAR *const (__cdecl *PLUGFUNC_GETCOPYRIGHT)();
		typedef uint32_t(__cdecl *PLUGFUNC_GETVERSION)();
		typedef uint32_t(__cdecl *PLUGFUNC_GETCELERITYVERSION)();
		typedef uint32_t(__cdecl *PLUGFUNC_GETPLATFORM)();
		typedef bool(__cdecl *PLUGFUNC_ISUNICODE)();
		typedef bool(__cdecl *PLUGFUNC_ACTIVATE)(System *psys);
		typedef bool(__cdecl *PLUGFUNC_DEACTIVATE)(System *psys);

		System *m_pSys;

		HMODULE m_hMod;
		bool m_bActive;

		tstring m_Filename;

		PLUGFUNC_GETNAME m_fnGetName;
		PLUGFUNC_GETSHORTNAME m_fnGetShortName;
		PLUGFUNC_GETVERSION m_fnGetPluginVersion;
		PLUGFUNC_GETAUTHOR m_fnGetAuthor;
		PLUGFUNC_GETWEBSITE m_fnGetWebsite;
		PLUGFUNC_GETDESC m_fnGetDescription;
		PLUGFUNC_GETSHORTDESC m_fnGetShortDescription;
		PLUGFUNC_GETCOPYRIGHT m_fnGetCopyright;
		PLUGFUNC_ACTIVATE m_fnActivate;
		PLUGFUNC_DEACTIVATE m_fnDeactivate;


	public:

		PluginImpl(const TCHAR *filename, System *psys);
		virtual ~PluginImpl();

		virtual const TCHAR *GetFilename();

		virtual const TCHAR *GetName();

		virtual const TCHAR *GetShortName();

		virtual const TCHAR *GetAuthor();

		virtual const TCHAR *GetCopyright();

		virtual const TCHAR *GetWebsite();

		virtual const TCHAR *GetDescription();

		virtual const TCHAR *GetShortDescription();

		virtual uint32_t GetPluginVersion();

		virtual RETURNCODE Activate();

		virtual RETURNCODE Deactivate();

		virtual bool Active();

		virtual bool Loaded();

	};

};
