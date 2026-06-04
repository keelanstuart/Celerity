// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <Windows.h>
#include <shlwapi.h>

#include <tchar.h>

#include <map>
#include <deque>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <locale>
#include <codecvt>
#include <functional>


//#define GLLIBGEN_LOGCALLS

extern HMODULE g_C3Mod;


#include <C3.h>

#define IMGUI_USER_CONFIG	<C3ImGuiConfig.h>
#include <../third-party/imgui/imgui.h>

// a used in std::*_convert operations
template <typename Facet>
struct deletable_facet : Facet
{
	using Facet::Facet;
};

#endif //PCH_H
