// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class ResourceType;

	class Resource
	{

	public:

		typedef enum
		{
			RS_NONE = 0,
			RS_LOADED,
			RS_LOADING,
			RS_NOTFOUND,
			RS_LOADERROR,

			RS_NUMSTATUSES

		} Status;

		/// Reports the Status of the Resource (whether it is currently loading, already loaded, or just empty)
		virtual Status GetStatus() const = NULL;

		/// Returns the ResourceType for this Resource
		virtual const ResourceType *GetType() const = NULL;

		/// Returns the filename associated with this Resource
		virtual const TCHAR *GetFilename() const = NULL;

		/// Gets the data associated with this Resource. It is up to you to know what it is.
		virtual void *GetData() const = NULL;

		/// Adds a reference to this Resource
		virtual void AddRef() = NULL;

		/// Removes a reference to thie Resourse
		virtual void DelRef() = NULL;

	};


	class ResourceType
	{

	public:

		typedef enum
		{
			LR_SUCCESS = 0,
			LR_NOTFOUND,
			LR_ERROR,

			LR_NUMRESULTS

		} LoadResult;

		/// Set this flag when you define your ResourceType is you need the underlying data to be loaded in the render thread
		#define RTFLAG_RUNBYRENDERER		0x0001

		/// Returns the flags associated with this ResourceType
		virtual props::TFlags64 Flags() const = NULL;

		/// Returns the name of the ResourceType
		virtual const TCHAR *GetName() const = NULL;

		/// Returns a string that describes the Resource type
		virtual const TCHAR *GetDescription() const = NULL;

		/// Returns the user-defined GUID that identifies the Resource
		virtual GUID GetGUID() const = NULL;

		/// Returns the list of file extensions that are able to be loaded by this ResourceType
		/// This should be a semi-colon-delimited string
		virtual const TCHAR *GetReadableExtensions() const = NULL;

		/// Returns the list of file extensions that are able to be saved by this ResourceType
		/// This should be a semi-colon-delimited string
		virtual const TCHAR *GetSavableExtensions() const = NULL;

		/// Reads and returns a file-based resource
		virtual LoadResult ReadFromFile(c3::System *psys, const TCHAR *filename, void **returned_data) const = NULL;

		/// Reads and returns a memory-based resource
		virtual LoadResult ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, void **returned_data) const = NULL;

		/// Saves the resource to the given file
		virtual bool WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const = NULL;

		/// Releases any resources allocated by this..... Resource
		virtual void Unload(void *data) const = NULL;

	};


	/// This is the standard way of registering your ResourceType
	/// Do your own thing, but beware! These are provided as a way to remind you which methods you need to implement
	/// and fill out the trivial ones for you.

	#define RESOURCETYPENAME(resource_class)	resource_class##ResourceType


	/// THIS GOES IN YOUR HEADER
	#define DEFINE_RESOURCETYPE(resource_class, flags, guid, name, description, loadexts, saveexts)															\
		class RESOURCETYPENAME(resource_class) : public c3::ResourceType																					\
		{																																					\
		public:																																				\
			static RESOURCETYPENAME(resource_class) self;																									\
			static void Register(c3::ResourceManager *resman) {																								\
				if (resman) { resman->RegisterResourceType(&self); } }																						\
			static void Unregister(c3::ResourceManager *resman) {																							\
				if (resman) { resman->UnregisterResourceType(&self); } }																					\
			virtual props::TFlags64 Flags() const { return props::TFlags64(flags); }																		\
			virtual const TCHAR *GetName() const { return _T(name); }																						\
			virtual const TCHAR *GetDescription() const { return _T(description); }																			\
			virtual GUID GetGUID() const { return guid; }																									\
			virtual const TCHAR *GetReadableExtensions() const { return _T(loadexts); }																		\
			virtual const TCHAR *GetSavableExtensions() const { return _T(saveexts); }																		\
			virtual c3::ResourceType::LoadResult ReadFromFile(c3::System *psys, const TCHAR *filename, void **returned_data) const;							\
			virtual c3::ResourceType::LoadResult ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, void **returned_data) const;	\
			virtual bool WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const;														\
			virtual void Unload(void *data) const;																											\
		}


	/// THIS GOES IN YOUR SOURCE
	/// You'll still need to implement Read*, Write*, and Unload methods yourself
	#define DECLARE_RESOURCETYPE(resource_class)																											\
		RESOURCETYPENAME(resource_class) RESOURCETYPENAME(resource_class)::self									


	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
	#define REGISTER_RESOURCETYPE(resource_class, resmanager)	RESOURCETYPENAME(resource_class)::Register(resmanager)


	/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
	#define UNREGISTER_RESOURCETYPE(resource_class, resmanager)	RESOURCETYPENAME(resource_class)::Unregister(resmanager)

}
