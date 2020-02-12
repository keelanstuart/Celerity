// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3ResourceManager.h>

namespace c3
{

	class ResourceManagerImpl : public ResourceManager
	{

	protected:
		System *m_pSys;

		typedef std::map<tstring, Resource *, std::less<tstring>> TResourceMap;
		TResourceMap m_ResMap;

		typedef std::multimap<const ResourceType *, Resource *> TResourceByTypeMap;
		TResourceByTypeMap m_ResByTypeMap;

		typedef std::deque<const ResourceType *> TResourceTypeArray;
		TResourceTypeArray m_ResTypes;

		typedef std::multimap<tstring, const ResourceType *> TExtToResourceTypeMap;
		TExtToResourceTypeMap m_ExtResTypeMap;


	public:

		ResourceManagerImpl(System *psys);

		virtual ~ResourceManagerImpl();

		static void __cdecl LoadingThreadProc(LPVOID presmanimpl, LPVOID pres, size_t task_number);

		virtual Resource *GetResource(const TCHAR *filename, props::TFlags64 flags, const ResourceType *restype);

		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype);

		virtual void RegisterResourceType(const ResourceType *restype);

		virtual void UnregisterResourceType(const ResourceType *restype);

		virtual size_t GetNumResourceTypes();

		virtual const ResourceType *GetResourceType(size_t index);

		virtual const ResourceType *FindResourceType(const TCHAR *ext);

		virtual const ResourceType *FindResourceType(GUID guid);

		virtual void Reset();

	};

};
