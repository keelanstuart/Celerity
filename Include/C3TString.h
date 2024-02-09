// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <tchar.h>
#include <string>
#include <malloc.h>


typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstring;
typedef std::basic_ios<TCHAR, std::char_traits<TCHAR>> tios;
typedef std::basic_streambuf<TCHAR, std::char_traits<TCHAR>> tstreambuf;
typedef std::basic_istream<TCHAR, std::char_traits<TCHAR>> tistream;
typedef std::basic_ostream<TCHAR, std::char_traits<TCHAR>> tostream;
typedef std::basic_iostream<TCHAR, std::char_traits<TCHAR>> tiostream;
typedef std::basic_stringbuf<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> tstringbuf;
typedef std::basic_istringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> tistringstream;
typedef std::basic_ostringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> tostringstream;
typedef std::basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> tstringstream;
typedef std::basic_filebuf<TCHAR, std::char_traits<TCHAR>> tfilebuf;
typedef std::basic_ifstream<TCHAR, std::char_traits<TCHAR>> tifstream;
typedef std::basic_ofstream<TCHAR, std::char_traits<TCHAR>> tofstream;
typedef std::basic_fstream<TCHAR, std::char_traits<TCHAR>> tfstream;


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
