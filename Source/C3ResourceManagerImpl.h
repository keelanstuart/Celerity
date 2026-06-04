// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3ResourceManager.h>
#include "C3ZipImpl.h"
#include <C3Utility.h>
#include <atomic>

namespace c3
{

	class ResourceManagerImpl : public ResourceManager
	{

	protected:
		System *m_pSys;
		std::atomic<uint64_t> m_LastFrameChanged;

		using TResourceMap = std::map<tstring, Resource *, std::less<tstring>>;
		TResourceMap m_ResMap;

		using TResourceByTypeMap = std::multimap<const ResourceType *, Resource *>;
		TResourceByTypeMap m_ResByTypeMap;

		using TResourceTypeArray = std::deque<const ResourceType *>;
		TResourceTypeArray m_ResTypes;

		using TResourceTypeToExtSetMap = std::map<const ResourceType *, TStringSet>;
		TResourceTypeToExtSetMap m_ResTypeExtSetMap;

		using TResourceTypeToExtsMap = std::map<const ResourceType *, tstring>;
		TResourceTypeToExtsMap m_ResTypeExtsMap;

		using TResourceTypeToCodecMap = std::multimap<const ResourceType *, const ResourceCodec *>;
		TResourceTypeToCodecMap m_ResTypeToCodec;

		using TResourceTypeToExtListMap = std::map<const ResourceType *, tstring>;

		// Readable extension lists per registered ResourceType, based on registered ResourceCodecs
		TResourceTypeToExtListMap m_ResTypeToExtsRead;

		// Saveable extension lists per registered ResourceType, based on registered ResourceCodecs
		TResourceTypeToExtListMap m_ResTypeToExtsSave;

		using ArchiveID = uint16_t;

		using TZipFileRegistry = std::map<ArchiveID, std::pair<tstring, ZipFile *>>;
		TZipFileRegistry m_ZipFileRegistry;

		friend class ResourceImpl;

	public:

		ResourceManagerImpl(System *psys);

		virtual ~ResourceManagerImpl();

		static pool::IThreadPool::TASK_RETURN __cdecl LoadingThreadProc(void *presmanimpl, void *pres, size_t task_number);

		virtual Resource *GetResource(const TCHAR *filename, ResourceFlags flags = 0,
			const ResourceType *restype = nullptr, const ResourceCodec *pcodec = nullptr,
			const void *data = nullptr);

		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype = nullptr, props::TFlags64 restypeflags = 0, ResTypeFlagMode flagmode = RTFM_IGNORE);

		virtual void RegisterResourceType(const ResourceType *restype);

		virtual void UnregisterResourceType(const ResourceType *restype);

		virtual size_t GetNumResourceTypes() const;

		virtual const ResourceType *GetResourceType(size_t index) const;

		virtual const ResourceType *FindResourceTypeByName(const TCHAR *name) const;

		virtual const ResourceType *FindResourceType(GUID guid) const;

		virtual void RegisterResourceCodec(const ResourceCodec *pcodec);

		virtual void UnregisterResourceCodec(const ResourceCodec *pcodec);

		virtual const ResourceCodec *FindBestCodecByExt(const TCHAR *ext, const ResourceType *restype = nullptr, bool for_write = false) const;

		virtual void BuildExtensionListForType(const ResourceType *restype, tstring &extlist);

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

		virtual System *GetSystem() const;

		void UpdateLastFrameChanged();

	};

};
