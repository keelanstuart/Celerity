// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Plugin.h>

namespace c3
{

	class PluginManager
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_BADFILESPEC,
			RET_NOTFOUND
		};

		#if defined(DEBUG)
			#define DEFAULT_CELERITY_PLUGIN_EXT			_T("*.dcll")
		#else
			#define DEFAULT_CELERITY_PLUGIN_EXT			_T("*.cll")
		#endif

		/// Discovers plug-ins matching the given filespec and can auto-activate them if desired
		virtual RETURNCODE DiscoverPlugins(const TCHAR *filespec = DEFAULT_CELERITY_PLUGIN_EXT, bool auto_activate = true, size_t *numfound = nullptr) = NULL;

		/// Unloads a specific plug-in
		virtual RETURNCODE UnloadPlugin(Plugin *pplug) = NULL;

		/// Unloads all plug-ins
		virtual void UnloadAllPlugins() = NULL;

		/// Returns the number of plug-ins that were successfully loaded
		virtual size_t GetNumPlugins() = NULL;

		/// Returns an interface to a plug-in, given its index
		virtual Plugin *GetPlugin(size_t idx) = NULL;

	};

};