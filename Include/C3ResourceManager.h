// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Resource.h>

namespace c3
{

	class ResourceManager
	{

	public:

		typedef props::TFlags64		ResourceFlags;

		// Flag values for use in GetResource
		#define RESF_DEMANDLOAD			0x00000001			// Loads the Resource immediately in the current thread
		#define RESF_CREATEENTRYONLY	0x00000002			// Creates an empty Resource in the table (or replaces the current resource with data)
		#define RESF_ZIPRES				0x00000004			// Indicates that the resource comes from a zipfile (loads only from memory)
		#define RESF_FINDENTRYONLY		0x00000008			// Only check to see if the resource exists - don't create an new entry or attempt to load anything!
		#define RESF_CACHENATIVE		0x00000010			// For GPU resources, will keep a CPU-side cache


		/// ResTypeFlagMode is used in ForAllResourceDo calls
		using ResTypeFlagMode = enum
		{
			RTFM_IGNORE = 0,		/// Don't use the flags
			RTFM_ANY,				/// Passes if ANY ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here
			RTFM_ALL,				/// Passes only if ALL ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here
			RTFM_NONE				/// Passes if NONE OF the ResType flags (set in your DEFINE_RESOURCETYPE code) match the flags given here

		};

		// resource handling functions
		typedef bool (__cdecl *RESOURCE_CALLBACK_FUNC)(Resource *pres);

		// Returns a resource object.  if the resource needs to be loaded, it can either be
		// demand-loaded (blocking; does it in the same thread) or added to the loading
		// queue (non-blocking; returns "instantly") and brought in by a worker thread.
		// In either case, the resource status should be checked prior to accessing it's
		// data.
		virtual Resource *GetResource(const TCHAR *filename, ResourceFlags flags = 0, const ResourceType *restype = nullptr, const void *data = nullptr) = NULL;

		// For all the resources currently managed by the system, call back into this function. Optionally filter by ResourceType.
		virtual void ForAllResourcesDo(RESOURCE_CALLBACK_FUNC func, const ResourceType *restype = nullptr, props::TFlags64 restypeflags = 0, ResTypeFlagMode flagmode = RTFM_IGNORE) = NULL;

		virtual void RegisterResourceType(const ResourceType *restype) = NULL;

		virtual void UnregisterResourceType(const ResourceType *restype) = NULL;

		virtual size_t GetNumResourceTypes() const = NULL;

		virtual const ResourceType *GetResourceType(size_t index) const = NULL;

		// Finds a resource type that has been previously registered, based on file extension
		// if the same file extension could be used for multiple data types, then 
		virtual const ResourceType *FindResourceTypeByExt(const TCHAR *ext) const = NULL;

		virtual const ResourceType *FindResourceTypeByName(const TCHAR *name) const = NULL;

		virtual const ResourceType *FindResourceType(GUID guid) const = NULL;

		// Deletes all references on all resources, effectively unloading everything
		virtual void Reset() = NULL;

		// Returns the number of zip archives currently registered with the ResourceManager
		virtual size_t GetNumRegisteredZipArchives() const = NULL;

		// Returns the number of zip archives currently registered with the ResourceManager
		virtual const TCHAR *GetRegisteredZipArchivePath(size_t idx) const = NULL;

		// returns true is the given path is registered as a zip archive, false otherwise
		virtual bool IsZipArchiveRegistered(const TCHAR *filename) const = NULL;

		// Register a c3z archive with the ResourceManager, letting you load resources inside
		virtual bool RegisterZipArchive(const TCHAR *filename) = NULL;

		// Remove a c3z archive from the ResourceManager
		virtual void UnregisterZipArchive(const TCHAR *filename) = NULL;

		virtual bool FindZippedFile(const TCHAR *filename, TCHAR *fullpath, size_t fullpathlen) = NULL;

		virtual size_t GetNumResources() = NULL;

		virtual Resource *GetResourceByIndex(size_t index) = NULL;

		virtual uint64_t GetLastFrameChanged() = NULL;

		virtual System *GetSystem() const = NULL;

	};

};
