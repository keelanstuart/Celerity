// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3PluginManager.h>

namespace c3
{

	class PluginManagerImpl : public PluginManager
	{

	protected:
		System *m_pSys;

		typedef std::vector<Plugin *> TPluginArray;
		TPluginArray m_Plugins;

	public:

		PluginManagerImpl(System *psys);

		virtual ~PluginManagerImpl();

		virtual RETURNCODE DiscoverPlugins(const TCHAR *path, const TCHAR *filespec, bool auto_activate, size_t *numfound);

		virtual RETURNCODE UnloadPlugin(Plugin *pplug);

		virtual void UnloadAllPlugins();

		virtual size_t GetNumPlugins();

		virtual Plugin *GetPlugin(size_t idx);

	};

};