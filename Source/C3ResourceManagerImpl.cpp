// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3ResourceManagerImpl.h>
#include <C3ResourceImpl.h>
#include <Shlwapi.h>
#include <C3RendererImpl.h>


using namespace c3;


ResourceManagerImpl::ResourceManagerImpl(System *psys)
{
	m_pSys = psys;
	m_LastFrameChanged = 0;

}


ResourceManagerImpl::~ResourceManagerImpl()
{
	// reset all resources
	Reset();

	for (auto &zit : m_ZipFileRegistry)
	{
		if (zit.second.second)
			delete zit.second.second;
	}

	m_ZipFileRegistry.clear();
}


Resource *ResourceManagerImpl::GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype, const void *data)
{
	if (!filename || !*filename)
		return nullptr;

	Resource *pres = nullptr;

	// don't transform the options so that they are lower case -- preserve them... important for shader compiler preprocessor directives, amongst others
	tstring key = filename;
	size_t opts_ofs = key.find(_T('|'));
	std::transform(key.begin(), (opts_ofs == tstring::npos) ? key.end() : key.begin() + opts_ofs, key.begin(), tolower);

	tstring filename_only = key;
	tstring opts;
	if (opts_ofs != tstring::npos)
	{
		opts = key.c_str() + opts_ofs + 1;
		filename_only.erase(opts_ofs, opts.length() + 1);
	}

#if 0
	// the upper bound will be resmap.end() if either there are no matching entries or
	// if the entry is the last in the map.
	TResourceMap::const_iterator e = m_ResMap.upper_bound(key);
	for (TResourceMap::const_iterator i = m_ResMap.lower_bound(key); i != e; i++)
	{
		pres = i->second;
		if (flags.IsSet(RESF_FINDENTRYONLY) || ((pres->GetStatus() == Resource::RS_LOADED) && (!pres->GetType() || (pres->GetType() == restype))))
			return pres;
	}
#else
	TResourceMap::const_iterator e = m_ResMap.find(key);
	if (e != m_ResMap.end())
	{
		pres = e->second;
		if (flags.IsSet(RESF_FINDENTRYONLY) || ((pres->GetStatus() == Resource::RS_LOADED) && (!pres->GetType() || (pres->GetType() == restype))))
			return pres;
	}
#endif

	if (flags.IsSet(RESF_FINDENTRYONLY))
		return pres;

	bool only_create_entry = flags.IsSet(RESF_CREATEENTRYONLY);

	if (!pres)
	{
		const TCHAR *ext = NULL;

		//if (!only_create_entry)
		{
			// find the file extension and advance past the '.' if possible
			ext = PathFindExtension(filename_only.c_str());
		}

		if (ext)
		{
			ext++;
		}

		// if a specific resource type wasn't given and we're not just creating an entry in the table,
		/// then find the resource type based on the file extension
		if (!restype)
		{
			if (ext)
			{
				TExtToResourceTypeMap::const_iterator it = m_ExtResTypeMap.lower_bound(ext);
				if (it != m_ExtResTypeMap.cend())
					restype = it->second;
			}
		}

		if (!restype)
			return nullptr;

		TCHAR fullpath[MAX_PATH];

		if (!only_create_entry)
		{
			if (!m_pSys->GetFileMapper()->FindFile(filename_only.c_str(), fullpath, MAX_PATH))
				return nullptr;
		}

		pres = new ResourceImpl(only_create_entry ? filename_only.c_str() : fullpath, opts.c_str(), restype, only_create_entry ? data : nullptr);
		if (pres)
		{
			m_ResMap.insert(TResourceMap::value_type(key, pres));
			m_ResByTypeMap.insert(TResourceByTypeMap::value_type(restype, pres));

			UpdateLastFrameChanged();
		}
	}

	if (pres)
	{
		if (!only_create_entry)
		{
			if (pres->GetStatus() == Resource::Status::RS_NONE)
			{
				std::function<pool::IThreadPool::TASK_RETURN(size_t task_number)> LoadingThreadProc =
				[res = pres](size_t task_number) -> pool::IThreadPool::TASK_RETURN
				{
					if (!res)
						return pool::IThreadPool::TR_OK;

					((Resource *)res)->AddRef();

					return pool::IThreadPool::TR_OK;
				};

				if (flags.IsSet(RESF_DEMANDLOAD) || !pres->GetType()->Flags().IsSet(RTFLAG_RUNBYRENDERER))
				{
					if (flags.IsSet(RESF_DEMANDLOAD))
					{
						// Just adding a reference should cause the resource to load... and in this thread.
						pres->AddRef();
					}
					else
					{
						// Since we didn't demand that this get loaded right now, schedule it on the thread pool.
						m_pSys->GetThreadPool()->RunTask(LoadingThreadProc);
					}
				}
				else
				{
					((RendererImpl *)(m_pSys->GetRenderer()))->GetTaskPool()->RunTask(LoadingThreadProc);
				}
			}
		}
		else if (data && (pres->GetType() == restype))
		{
			((ResourceImpl *)pres)->OverrideData((void *)data);
		}
	}

	return pres;
}

void ResourceManagerImpl::ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype, props::TFlags64 restypeflags, ResTypeFlagMode flagmode)
{
	if (!func)
		return;

	TResourceByTypeMap::iterator it = restype ? m_ResByTypeMap.lower_bound(restype) : m_ResByTypeMap.begin();
	TResourceByTypeMap::iterator last_it = restype ? m_ResByTypeMap.upper_bound(restype) : m_ResByTypeMap.end();

	// if no resource type was given, then recurse through into this function for each ResourceType in the map
	if (it == m_ResByTypeMap.end())
		return;

	for (; it != last_it; it++)
	{
		if (flagmode != ResTypeFlagMode::RTFM_IGNORE)
		{
			if ((flagmode == ResTypeFlagMode::RTFM_ANY) && !it->first->Flags().AnySet(restypeflags))
				continue;

			if ((flagmode == ResTypeFlagMode::RTFM_ALL) && (it->first->Flags() != restypeflags))
				continue;

			if ((flagmode == ResTypeFlagMode::RTFM_NONE) && it->first->Flags().AnySet(restypeflags))
				continue;
		}

		Resource *pres = it->second;
		func(pres);
	}
}


void ResourceManagerImpl::RegisterResourceType(const ResourceType *restype)
{
	if (!restype)
		return;

	if (std::find(m_ResTypes.cbegin(), m_ResTypes.cend(), restype) != m_ResTypes.cend())
		return;

	const TCHAR *t = restype->GetReadableExtensions(), *h = t;
	while (*t)
	{
		while (*h && (*h != _T(';')))
		{
			h++;
		}

		tstring ext;
		while (t < h)
		{
			ext += *(t++);
		}

		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		m_ExtResTypeMap.insert(TExtToResourceTypeMap::value_type(ext, restype));

		if (std::find(m_ResTypes.begin(), m_ResTypes.end(), restype) == m_ResTypes.end())
			m_ResTypes.push_back(restype);

		h++;
		t = h;
	}
}


void ResourceManagerImpl::UnregisterResourceType(const ResourceType *restype)
{
	const TCHAR *t = restype->GetReadableExtensions(), *h = t;
	while (*t)
	{
		while (*h && (*h != _T(';')))
		{
			h++;
		}

		tstring ext;
		while (t < h)
		{
			ext += *(t++);
		}

		std::transform(ext.begin(), ext.end(), ext.begin(), _tolower);

		TExtToResourceTypeMap::iterator it = m_ExtResTypeMap.lower_bound(ext);
		TExtToResourceTypeMap::iterator last_it = m_ExtResTypeMap.upper_bound(ext);
		while (it != last_it)
		{
			if (it->second == restype)
			{
				m_ExtResTypeMap.erase(it);
				break;
			}
		}

		h++;
		t = h;
	}

	TResourceTypeArray::iterator eit = std::find(m_ResTypes.begin(), m_ResTypes.end(), restype);
	if (eit != m_ResTypes.end())
		m_ResTypes.erase(eit);
}


size_t ResourceManagerImpl::GetNumResourceTypes() const
{
	return m_ResTypes.size();
}


const ResourceType *ResourceManagerImpl::GetResourceType(size_t index) const
{
	if (index < m_ResTypes.size())
		return m_ResTypes[index];

	return nullptr;
}


const ResourceType *ResourceManagerImpl::FindResourceTypeByExt(const TCHAR *ext) const
{
	if (ext)
	{
		if (*ext == _T('.'))
			ext++;

		TExtToResourceTypeMap::const_iterator it = m_ExtResTypeMap.find(ext);
		if (it != m_ExtResTypeMap.end())
			return it->second;
	}

	return nullptr;
}


const ResourceType *ResourceManagerImpl::FindResourceTypeByName(const TCHAR *name) const
{
	if (name)
	{
		for (auto rt : m_ResTypes)
		{
			if (!_tcsicmp(rt->GetName(), name))
				return rt;
		}
	}

	return nullptr;
}


const ResourceType *ResourceManagerImpl::FindResourceType(GUID guid) const
{
	for (const auto &it : m_ResTypes)
	{
		if (it->GetGUID() == guid)
			return it;
	}

	return nullptr;
}


void ResourceManagerImpl::Reset()
{
	for (auto &it : m_ResMap)
	{
		Resource *pres = it.second;
		while (pres->GetStatus() == Resource::Status::RS_LOADED)
			pres->DelRef();
	}

	UpdateLastFrameChanged();
}


bool ResourceManagerImpl::IsZipArchiveRegistered(const TCHAR *filename) const
{
	for (auto zit = m_ZipFileRegistry.cbegin(); zit != m_ZipFileRegistry.cend(); zit++)
	{
		if (!_tcsicmp(filename, zit->second.first.c_str()))
			return true;
	}

	return false;
}

bool ResourceManagerImpl::RegisterZipArchive(const TCHAR *filename)
{
	if (!filename)
		return false;

	if (IsZipArchiveRegistered(filename))
		return true;

	bool ret = false;

	TCHAR full_filename[MAX_PATH * 2];
	if (m_pSys->GetFileMapper()->FindFile(filename, full_filename, MAX_PATH * 2))
	{
		ZipFile *pzf = new ZipFile();
		if (pzf)
		{
			ret = pzf->Open(full_filename, ZIPOPEN_READ);
			if (ret)
			{
				static uint16_t sZipId = 1;

				m_ZipFileRegistry.insert(TZipFileRegistry::value_type(sZipId, TZipFileRegistry::mapped_type(filename, pzf)));

				TCHAR *cfn = _tcsdup(full_filename);
				PathRemoveExtension(cfn);

				TCHAR *dfn = PathFindFileName(cfn);

				tstring rfn;
				rfn.reserve(_tcslen(cfn) * 2);

				// register all the files inside
				for (size_t i = 0, maxi = pzf->GetNumEntries(); i < maxi; i++)
				{
					rfn = dfn;
					rfn += _T('/');
					rfn += pzf->GetContentInfo(i)->fname;

					ResourceImpl *pr = (ResourceImpl *)GetResource(rfn.c_str(), RESF_CREATEENTRYONLY | RESF_ZIPRES);
					if (pr)
						pr->SetAux(sZipId, RESF_ZIPRES);
				}

				free(cfn);

				sZipId++;
			}
		}
	}

	return ret;
}


void ResourceManagerImpl::UnregisterZipArchive(const TCHAR *filename)
{
	TZipFileRegistry::iterator it = m_ZipFileRegistry.begin();
	while (it != m_ZipFileRegistry.end())
	{
		if (!_tcsicmp(it->second.first.c_str(), filename))
		{
			it->second.second->Close();
			delete it->second.second;

			m_ZipFileRegistry.erase(it);

			break;
		}

		it++;
	}
}


const ZipFile *ResourceManagerImpl::GetZipFile(uint16_t zipid) const
{
	TZipFileRegistry::const_iterator it = m_ZipFileRegistry.find(zipid);
	if (it != m_ZipFileRegistry.cend())
		return it->second.second;

	return nullptr;
}


size_t ResourceManagerImpl::GetNumRegisteredZipArchives() const
{
	return m_ZipFileRegistry.size();
}


const TCHAR *ResourceManagerImpl::GetRegisteredZipArchivePath(size_t idx) const
{
	if (idx >= m_ZipFileRegistry.size())
		return nullptr;

	auto it = m_ZipFileRegistry.cbegin();
	while (idx)
	{
		it++;
		idx--;
	}

	return it->second.first.c_str();
}


bool ResourceManagerImpl::FindZippedFile(const TCHAR *filename, TCHAR *fullpath, size_t fullpathlen)
{
	TZipFileRegistry::const_iterator it = m_ZipFileRegistry.cbegin();
	while (it != m_ZipFileRegistry.cend())
	{
		TCHAR tmp[512];

		_tcscpy_s(tmp, PathFindFileName(it->second.first.c_str()));
		PathRemoveExtension(tmp);
		_tcscat_s(tmp, _T("/"));
		_tcscat_s(tmp, filename);
		if (GetResource(tmp, RESF_FINDENTRYONLY) != nullptr)
		{
			if (fullpath)
				_tcscpy_s(fullpath, fullpathlen, tmp);
			return true;
		}

		it++;
	}

	if (fullpath && fullpathlen > 0)
		*fullpath = _T('\0');

	return false;
}


size_t ResourceManagerImpl::GetNumResources()
{
	return m_ResMap.size();
}


Resource *ResourceManagerImpl::GetResourceByIndex(size_t index)
{
	TResourceMap::const_iterator it = m_ResMap.cbegin();
	while (index-- && (it != m_ResMap.cend()))
		it++;

	if (it != m_ResMap.cend())
		return it->second;

	return nullptr;
}


uint64_t ResourceManagerImpl::GetLastFrameChanged()
{
	return m_LastFrameChanged;
}

void ResourceManagerImpl::UpdateLastFrameChanged()
{
	m_LastFrameChanged.fetch_add(1, std::memory_order_relaxed);
}


System *ResourceManagerImpl::GetSystem() const
{
	return m_pSys;
}