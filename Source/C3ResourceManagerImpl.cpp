// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3ResourceManagerImpl.h>
#include <C3ResourceImpl.h>
#include <Shlwapi.h>
#include <C3RendererImpl.h>
#include <C3BlobImpl.h>
#include <C3Utility.h>


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

#if 1
Resource *ResourceManagerImpl::GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype, const ResourceCodec *rescodec, const void *data)
{
	if (!filename || !*filename)
		return nullptr;

	Resource *pres = nullptr;

	// don't transform the options so that they are lower case -- preserve them... important for shader compiler preprocessor directives, amongst others
	tstring key = filename;
	size_t opts_ofs = key.find(_T('|'));
	std::transform(key.begin(), (opts_ofs == tstring::npos) ? key.end() : key.begin() + opts_ofs, key.begin(),
		[](TCHAR c) -> TCHAR
	{
		if (c == _T('\\'))
			return _T('/');
		return tolower(c);
	});

	tstring filename_only = key;
	tstring opts;
	if (opts_ofs != tstring::npos)
	{
		opts = key.c_str() + opts_ofs + 1;
		filename_only.erase(opts_ofs, opts.length() + 1);
	}

	TResourceMap::const_iterator e = m_ResMap.find(key);
	if (e != m_ResMap.end())
		pres = e->second;

	if (flags.IsSet(RESF_FINDENTRYONLY))
		return pres;

	bool only_create_entry = flags.IsSet(RESF_CREATEENTRYONLY);

	// there's no entry for the resource? ok... make one!
	if (!pres)
	{
		if (!only_create_entry || flags.IsSet(RESF_ACQUIRECODEC))
		{
			const TCHAR *ext = NULL;

			// find the file extension and advance past the '.' if possible
			ext = PathFindExtension(filename_only.c_str());
			if (ext)
				ext++;

			if (!rescodec || flags.IsSet(RESF_ACQUIRECODEC))
			{
				rescodec = FindBestCodecByExt(ext, restype, false);
				if (rescodec)
					restype = rescodec->GetResourceType();
			}
		}

		// If no semantic type was given, derive it from the codec
		if (!restype && rescodec)
			restype = rescodec->GetResourceType();

		// if a specific resource type wasn't given then force it to be a blob
		if (!restype || !rescodec)
		{
			restype = RESOURCETYPE(Blob);
			rescodec = RESOURCECODEC(Blobs);
		}

		if (!restype)
			return nullptr;

		if (rescodec && (restype != rescodec->GetResourceType()))
		{
			m_pSys->GetLog()->Print(_T("Resource Type/Codec Mismatch! \"%s\" %s : %s"),
				key.c_str(), restype->GetName(), rescodec->GetName());
		}

		TCHAR fullpath[MAX_PATH];

		if (!only_create_entry)
		{
			if (!m_pSys->GetFileMapper()->FindFile(filename_only.c_str(), fullpath, MAX_PATH))
				return nullptr;
		}

		pres = new ResourceImpl(only_create_entry ? filename_only.c_str() : fullpath, opts.c_str(), restype, rescodec, only_create_entry ? data : nullptr);
		if (pres)
		{
			// if the resource already exists under the filename_only key,
			// then we may need to copy the aux data for zipfile support
			TResourceMap::const_iterator ex = m_ResMap.find(filename_only);
			if (ex != m_ResMap.end())
			{
				((ResourceImpl *)pres)->m_Aux = ((ResourceImpl *)(ex->second))->m_Aux;
				((ResourceImpl *)pres)->m_AuxFlags = ((ResourceImpl *)(ex->second))->m_AuxFlags;
			}

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
#else
Resource *ResourceManagerImpl::GetResource(const TCHAR *filename, props::TFlags64 flags,
	const ResourceType *restype, const ResourceCodec *pcodec,
	const void *data)
{
	if (!filename || !*filename)
		return nullptr;

	TCHAR key[1024];
	TCHAR filename_only[512];
	TCHAR opts[512];

	TCHAR cache_key[1024];
	cache_key[0] = _T('\0');

	bool user_supplied_type = (restype != nullptr);
	bool user_supplied_codec = (pcodec != nullptr);

	Resource *pres = nullptr;

	// Preserve options case exactly as before.
	_tcscpy_s(key, filename);
	TCHAR *opts_start = _tcschr(key, _T('|'));
	if (opts_start)
	{
		*opts_start = _T('\0');
		opts_start++;
	}
	else
		opts_start = _T("");

	TCHAR *pkey = key;
	while (*pkey)
	{
		TCHAR kc = *pkey;
		if (kc == _T('\\'))
			kc = _T('/');
		else
			kc = (TCHAR)tolower(kc);

		*pkey = kc;

		pkey++;
	};

	_tcscpy_s(filename_only, key);

	if (opts_start)
		_tcscpy_s(opts, opts_start);

	bool only_create_entry = flags.IsSet(RESF_CREATEENTRYONLY);
	bool find_entry_only   = flags.IsSet(RESF_FINDENTRYONLY);

	// Determine extension from the normalized filename-only key.
	const TCHAR *ext = PathFindExtension(filename_only);
	if (ext && *ext == _T('.'))
		ext++;

	// If caller explicitly gave a codec, trust it.
	// Otherwise, if this is not just a placeholder creation, try to resolve one now.
	if (!pcodec && (!only_create_entry || flags.IsSet(RESF_ACQUIRECODEC)))
		pcodec = FindBestCodecByExt(ext, restype, false);

	// If no semantic type was given, derive it from the codec.
	if (!restype && pcodec)
		restype = pcodec->GetResourceType();

	// For create-entry-only, preserve old behavior: allow placeholders even if
	// we have not fully resolved the codec yet. But still prefer a meaningful type.
	if (!restype)
	{
		if (!only_create_entry && ext)
		{
			// If your FindBestCodecByExt returned null but you still want a fallback:
			// leave this as Blob.
		}

		restype = RESOURCETYPE(Blob);
	}

	// Make cache identity codec-aware if a codec is known.
	// This prevents one interpretation from aliasing another.
	_tcscpy_s(cache_key, key);

#if 0
	if (user_supplied_type && restype)
	{
		OLECHAR gbuf[64]; gbuf[0] = _T('\0');
		StringFromGUID2(restype->GetGUID(), gbuf, 64);
		_tcscat_s(cache_key, _T("|type="));
		_tcscat_s(cache_key, gbuf);
	}

	if (user_supplied_codec && pcodec)
	{
		OLECHAR gbuf[64]; gbuf[0] = _T('\0');
		StringFromGUID2(pcodec->GetGUID(), gbuf, 64);
		_tcscat_s(cache_key, _T("|codec="));
		_tcscat_s(cache_key, gbuf);
	}
#endif

	TResourceMap::const_iterator e = m_ResMap.find(cache_key);
	if (e != m_ResMap.end())
		pres = e->second;

	if (find_entry_only)
		return pres;

	// No entry yet? Create one.
	if (!pres)
	{
		TCHAR fullpath[512]; fullpath[0] = _T('\0');

		// Preserve existing behavior:
		// CREATEENTRYONLY does NOT require the file to exist or be mappable yet.
		if (!only_create_entry)
		{
			if (!m_pSys->GetFileMapper()->FindFile(filename_only, fullpath, MAX_PATH))
				return nullptr;
		}

		pres = new ResourceImpl(
			only_create_entry ? filename_only : fullpath,
			opts,
			restype,
			pcodec,
			only_create_entry ? data : nullptr);

		if (pres)
		{
			// Preserve the old zip/aux inheritance behavior from an existing
			// filename_only entry if present.
			TResourceMap::const_iterator ex = m_ResMap.find(filename_only);
			if (ex != m_ResMap.end())
			{
				((ResourceImpl *)pres)->m_Aux = ((ResourceImpl *)(ex->second))->m_Aux;
				((ResourceImpl *)pres)->m_AuxFlags = ((ResourceImpl *)(ex->second))->m_AuxFlags;
			}

			m_ResMap.insert(TResourceMap::value_type(cache_key, pres));
			m_ResByTypeMap.insert(TResourceByTypeMap::value_type(restype, pres));
			UpdateLastFrameChanged();
		}
	}

	if (!pres)
		return nullptr;

	// Preserve the old CREATEENTRYONLY "replace existing data" behavior.
	if (only_create_entry)
	{
		if (data && (pres->GetType() == restype))
			((ResourceImpl *)pres)->OverrideData((void *)data);

		// Placeholder entries may not have a codec yet.
		if (pcodec && !pres->GetCodec())
			((ResourceImpl *)pres)->SetCodec(pcodec);

		return pres;
	}

	// If this was an existing placeholder without a codec, fill it in now.
	if (pcodec && !pres->GetCodec())
		((ResourceImpl *)pres)->SetCodec(pcodec);

	// Existing placeholder may also still be Blob if it was created early.
	// If you add SetType(), this is a good place to refine it.
	// if (restype && (pres->GetType() != restype))
	//     ((ResourceImpl *)pres)->SetType(restype);

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
				// Existing behavior: AddRef triggers synchronous load.
				pres->AddRef();
			}
			else
			{
				m_pSys->GetThreadPool()->RunTask(LoadingThreadProc);
			}
		}
		else
		{
			((RendererImpl *)(m_pSys->GetRenderer()))->GetTaskPool()->RunTask(LoadingThreadProc);
		}
	}

	return pres;
}
#endif

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
	{
		m_pSys->GetLog()->Print(_T("[C3] Attempting to re-register %s (ResourceType)!\n"), restype->GetName());
		return;
	}

	m_ResTypes.push_back(restype);

	m_ResTypeExtSetMap.insert_or_assign(restype, TStringSet());

	m_ResTypeExtsMap.insert_or_assign(restype, _T(""));
}


void ResourceManagerImpl::UnregisterResourceType(const ResourceType *restype)
{
	TResourceTypeToExtSetMap::iterator esit = m_ResTypeExtSetMap.find(restype);
	if (esit != m_ResTypeExtSetMap.end())
		m_ResTypeExtSetMap.erase(esit);

	TResourceTypeToExtsMap::iterator sit = m_ResTypeExtsMap.find(restype);
	if (sit != m_ResTypeExtsMap.end())
		m_ResTypeExtsMap.erase(sit);

	TResourceTypeArray::iterator it = std::find(m_ResTypes.begin(), m_ResTypes.end(), restype);
	if (it != m_ResTypes.end())
		m_ResTypes.erase(it);
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


void ResourceManagerImpl::RegisterResourceCodec(const ResourceCodec *pcodec)
{
	if (!pcodec)
		return;

	const ResourceType *rt = pcodec->GetResourceType();

	if (std::find(m_ResTypes.cbegin(), m_ResTypes.cend(), rt) == m_ResTypes.cend())
	{
		m_pSys->GetLog()->Print(_T("[C3] Attempting to register codec (%s) before type (%s)!\n"), pcodec->GetName(), rt->GetName());
		return;
	}

	auto itr = m_ResTypeToCodec.equal_range(rt);

	for (TResourceTypeToCodecMap::const_iterator it = itr.first; it != itr.second; it++)
	{
		// if it's already registered, get out
		if (it->second == pcodec)
		{
			m_pSys->GetLog()->Print(_T("[C3] Attempting to re-register %s (ResourceCodec)!\n"), pcodec->GetName());
			return;
		}
	}

	m_ResTypeToCodec.insert(TResourceTypeToCodecMap::value_type(rt, pcodec));

	TStringSet ss_read, ss_save;
	util::MakeSetFromDelimitedList(pcodec->GetReadableExtensions(), _T(';'), ss_read);
	util::MakeSetFromDelimitedList(pcodec->GetSavableExtensions(), _T(';'), ss_save);

	TResourceTypeToExtSetMap::iterator srit = m_ResTypeExtSetMap.find(rt);
	if (srit != m_ResTypeExtSetMap.end())
		util::CombineStringSets(srit->second, ss_read, srit->second);

	TResourceTypeToExtSetMap::iterator ssit = m_ResTypeExtSetMap.find(rt);
	if (srit != m_ResTypeExtSetMap.end())
		util::CombineStringSets(srit->second, ss_read, srit->second);

	TResourceTypeToExtListMap::iterator erit = m_ResTypeToExtsRead.find(rt);
	if (erit != m_ResTypeToExtsRead.end())
	{
		util::CombineDelimitedLists(erit->second.c_str(), pcodec->GetReadableExtensions(), _T(';'), erit->second);
	}
	else
	{
		m_ResTypeToExtsRead.insert(TResourceTypeToExtListMap::value_type(rt,
			tstring(pcodec->GetReadableExtensions() ? pcodec->GetReadableExtensions() : _T(""))));
	}

	TResourceTypeToExtListMap::iterator esit = m_ResTypeToExtsSave.find(rt);
	if (esit != m_ResTypeToExtsSave.end())
	{
		util::CombineDelimitedLists(esit->second.c_str(), pcodec->GetSavableExtensions(), _T(';'), esit->second);
	}
	else
	{
		m_ResTypeToExtsSave.insert(TResourceTypeToExtListMap::value_type(rt,
			tstring(pcodec->GetSavableExtensions() ? pcodec->GetSavableExtensions() : _T(""))));
	}
}


void ResourceManagerImpl::UnregisterResourceCodec(const ResourceCodec *pcodec)
{
	if (!pcodec)
		return;

	tstring rexts = _T("");
	tstring sexts = _T("");

	const ResourceType *rt = pcodec->GetResourceType();
	TResourceTypeToCodecMap::const_iterator it = m_ResTypeToCodec.lower_bound(rt);
	TResourceTypeToCodecMap::const_iterator eit = m_ResTypeToCodec.end();

	while ((it != m_ResTypeToCodec.end()) && (it->first == rt))
	{
		// if it's already registered, get out
		if (it->second == pcodec)
		{
			eit = it;
		}
		else
		{
			util::CombineDelimitedLists(rexts.c_str(), it->second->GetReadableExtensions(), _T(';'), rexts);
			util::CombineDelimitedLists(sexts.c_str(), it->second->GetSavableExtensions(), _T(';'), sexts);
		}

		it++;
	}

	if (eit != m_ResTypeToCodec.end())
		m_ResTypeToCodec.erase(eit);

	TResourceTypeToExtListMap::iterator rit = m_ResTypeToExtsRead.find(rt);
	if (rit != m_ResTypeToExtsRead.end())
	{
		if (rexts.empty())
			m_ResTypeToExtsRead.erase(rit);
		else
			rit->second = rexts;
	}

	TResourceTypeToExtListMap::iterator sit = m_ResTypeToExtsSave.find(rt);
	if (sit != m_ResTypeToExtsSave.end())
	{
		if (sexts.empty())
			m_ResTypeToExtsSave.erase(sit);
		else
			sit->second = sexts;
	}
}


const ResourceCodec *ResourceManagerImpl::FindBestCodecByExt(const TCHAR *ext, const ResourceType *restype,	bool for_write) const
{
	if (!ext || !*ext)
		return nullptr;

	if (*ext == _T('.'))
		ext++;

	const ResourceCodec *best = nullptr;
	int bestpri = INT_MIN;

	auto codec_supports_ext = [&ext, for_write](const ResourceCodec *pc) -> bool
	{
		if (!pc)
			return false;

		const TCHAR *exts = for_write ? pc->GetSavableExtensions() : pc->GetReadableExtensions();

		return util::DelimitedListContains(exts, _T(';'), ext);
	};

	if (restype)
	{
		// Optional fast reject using aggregated extension list
		const TResourceTypeToExtListMap &extmap =
			for_write ? m_ResTypeToExtsSave : m_ResTypeToExtsRead;

		TResourceTypeToExtListMap::const_iterator xit = extmap.find(restype);
		if (xit == extmap.end())
			return nullptr;

		if (!util::DelimitedListContains(xit->second.c_str(), _T(';'), ext))
			return nullptr;

		// Search codecs for this type
		TResourceTypeToCodecMap::const_iterator it = m_ResTypeToCodec.lower_bound(restype);
		while ((it != m_ResTypeToCodec.end()) && (it->first == restype))
		{
			const ResourceCodec *pc = it->second;
			if (codec_supports_ext(pc))
			{
				if (!best || (pc->GetPriority() > bestpri))
				{
					best = pc;
					bestpri = pc->GetPriority();
				}
			}

			it++;
		}
	}
	else
	{
		// No type specified: search all codecs
		for (TResourceTypeToCodecMap::const_iterator it = m_ResTypeToCodec.cbegin(); it != m_ResTypeToCodec.cend(); it++)
		{
			const ResourceCodec *pc = it->second;
			if (codec_supports_ext(pc))
			{
				if (!best || (pc->GetPriority() > bestpri))
				{
					best = pc;
					bestpri = pc->GetPriority();
				}
			}
		}
	}

	return best;
}

void ResourceManagerImpl::BuildExtensionListForType(const ResourceType *restype, tstring &extlist)
{
	auto r = this->m_ResTypeToExtsRead.equal_range(restype);

	extlist.clear();
	for (TResourceTypeToExtListMap::const_iterator it = r.first; it != r.second; it++)
	{
		if (it != r.first)
			extlist += _T(';');

		extlist += it->second;
	}
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
		// absolute paths should be checked 
		if (!PathIsRelative(filename))
		{
			TCHAR fullpath[MAX_PATH * 2];

			if (m_pSys->GetFileMapper()->FindFile(filename, fullpath, MAX_PATH * 2) && !_tcsicmp(filename, zit->second.first.c_str()))
				return true;
		}

		if (!_tcsicmp(filename, zit->second.first.c_str()))
			return true;
	}

	return false;
}

bool ResourceManagerImpl::RegisterZipArchive(const TCHAR *filename)
{
	if (!filename)
		return false;

	bool ret = false;

	TCHAR full_filename[MAX_PATH * 2];
	if (m_pSys->GetFileMapper()->FindFile(filename, full_filename, MAX_PATH * 2))
	{
		if (IsZipArchiveRegistered(full_filename))
			return true;

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

					ResourceImpl *pr = (ResourceImpl *)GetResource(rfn.c_str(), RESF_CREATEENTRYONLY | RESF_ACQUIRECODEC);
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
		for (TCHAR *c = tmp; *c != 0; c++)
			*c = std::tolower(*c);

		const TCHAR *lpath = filename;

		// if the root of the filename is the name of this packfile, then
		// assume this is it and skip ahead of that name
		const TCHAR *s = _tcsstr(filename, tmp);
		if (s != filename)
		{
			_tcscat_s(tmp, _T("/"));
			_tcscat_s(tmp, filename);
			lpath = tmp;
		}

		if (GetResource(lpath, RESF_FINDENTRYONLY) != nullptr)
		{
			if (fullpath)
				_tcscpy_s(fullpath, fullpathlen, lpath);

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
