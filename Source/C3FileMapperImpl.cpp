// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3FileMapperImpl.h>
#include <Shlwapi.h>


using namespace c3;


FileMapperImpl::FileMapperImpl(System *psys)
{
	m_pSys = psys;
	m_nUniqueExts = 0;
}


FileMapperImpl::~FileMapperImpl()
{
	m_mapExts.clear();
	m_nUniqueExts = 0;
}


// Returns the number of unique extension maps you have registered
size_t FileMapperImpl::GetNumMappings()
{
	return m_nUniqueExts;
}


// Returns information about the mapping at the given index
const TCHAR *FileMapperImpl::GetMapping(size_t mappingidx)
{
	uint32_t k = 0;
	TExtToPathMap::const_iterator i = m_mapExts.begin();
	TExtToPathMap::const_iterator e = m_mapExts.end();
	tstring ext;

	if (i != e)
	{
		ext = i->first;
		i++;
	}

	while ((k < mappingidx) && (i != e))
	{
		if (!(ext == i->first))
		{
			ext = i->first;
			k++;
		}
	}

	if (i != e)
	{
		return i->first.c_str();
	}

	return NULL;
}


// Finds the index of the mapping for the given extension
bool FileMapperImpl::FindMapping(const TCHAR *extension, size_t *idx)
{
	if (!extension)
		return false;

	TExtToPathMap::const_iterator i = m_mapExts.begin();
	TExtToPathMap::const_iterator e = m_mapExts.end();

	if (i == e)
		return false;

	tstring ext = extension;
	tstring tmp = i->first;
	int32_t k = 0;

	while (i != e)
	{
		if (ext == tmp)
		{
			*idx = k;
			return true;
		}

		if (!(tmp == i->first))
		{
			tmp = i->first;
			k++;
		}

		i++;
	}

	return false;
}


// Gets the number of paths that have been registered for the given extension
size_t FileMapperImpl::GetNumPaths(const TCHAR *extension)
{
	if (!extension)
		return NULL;

	return m_mapExts.count(extension);
}


// Gets the path at the given index for the given extension
const TCHAR *FileMapperImpl::GetPath(const TCHAR *extension, size_t pathidx)
{
	if (!extension)
		return NULL;

	TExtToPathMap::const_iterator i = m_mapExts.lower_bound(extension);
	TExtToPathMap::const_iterator e = m_mapExts.upper_bound(extension);

	if (i == e)
		return NULL;

	uint32_t k = 0;

	while ((i != e) && (k < pathidx))
	{
		i++;
		k++;
	}

	if (i != e)
		return i->second.c_str();

	return NULL;
}


// Add the path to the mapping for whatever extension you want
void FileMapperImpl::AddMapping(const TCHAR *extension, const TCHAR *path)
{
	if (extension && path)
	{
		TExtToPathMap::const_iterator i = m_mapExts.lower_bound(extension);
		TExtToPathMap::const_iterator e = m_mapExts.upper_bound(extension);

		if (i == e)
		{
			m_nUniqueExts++;
		}

		// search for the mapping in question...
		while ((i != e) && (i->second != path))
		{
			i++;
		}

		if (i == e)
		{
			TExtToPathMap::value_type p(extension, path);
			for (auto &it : p.second)
			{
				if (it == _T('/'))
				{
					it = _T('\\');
				}
			}

			if (p.second.size() && (*(p.second.rbegin()) != _T('\\')))
			{
				p.second.append(_T("\\"));
			}

			m_mapExts.insert(p);
		}
	}
}


// removes the path to the mapping for whatever extension you want
void FileMapperImpl::RemoveMapping(const TCHAR *extension, const TCHAR *path)
{
	if (extension && path)
	{
		TExtToPathMap::iterator i = m_mapExts.lower_bound(extension);
		TExtToPathMap::iterator e = m_mapExts.upper_bound(extension);

		while (i != e)
		{
			if (i->second == path)
			{
				m_mapExts.erase(i);

				i = m_mapExts.find(extension);
				if (i == m_mapExts.end())
					m_nUniqueExts--;

				return;
			}

			i++;
		}
	}
}


// Supply a filename only and receive a fully qualified path in return, based on the
// file extension mappings you have previously set up.
// Returns true if the file was found
bool FileMapperImpl::FindFile(const TCHAR *filename, TCHAR *fullpath, size_t fullpathlen)
{
	if (!filename || !(*filename))
		return false;

	TCHAR internal_fullpath[MAX_PATH];
	internal_fullpath[0] = '\0';

	const TCHAR *ext = _tcsrchr(filename, '.');

	bool found = false;

	if (ext)
	{
		ext++;

		TExtToPathMap::const_iterator e = m_mapExts.upper_bound(ext);
		for (TExtToPathMap::const_iterator i = m_mapExts.lower_bound(ext); i != e; i++)
		{
			_tcsncpy_s(internal_fullpath, i->second.c_str(), MAX_PATH - 1);

			size_t plen = i->second.length();

			if (plen && (internal_fullpath[plen - 1] != '\\'))
			{
				if (internal_fullpath[plen - 1] == '/')
					internal_fullpath[plen - 1] = '\\';
				else
					_tcsncat_s(internal_fullpath, _T("\\"), MAX_PATH - 1);
			}

			_tcsncat_s(internal_fullpath, filename, MAX_PATH - 1);

			if (PathFileExists(internal_fullpath))
			{
				if (fullpath)
					_tcsncpy_s(fullpath, MAX_PATH, internal_fullpath, fullpathlen);

				return true;
			}
		}
	}

	if (PathFileExists(filename))
	{
		if (fullpath)
			_tcsncpy_s(fullpath, MAX_PATH, filename, fullpathlen);

		return true;
	}

	if (fullpath && (fullpathlen > 0))
		*fullpath = '\0';

	return false;
}


// this functions sets the path mappings for multiple extensions
// for example, you can set the path mappings for all texture file extensions at once...
// exts = "jpg;png;bmp;dds;tga;pcx;gif"
// paths = "./textures;./;../data/textures"
void FileMapperImpl::SetMappingsFromDelimitedStrings(const TCHAR *exts, const TCHAR *paths, const TCHAR delimiter)
{
	const TCHAR *c = exts;
	tstring tmp_ext;

	while (c && *c)
	{
		tmp_ext.clear();
		while (*c && (*c != delimiter))
		{
			tmp_ext += *c;
			c++;
		}

		if (*c == delimiter)
			c++;

		const TCHAR *d = paths;
		tstring tmp_path;

		while (d && *d)
		{
			tmp_path.clear();
			while (*d && (*d != delimiter))
			{
				tmp_path += *d;
				d++;
			}

			if (*d == delimiter)
				d++;

			if (!tmp_path.empty())
			{
				AddMapping(tmp_ext.c_str(), tmp_path.c_str());
			}
		}
	}
}