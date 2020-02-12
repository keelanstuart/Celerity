// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Resource.h>

namespace c3
{

	class ResourceManager
	{

	public:

		enum
		{
			SHIFT_RESFLAG_DEMANDLOAD = 0,
			SHIFT_RESFLAG_CREATEENTRYONLY,
		} ;

		#define RESFLAG_DEMANDLOAD				(1 << c3::ResourceManager::SHIFT_RESFLAG_DEMANDLOAD)				// Loads the Resource immediately in the current thread
		#define RESFLAG_CREATEENTRYONLY			(1 << c3::ResourceManager::SHIFT_RESFLAG_CREATEENTRYONLY)			// Creates an empty Resource in the table

		// resource handling functions
		typedef bool (__cdecl *RESOURCE_CALLBACK_FUNC)(Resource *pres);

		// Returns a resource object.  if the resource needs to be loaded, it can either be
		// demand-loaded (blocking; does it in the same thread) or added to the loading
		// queue (non-blocking; returns "instantly") and brought in by a worker thread.
		// In either case, the resource status should be checked prior to accessing it's
		// data.
		virtual Resource *GetResource(const TCHAR *filename, props::TFlags64 flags = 0, const ResourceType *restype = nullptr) = NULL;

		// For all the resources currently managed by the system, call back into this function. Optionally filter by ResourceType.
		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype = nullptr) = NULL;

		virtual void RegisterResourceType(const ResourceType *restype) = NULL;

		virtual void UnregisterResourceType(const ResourceType *restype) = NULL;

		virtual size_t GetNumResourceTypes() = NULL;

		virtual const ResourceType *GetResourceType(size_t index) = NULL;

		// Finds a resource type that has been previously registered, based on file extension
		// if the same file extension could be used for multiple data types, then 
		virtual const ResourceType *FindResourceType(const TCHAR *ext) = NULL;

		virtual const ResourceType *FindResourceType(GUID guid) = NULL;

		// Deletes all references on all resources, effectively unloading everything
		virtual void Reset() = NULL;

	};

};
