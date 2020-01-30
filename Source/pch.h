// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <tchar.h>

#include <map>
#include <deque>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

#define C3_PI		3.14159f

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstring;
typedef std::basic_ostream<TCHAR, std::char_traits<TCHAR> > tostream;
typedef std::basic_istream<TCHAR, std::char_traits<TCHAR> > tistream;

#define CONVERT_WCS2MBCS(wcs, mbcs) if (wcs) {			\
  size_t origsize = _tcslen(wcs) + 1;					\
  size_t newsize = (origsize * 2) * sizeof(char);		\
  mbcs = (char *)_alloca(newsize);						\
  size_t retval = 0;									\
  wcstombs_s(&retval, mbcs, newsize, wcs, newsize); }	\
  else mbcs = nullptr;

#define CONVERT_MBCS2WCS(mbcs, wcs) if (mbcs) {			\
  size_t origsize = strlen(mbcs) + 1;					\
  size_t newsize = origsize * sizeof(wchar_t);			\
  wcs = (wchar_t *)_alloca(newsize);					\
  size_t retval = 0;									\
  mbstowcs_s(&retval, wcs, origsize, mbcs, newsize); }	\
  else wcs = nullptr;

#if defined(_UNICODE) || defined(UNICODE)

#define CONVERT_TCS2MBCS(tcs, mbcs) CONVERT_WCS2MBCS(tcs, mbcs)

#define CONVERT_TCS2WCS(tcs, wcs) wcs = (wchar_t *)tcs;

#define CONVERT_MBCS2TCS(mbcs, tcs) CONVERT_MBCS2WCS(mbcs, tcs)

#define CONVERT_WCS2TCS(wcs, tcs) tcs = (TCHAR *)wcs;

#else

#define CONVERT_TCS2MBCS(tcs, mbcs) mbcs = (char *)tcs;

#define CONVERT_TCS2WCS(tcs, wcs) CONVERT_MBCS2WCS(tcs, wcs)

#define CONVERT_MBCS2TCS(mbcs, tcs) tcs = (TCHAR *)mbcs;

#define CONVERT_WCS2TCS(tcs, wcs) CONVERT_WCS2MBCS(wcs, tcs)

#endif


#include <C3.h>

#include <imgui.h>

#endif //PCH_H
