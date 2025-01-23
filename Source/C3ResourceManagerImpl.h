// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3ResourceManager.h>
#include "C3ZipImpl.h"
#include <atomic>

namespace c3
{

	class ResourceManagerImpl : public ResourceManager
	{

	protected:
		System *m_pSys;
		std::atomic<uint64_t> m_LastFrameChanged;

		typedef std::map<tstring, Resource *, std::less<tstring>> TResourceMap;
		TResourceMap m_ResMap;

		typedef std::multimap<const ResourceType *, Resource *> TResourceByTypeMap;
		TResourceByTypeMap m_ResByTypeMap;

		typedef std::deque<const ResourceType *> TResourceTypeArray;
		TResourceTypeArray m_ResTypes;

		typedef std::multimap<tstring, const ResourceType *> TExtToResourceTypeMap;
		TExtToResourceTypeMap m_ExtResTypeMap;

		using ArchiveID = uint16_t;

		typedef std::map<ArchiveID, std::pair<tstring, ZipFile *>> TZipFileRegistry;
		TZipFileRegistry m_ZipFileRegistry;

		friend class ResourceImpl;


	public:

		ResourceManagerImpl(System *psys);

		virtual ~ResourceManagerImpl();

		static pool::IThreadPool::TASK_RETURN __cdecl LoadingThreadProc(void *presmanimpl, void *pres, size_t task_number);

		virtual Resource *GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype = nullptr, const void *data = nullptr);

		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype = nullptr, props::TFlags64 restypeflags = 0, ResTypeFlagMode flagmode = RTFM_IGNORE);

		virtual void RegisterResourceType(const ResourceType *restype);

		virtual void UnregisterResourceType(const ResourceType *restype);

		virtual size_t GetNumResourceTypes() const;

		virtual const ResourceType *GetResourceType(size_t index) const;

		virtual const ResourceType *FindResourceTypeByExt(const TCHAR *ext) const;

		virtual const ResourceType *FindResourceTypeByName(const TCHAR *name) const;

		virtual const ResourceType *FindResourceType(GUID guid) const;

		virtual void Reset();

		virtual bool IsZipArchiveRegistered(const TCHAR *filename) const;

		virtual bool RegisterZipArchive(const TCHAR *filename);

		virtual void UnregisterZipArchive(const TCHAR *filename);

		virtual size_t GetNumRegisteredZipArchives() const;

		virtual const TCHAR *GetRegisteredZipArchivePath(size_t idx) const;

		const ZipFile *GetZipFile(uint16_t zipid) const;

		virtual bool FindZippedFile(const TCHAR *filename, TCHAR *fullpath, size_t fullpathlen);

		virtual size_t GetNumResources();

		virtual Resource *GetResourceByIndex(size_t index);

		virtual uint64_t GetLastFrameChanged();

		void UpdateLastFrameChanged();

	};

};
