// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	// Macros to use in plugin libraries to create and export relevant functions

	#define CELERITYPLUGIN_NAME(s)				extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetName() { return _T(s); } }
	#define CELERITYPLUGIN_SHORTNAME(s)			extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetShortName() { return _T(s); } }
	#define CELERITYPLUGIN_AUTHOR(s)			extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetAuthor() { return _T(s); } }
	#define CELERITYPLUGIN_WEBSITE(s)			extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetWebsite() { return _T(s); } }
	#define CELERITYPLUGIN_DESC(s)				extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetDescription() { return _T(s); } }
	#define CELERITYPLUGIN_COPYRIGHT(s)			extern "C" { const __declspec(dllexport) TCHAR *CelerityPlugin_GetCopyright() { return _T(s); } }
	#define CELERITYPLUGIN_VERSION(n)			extern "C" { uint32_t __declspec(dllexport) CelerityPlugin_GetPluginVersion() { return n; } }
	#define CELERITYPLUGIN_CELERITYVERSION()	extern "C" { uint32_t __declspec(dllexport) CelerityPlugin_GetCelerityVersion() { return CELERITY_VERSION; } }

	// The Activate and Deactivate functions are meant to provide your plug-in with a way of allocating
	// or freeing system resources. Client applications may choose to demand load / unload your
	// code, so this it is essential for "good behavior" that you use these appropriately.
	#define CELERITYPLUGIN_ONACTIVATE_BEGIN()	extern "C" { bool __declspec(dllexport) CelerityPlugin_Activate(c3::System *psys) {
	#define CELERITYPLUGIN_ONACTIVATE_END()		} }

	#define CELERITYPLUGIN_ONDEACTIVATE_BEGIN()	extern "C" { bool __declspec(dllexport) CelerityPlugin_Deactivate(c3::System *psys) {
	#define CELERITYPLUGIN_ONDEACTIVATE_END()	} }

	// Use these functions like this in your original plug-in code:

	/*
	C3PLUGIN_NAME("MyPlugin");
	C3PLUGIN_AUTHOR("Me");
	C3PLUGIN_DESC("This does all the things I want");
	C3PLUGIN_COPYRIGHT("2020 Me, All Rights Reserved");

	C3PLUGIN_ONACTIVATE_BEGIN()
		// example activation code...
		::MessageBox("blah", "blah", MB_OK);
	C3PLUGIN_ONACTIVATE_END()

	int dllmain()
	{
		....
		return 0;
	}
	*/


	/// The Plugin information class
	class Plugin
	{

	public:

		enum RETURNCODE
		{
			RET_OK = 0,
			RET_NOACTIVATE,
			RET_NODEACTIVATE
		};

		virtual const TCHAR *GetFilename() = NULL;

		virtual const TCHAR *GetName() = NULL;

		virtual const TCHAR *GetShortName() = NULL;

		virtual const TCHAR *GetAuthor() = NULL;

		virtual const TCHAR *GetCopyright() = NULL;

		virtual const TCHAR *GetWebsite() = NULL;

		virtual const TCHAR *GetDescription() = NULL;

		virtual UINT32 GetPluginVersion() = NULL;

		virtual RETURNCODE Activate() = NULL;

		virtual RETURNCODE Deactivate() = NULL;

		virtual bool Active() = NULL;

		virtual bool Loaded() = NULL;

	};

};
