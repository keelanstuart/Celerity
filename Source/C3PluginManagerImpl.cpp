// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3PluginManagerImpl.h>
#include <C3PluginImpl.h>

#include <Shlwapi.h>
#include <io.h>


using namespace c3;


PluginManagerImpl::PluginManagerImpl(System *psys)
{
	m_pSys = psys;
}


PluginManagerImpl::~PluginManagerImpl()
{
	UnloadAllPlugins();
}


void __cdecl ActivatePlugin(LPVOID param0, LPVOID param1, size_t task_number)
{
	Plugin *pp = (Plugin *)param0;

	pp->Activate();
}

PluginManager::RETURNCODE PluginManagerImpl::LoadPlugin(const TCHAR *filename, bool activate)
{
	if (!filename)
		return RETURNCODE::RET_BADFILESPEC;

	if (PluginIsLoaded(filename))
		return RETURNCODE::RET_OK;

	Plugin *tmp = new PluginImpl(filename, m_pSys);

	if (tmp->Loaded())
	{
		m_Plugins.push_back(tmp);

		if (activate)
		{
#if 0
			m_pSys->ThreadPool()->RunTask(ActivatePlugin, (LPVOID)tmp);
#else
			tmp->Activate();
#endif
		}

		return RET_OK;
	}

	delete tmp;
	return RETURNCODE::RET_NOTFOUND;
}

PluginManager::RETURNCODE PluginManagerImpl::DiscoverPlugins(const TCHAR *path, const TCHAR *filespec, bool auto_activate, size_t *numfound)
{
	if (!filespec)
		return RETURNCODE::RET_BADFILESPEC;

	size_t ret = 0;

	TCHAR _filespec[MAX_PATH] = { 0 };

	if (path)
		_tcsncpy_s(_filespec, MAX_PATH, path, MAX_PATH - 1);
	size_t fslen = _tcslen(_filespec);
	if (fslen)
	{
		fslen--;
		if ((_filespec[fslen] != _T('\\')) && (_filespec[fslen] != _T('/')))
			_tcsncat_s(_filespec, MAX_PATH, _T("/"), MAX_PATH - 1);
	}
	_tcsncat_s(_filespec, MAX_PATH, filespec, MAX_PATH - 1);

	TCHAR *c = _filespec;
	while (*c)
	{
		if (*c == '/')
			*c = '\\';
		c++;
	}

	if (!PathIsRoot(_filespec))
	{
		TCHAR cwd[MAX_PATH];
		_tgetcwd(cwd, MAX_PATH - 1);

		PathCombine(_filespec, cwd, _filespec);
	}

	TCHAR dir[MAX_PATH];
	_tcsncpy_s(dir, MAX_PATH, _filespec, MAX_PATH - 1);
	PathRemoveFileSpec(dir);
	PathAddBackslash(dir);

	WIN32_FIND_DATA fd;
	HANDLE hfind = FindFirstFile(_filespec, &fd);

	if (hfind != INVALID_HANDLE_VALUE)
	{
		TCHAR plugname[MAX_PATH];

		do
		{
			_tcsncpy_s(plugname, MAX_PATH, dir, MAX_PATH - 1);
			_tcsncat_s(plugname, MAX_PATH, fd.cFileName, MAX_PATH - 1);

			if (!PluginIsLoaded(plugname))
			{
				if (LoadPlugin(plugname, auto_activate) == RET_OK)
					ret++;
			}
		}
		while (FindNextFile(hfind, &fd));

		FindClose(hfind);
	}

	if (numfound)
		*numfound = ret;

	return RET_OK;
}


PluginManager::RETURNCODE PluginManagerImpl::UnloadPlugin(Plugin *pplug)
{
	for (TPluginArray::iterator it = m_Plugins.begin(), last_it = m_Plugins.end(); it != last_it; it++)
	{
		// first, make sure the plugin is being managed
		if (pplug == *it)
		{
			pplug->Deactivate();

			delete pplug;
			m_Plugins.erase(it);

			return RET_OK;
		}
	}

	return RET_NOTFOUND;
}


void PluginManagerImpl::UnloadAllPlugins()
{
	for (auto &it : m_Plugins)
	{
		it->Deactivate();

		delete it;
	}

	m_Plugins.clear();
}


size_t PluginManagerImpl::GetNumPlugins() const
{
	return m_Plugins.size();
}


Plugin *PluginManagerImpl::GetPlugin(size_t idx) const
{
	if (idx < m_Plugins.size())
		return m_Plugins[idx];

	return nullptr;
}


bool PluginManagerImpl::PluginIsLoaded(const TCHAR *path, size_t *idx) const
{
	for (size_t i = 0, maxi = m_Plugins.size(); i < maxi; i++)
	{
		if (!_tcsicmp(m_Plugins[i]->GetFilename(), path))
		{
			if (idx)
				*idx = i;

			return true;
		}
	}

	return false;
}
