// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3FileMapper.h>

namespace c3
{

	class FileMapperImpl : public FileMapper
	{

	protected:
		System *m_pSys;

		typedef std::multimap< tstring, tstring, std::less<tstring> > TExtToPathMap;
		TExtToPathMap m_mapExts;
		size_t m_nUniqueExts;

	public:
		FileMapperImpl(System *psys);
		virtual ~FileMapperImpl();

		virtual size_t GetNumMappings();

		virtual const TCHAR *GetMapping(size_t mappingidx);

		virtual bool FindMapping(const TCHAR *extension, size_t *idx);

		virtual size_t GetNumPaths(const TCHAR *extension);

		virtual const TCHAR *GetPath(const TCHAR *extension, size_t pathidx);

		virtual void AddMapping(const TCHAR *extension, const TCHAR *path);

		virtual void RemoveMapping(const TCHAR *extension, const TCHAR *path);

		virtual bool FindFile(const TCHAR *filename, TCHAR *fullpath, size_t fullpathlen);

		virtual void SetMappingsFromDelimitedStrings(const TCHAR *exts, const TCHAR *paths, const TCHAR delimiter);

	};

};