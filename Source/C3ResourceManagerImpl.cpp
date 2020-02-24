// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


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
}

void ResourceManagerImpl::LoadingThreadProc(LPVOID presmanimpl, LPVOID pres, size_t task_number)
{
	assert(presmanimpl);

	if (!pres)
		return;

	((Resource *)pres)->AddRef();
}


Resource *ResourceManagerImpl::GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype)
{
	if (!filename || !*filename)
		return NULL;

	Resource *pres = NULL;

	tstring key = filename;
	std::transform(key.begin(), key.end(), key.begin(), tolower);

	// the upper bound will be resmap.end() if either there are no matching entries or
	// if the entry is the last in the map.
	TResourceMap::const_iterator e = m_ResMap.upper_bound(key);
	for (TResourceMap::const_iterator i = m_ResMap.lower_bound(key); i != e; i++)
	{
		pres = i->second;
		if ((pres->GetStatus() == Resource::RS_LOADED) && (!pres->GetType() || (pres->GetType() == restype)))
			return pres;
	}

	if (!pres)
	{
		bool only_create_entry = flags.IsSet(RESFLAG_CREATEENTRYONLY);

		const TCHAR *ext = NULL;

		if (!only_create_entry)
		{
			// find the file extension and advance past the '.' if possible
			ext = PathFindExtension(key.c_str());
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
			m_pSys->GetFileMapper()->FindFile(key.c_str(), fullpath, MAX_PATH);
		}

		pres = new ResourceImpl(m_pSys, only_create_entry ? key.c_str() : fullpath, restype);
		if (pres && !only_create_entry)
		{
			m_ResMap.insert(TResourceMap::value_type(key, pres));
			m_ResByTypeMap.insert(TResourceByTypeMap::value_type(restype, pres));

			if (!restype->Flags().IsSet(RTFLAG_RUNBYRENDERER))
			{
				if (flags.IsSet(RESFLAG_DEMANDLOAD))
				{
					// Just adding a reference should cause the resource to load... and in this thread.
					pres->AddRef();
				}
				else
				{
					// Since we didn't demand that this get loaded right now, schedule it on the thread pool.
					m_pSys->GetThreadPool()->RunTask(LoadingThreadProc, (LPVOID)this, (LPVOID)pres);
				}
			}
			else
			{
				((RendererImpl *)(m_pSys->GetRenderer()))->GetTaskPool()->RunTask(LoadingThreadProc, (LPVOID)this, (LPVOID)pres);
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

	while (it != last_it)
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
		it++;
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


size_t ResourceManagerImpl::GetNumResourceTypes()
{
	return m_ResTypes.size();
}


const ResourceType *ResourceManagerImpl::GetResourceType(size_t index)
{
	if (index < m_ResTypes.size())
		m_ResTypes[index];

	return nullptr;
}


const ResourceType *ResourceManagerImpl::FindResourceType(const TCHAR *ext)
{
	if (ext)
	{
		if (*ext == _T('.'))
			ext++;

		TExtToResourceTypeMap::const_iterator it = m_ExtResTypeMap.find(ext);
		if (it != m_ExtResTypeMap.end())
			return it->second;
	}

	return NULL;
}


const ResourceType *ResourceManagerImpl::FindResourceType(GUID guid)
{
	for (TResourceTypeArray::const_iterator it = m_ResTypes.cbegin(), last_it = m_ResTypes.cend(); it != last_it; it++)
	{
		if ((*it)->GetGUID() == guid)
			return *it;
	}

	return nullptr;
}


void ResourceManagerImpl::Reset()
{
	TResourceMap::iterator e = m_ResMap.end();
	for (TResourceMap::iterator i = m_ResMap.begin(); i != e; i++)
	{
		Resource *pres = i->second;
		while (pres->GetStatus() == Resource::Status::RS_LOADED);
			pres->DelRef();
	}
}
