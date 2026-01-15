// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3PluginImpl.h>

#include <Shlwapi.h>


using namespace c3;


PluginImpl::PluginImpl(const TCHAR *filename, System *psys)
{
	m_pSys = psys;

	m_Filename = filename;

	m_hMod = NULL;
	m_bActive = false;

	m_fnGetName = nullptr;
	m_fnGetShortName = nullptr;
	m_fnGetPluginVersion = nullptr;
	m_fnGetAuthor = nullptr;
	m_fnGetWebsite = nullptr;
	m_fnGetDescription = nullptr;
	m_fnGetShortDescription = nullptr;
	m_fnGetCopyright = nullptr;
	m_fnActivate = nullptr;
	m_fnDeactivate = nullptr;

	if (m_pSys)
		m_pSys->GetLog()->Print(_T("Loading Plug-In: \"%s\" ... "), m_Filename.c_str());

	if (PathFileExists(m_Filename.c_str()))
		m_hMod = LoadLibraryEx(m_Filename.c_str(), NULL, 0);

	if (m_hMod)
	{
		bool invalid = false;

		PLUGFUNC_GETCELERITYVERSION fnCelerityVersion = (PLUGFUNC_GETCELERITYVERSION)GetProcAddress(m_hMod, "CelerityPlugin_GetCelerityVersion");
		if (fnCelerityVersion)
		{
			uint32_t celerity_version_that_plugin_expects = fnCelerityVersion();
			uint32_t ver_maj = celerity_version_that_plugin_expects >> 24;

			if (CELERITY_VERSION_MAJ != ver_maj)
				invalid = true;
		}

		if (invalid)
		{
			m_pSys->GetLog()->Print(_T("failure (different major version of Celerity)\n"));
			FreeLibrary(m_hMod);

			m_hMod = NULL;

			return;
		}

		m_fnGetName = (PLUGFUNC_GETNAME)GetProcAddress(m_hMod, "CelerityPlugin_GetName");
		m_fnGetShortName = (PLUGFUNC_GETSHORTNAME)GetProcAddress(m_hMod, "CelerityPlugin_GetShortName");
		m_fnGetPluginVersion = (PLUGFUNC_GETVERSION)GetProcAddress(m_hMod, "CelerityPlugin_GetPluginVersion");
		m_fnGetAuthor = (PLUGFUNC_GETAUTHOR)GetProcAddress(m_hMod, "CelerityPlugin_GetAuthor");
		m_fnGetWebsite = (PLUGFUNC_GETWEBSITE)GetProcAddress(m_hMod, "CelerityPlugin_GetWebsite");
		m_fnGetDescription = (PLUGFUNC_GETDESC)GetProcAddress(m_hMod, "CelerityPlugin_GetDescription");
		m_fnGetShortDescription = (PLUGFUNC_GETSHORTDESC)GetProcAddress(m_hMod, "CelerityPlugin_GetShortDescription");
		m_fnGetCopyright = (PLUGFUNC_GETCOPYRIGHT)GetProcAddress(m_hMod, "CelerityPlugin_GetCopyright");
		m_fnActivate = (PLUGFUNC_ACTIVATE)GetProcAddress(m_hMod, "CelerityPlugin_Activate");
		m_fnDeactivate = (PLUGFUNC_DEACTIVATE)GetProcAddress(m_hMod, "CelerityPlugin_Deactivate");
	}
	else
	{
		m_pSys->GetLog()->Print(_T("failure (DLL load failure)\n"));
		return;
	}

	m_pSys->GetLog()->Print(_T("success\n"));
}


PluginImpl::~PluginImpl()
{
	if (m_hMod)
		FreeLibrary(m_hMod);

	m_hMod = NULL;
	m_bActive = false;

	m_fnGetName = nullptr;
	m_fnGetShortName = nullptr;
	m_fnGetAuthor = nullptr;
	m_fnGetWebsite = nullptr;
	m_fnGetDescription = nullptr;
	m_fnGetShortDescription = nullptr;
	m_fnGetCopyright = nullptr;
	m_fnGetPluginVersion = nullptr;
	m_fnActivate = nullptr;
	m_fnDeactivate = nullptr;
}


const TCHAR *PluginImpl::GetFilename()
{
	return m_Filename.c_str();
}


const TCHAR *PluginImpl::GetName()
{
	if (m_fnGetName)
		return m_fnGetName();

	return _T("");
}


const TCHAR *PluginImpl::GetShortName()
{
	if (m_fnGetShortName)
		return m_fnGetShortName();

	return _T("");
}


const TCHAR *PluginImpl::GetAuthor()
{
	if (m_fnGetAuthor)
		return m_fnGetAuthor();

	return _T("");
}


const TCHAR *PluginImpl::GetCopyright()
{
	if (m_fnGetCopyright)
		return m_fnGetCopyright();

	return _T("");
}


const TCHAR *PluginImpl::GetWebsite()
{
	if (m_fnGetWebsite)
		return m_fnGetWebsite();

	return _T("");
}


const TCHAR *PluginImpl::GetDescription()
{
	if (m_fnGetDescription)
		return m_fnGetDescription();

	return _T("");
}


const TCHAR *PluginImpl::GetShortDescription()
{
	if (m_fnGetShortDescription)
		return m_fnGetShortDescription();

	return _T("");
}


uint32_t PluginImpl::GetPluginVersion()
{
	if (m_fnGetPluginVersion)
		return m_fnGetPluginVersion();

	return 0;
}


Plugin::RETURNCODE PluginImpl::Activate()
{
	m_bActive = (m_fnActivate != nullptr) ? m_fnActivate(m_pSys) : true;

	return m_bActive ? RET_OK : RET_NOACTIVATE;
}


Plugin::RETURNCODE PluginImpl::Deactivate()
{
	m_bActive = (m_fnDeactivate != nullptr) ? !m_fnDeactivate(m_pSys) : false;

	return !m_bActive ? RET_OK : RET_NODEACTIVATE;
}

bool PluginImpl::Active()
{
	return m_bActive;
}

bool PluginImpl::Loaded()
{
	return (m_hMod != NULL) ? true : false;
}
