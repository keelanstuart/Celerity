// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ResourceManagerImpl.h>
#include <C3ResourceImpl.h>
#include <Shlwapi.h>
#include <C3RendererImpl.h>


using namespace c3;


ResourceManagerImpl::ResourceManagerImpl(System *psys)
{
	m_pSys = psys;
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

pool::IThreadPool::TASK_RETURN ResourceManagerImpl::LoadingThreadProc(void *presmanimpl, void *pres, size_t task_number)
{
	assert(presmanimpl);

	if (!pres)
		return pool::IThreadPool::TR_OK;

	((Resource *)pres)->AddRef();

	return pool::IThreadPool::TR_OK;
}


Resource *ResourceManagerImpl::GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype, const void *data)
{
	if (!filename || !*filename)
		return nullptr;

	Resource *pres = nullptr;

	tstring key = filename;
	std::transform(key.begin(), key.end(), key.begin(), tolower);

	// the upper bound will be resmap.end() if either there are no matching entries or
	// if the entry is the last in the map.
	TResourceMap::const_iterator e = m_ResMap.upper_bound(key);
	for (TResourceMap::const_iterator i = m_ResMap.lower_bound(key); i != e; i++)
	{
		pres = i->second;
		if (flags.IsSet(RESF_FINDENTRYONLY) || ((pres->GetStatus() == Resource::RS_LOADED) && (!pres->GetType() || (pres->GetType() == restype))))
			return pres;
	}

	if (flags.IsSet(RESF_FINDENTRYONLY))
		return pres;

	tstring filename_only = key;
	tstring opts;
	size_t opts_ofs = key.find(_T('|'));
	if (opts_ofs != tstring::npos)
	{
		opts = key.c_str() + opts_ofs + 1;
		filename_only.erase(opts_ofs, opts.length() + 1);
	}

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
		}
	}

	if (pres)
	{
		if (!only_create_entry && (pres->GetStatus() == Resource::Status::RS_NONE))
		{
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
					m_pSys->GetThreadPool()->RunTask(LoadingThreadProc, (void *)this, (void *)pres);
				}
			}
			else
			{
				((RendererImpl *)(m_pSys->GetRenderer()))->GetTaskPool()->RunTask(LoadingThreadProc, (void *)this, (void *)pres);
			}
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
}


bool ResourceManagerImpl::RegisterZipArchive(const TCHAR *filename)
{
	bool ret = false;

	if (filename)
	{
		ZipFile *pzf = new ZipFile();
		if (pzf)
		{
			ret = pzf->Open(filename, ZIPOPEN_READ);
			if (ret)
			{
				static uint16_t sZipId = 1;

				m_ZipFileRegistry.insert(TZipFileRegistry::value_type(sZipId, TZipFileRegistry::mapped_type(filename, pzf)));

				TCHAR *cfn = _tcsdup(filename);
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
