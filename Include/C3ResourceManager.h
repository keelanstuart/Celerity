// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Resource.h>

namespace c3
{

	class ResourceManager
	{

	public:

		typedef enum
		{
			DEMANDLOAD = 0,			// Loads the Resource immediately in the current thread
			CREATEENTRYONLY,		// Creates an empty Resource in the table
		} EResFlag;

		static constexpr uint64_t RESFLAG(EResFlag f) { return (1LL << (uint64_t)f); }


		/// ResTypeFlagMode is used in ForAllResourceDo calls
		typedef enum
		{
			RTFM_IGNORE = 0,		/// Don't use the flags
			RTFM_ANY,				/// Passes if ANY ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here
			RTFM_ALL,				/// Passes only if ALL ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here
			RTFM_NONE				/// Passes if NONE OF the ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here

		} ResTypeFlagMode;


		// resource handling functions
		typedef bool (__cdecl *RESOURCE_CALLBACK_FUNC)(Resource *pres);

		// Returns a resource object.  if the resource needs to be loaded, it can either be
		// demand-loaded (blocking; does it in the same thread) or added to the loading
		// queue (non-blocking; returns "instantly") and brought in by a worker thread.
		// In either case, the resource status should be checked prior to accessing it's
		// data.
		virtual Resource *GetResource(const TCHAR *filename, props::TFlags64 flags = 0, const ResourceType *restype = nullptr) = NULL;

		// For all the resources currently managed by the system, call back into this function. Optionally filter by ResourceType.
		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype = nullptr, props::TFlags64 restypeflags = 0, ResTypeFlagMode flagmode = RTFM_IGNORE) = NULL;

		virtual void RegisterResourceType(const ResourceType *restype) = NULL;

		virtual void UnregisterResourceType(const ResourceType *restype) = NULL;

		virtual size_t GetNumResourceTypes() const = NULL;

		virtual const ResourceType *GetResourceType(size_t index) const = NULL;

		// Finds a resource type that has been previously registered, based on file extension
		// if the same file extension could be used for multiple data types, then 
		virtual const ResourceType *FindResourceType(const TCHAR *ext) const = NULL;

		virtual const ResourceType *FindResourceType(GUID guid) const = NULL;

		// Deletes all references on all resources, effectively unloading everything
		virtual void Reset() = NULL;

	};

};
