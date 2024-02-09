// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#include "pch.h"

#include <C3Plugin.h>


CELERITYPLUGIN_NAME("Sample");
CELERITYPLUGIN_SHORTNAME("CelerityExamplePlugin");
CELERITYPLUGIN_AUTHOR("Keelan Stuart");
CELERITYPLUGIN_WEBSITE("https://github.com/keelanstuart/Celerity");
CELERITYPLUGIN_DESC("A simple plug-in for Celerity 3.x that demonstrates how to implement your own Component and Resource types");
CELERITYPLUGIN_COPYRIGHT("Copyright © 2001-2024, Keelan Stuart");
CELERITYPLUGIN_VERSION(1);
CELERITYPLUGIN_CELERITYVERSION();


// The Activate and Deactivate functions are meant to provide your plug-in with a way of allocating
// or freeing system resources. Client applications may choose to demand load / unload your
// code, so this it is essential for "good behavior" that you use these appropriately.

CELERITYPLUGIN_ONACTIVATE_BEGIN()
	psys->GetLog()->Print(_T("Activating C3SamplePlugin!\n"));

	// Register Component and Resource types

	return true;
CELERITYPLUGIN_ONACTIVATE_END()

CELERITYPLUGIN_ONDEACTIVATE_BEGIN()
	psys->GetLog()->Print(_T("Deactivating C3SamplePlugin!\n"));

	// Unregister Component and Resource types

	return true;
CELERITYPLUGIN_ONDEACTIVATE_END()



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }

	return TRUE;
}

