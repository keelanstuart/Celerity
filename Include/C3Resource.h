// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <c3.h>


namespace c3
{
	class System;
	class ResourceManager;
	class ResourceType;
	class ResourceCodec;

	class Resource
	{

	public:

		using Status = enum
		{
			RS_NONE = 0,
			RS_LOADED,
			RS_LOADING,
			RS_NOTFOUND,
			RS_LOADERROR,
			RS_NUMSTATUSES
		};

		/// Reports the Status of the Resource (whether it is currently loading, already loaded, or just empty)
		virtual Status GetStatus() const = NULL;

		/// Semantic / runtime type: Model, Texture, Sound, Blob...
		virtual const ResourceType *GetType() const = NULL;

		/// Actual codec that loaded/saves this resource
		virtual const ResourceCodec *GetCodec() const = NULL;
		virtual void SetCodec(const ResourceCodec *pcodec) = NULL;

		/// Returns the filename associated with this Resource
		virtual const TCHAR *GetFilename() const = NULL;

		/// Returns the options associated with this Resource
		virtual const TCHAR *GetOptions() const = NULL;

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

		#define RTFLAG_RUNBYRENDERER 0x0001

		virtual props::TFlags64 Flags() const = NULL;
		virtual const TCHAR *GetName() const = NULL;
		virtual const TCHAR *GetDescription() const = NULL;
		virtual GUID GetGUID() const = NULL;

		// Runtime object lifecycle belongs to the semantic type.
		virtual void DestroyData(void *data) const = NULL;

	};

	class ResourceCodec
	{

	public:

		using LoadResult = enum
		{
			LR_SUCCESS = 0,
			LR_NOTFOUND,
			LR_ERROR,
			LR_NOCODEC,
			LR_NUMRESULTS
		};

		using CodecFlags = enum
		{
			CCF_NONE       = 0x0000,
			CCF_CANREAD    = 0x0001,
			CCF_CANWRITE   = 0x0002,
			CCF_DEFAULT    = 0x0004,
			CCF_SNIFFABLE  = 0x0008,
			CCF_NOMEMREAD  = 0x0010
		};

		virtual props::TFlags64 Flags() const = NULL;

		virtual const TCHAR *GetName() const = NULL;
		virtual const TCHAR *GetDescription() const = NULL;
		virtual GUID GetGUID() const = NULL;

		virtual const ResourceType *GetResourceType() const = NULL;

		/// Returns the list of file extensions that are able to be loaded by this ResourceType
		/// This should be a semi-colon-delimited string
		virtual const TCHAR *GetReadableExtensions() const = NULL;
		virtual const TCHAR *GetSavableExtensions() const = NULL;

		// optional priority when multiple codecs claim same extension
		virtual int GetPriority() const = NULL;

		// optional content sniff
		virtual bool CanReadMemory(const BYTE *buffer, size_t buffer_length) const = NULL;

		virtual LoadResult ReadFromFile(
			System *psys,
			const TCHAR *filename,
			const TCHAR *options,
			void **returned_data) const = NULL;

		virtual LoadResult ReadFromMemory(
			System *psys,
			const TCHAR *contextname,
			const BYTE *buffer,
			size_t buffer_length,
			const TCHAR *options,
			void **returned_data) const = NULL;

		virtual bool WriteToFile(
			System *psys,
			const TCHAR *filename,
			const void *data) const = NULL;
	};

	#define RESOURCETYPENAME(resource_class) resource_class##ResourceType
	#define RESOURCETYPE(resource_class) (&(RESOURCETYPENAME(resource_class)::self))

	#define RESOURCECODECNAME(codec_class) codec_class##ResourceCodec
	#define RESOURCECODEC(codec_class) (&(RESOURCECODECNAME(codec_class)::self))

	// semantic/runtime type
	#define DEFINE_RESOURCETYPE(resource_class, flags, guid, name, description) \
		class RESOURCETYPENAME(resource_class) : public c3::ResourceType \
		{ \
			public: \
			static RESOURCETYPENAME(resource_class) self; \
			static const c3::ResourceType *Type() { return (const c3::ResourceType *)&self; } \
			static void RegisterResourceType(c3::ResourceManager *resman) { \
				if (resman) { resman->RegisterResourceType(&self); } } \
			static void UnregisterResourceType(c3::ResourceManager *resman) { \
				if (resman) { resman->UnregisterResourceType(&self); } } \
			virtual props::TFlags64 Flags() const { return props::TFlags64(flags); } \
			virtual const TCHAR *GetName() const { return _T(name); } \
			virtual const TCHAR *GetDescription() const { return _T(description); } \
			virtual GUID GetGUID() const { return guid; } \
			virtual void DestroyData(void *data) const; \
		}


	/// THIS GOES IN YOUR SOURCE
	/// You'll still need to implement DestroyData
	#define DECLARE_RESOURCETYPE(resource_class) \
		RESOURCETYPENAME(resource_class) RESOURCETYPENAME(resource_class)::self

	/// THIS GOES IN YOUR SOURCE
	/// Format codec associated with a semantic type
	/// You'll still need to implement ReadFromFile, ReadFromMemory, and WriteToFile
	#define DEFINE_RESOURCECODEC(codec_class, resource_class, flags, guid, name, description, loadexts, saveexts, priority) \
		class RESOURCECODECNAME(codec_class) : public c3::ResourceCodec \
		{ \
		public: \
			static RESOURCECODECNAME(codec_class) self; \
			static const c3::ResourceCodec *Codec() { return (const c3::ResourceCodec *)&self; } \
			static void RegisterResourceCodec(c3::ResourceManager *resman) { \
				if (resman) { resman->RegisterResourceCodec(&self); } } \
			static void UnregisterResourceCodec(c3::ResourceManager *resman) { \
				if (resman) { resman->UnregisterResourceCodec(&self); } } \
			virtual props::TFlags64 Flags() const { return props::TFlags64(flags); } \
			virtual const TCHAR *GetName() const { return _T(name); } \
			virtual const TCHAR *GetDescription() const { return _T(description); } \
			virtual GUID GetGUID() const { return guid; } \
			virtual const c3::ResourceType *GetResourceType() const { return RESOURCETYPE(resource_class); } \
			virtual const TCHAR *GetReadableExtensions() const { return _T(loadexts); } \
			virtual const TCHAR *GetSavableExtensions() const { return _T(saveexts); } \
			virtual int GetPriority() const { return priority; } \
			virtual bool CanReadMemory(const BYTE *buffer, size_t buffer_length) const { \
				return !(flags & c3::ResourceCodec::CCF_NOMEMREAD); } \
			virtual c3::ResourceCodec::LoadResult ReadFromFile( \
				c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const; \
			virtual c3::ResourceCodec::LoadResult ReadFromMemory( \
				c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const; \
			virtual bool WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const; \
		}

	#define DECLARE_RESOURCECODEC(codec_class) \
		RESOURCECODECNAME(codec_class) RESOURCECODECNAME(codec_class)::self

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
	#define REGISTER_RESOURCETYPE(resource_class, resmanager) \
			RESOURCETYPENAME(resource_class)::RegisterResourceType(resmanager)

	/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
	#define UNREGISTER_RESOURCETYPE(resource_class, resmanager) \
			RESOURCETYPENAME(resource_class)::UnregisterResourceType(resmanager)

	/// DO THIS AFTER YOU CALL c3::System::Create OR WHEN YOUR PLUG-IN IS INITIALIZED
	#define REGISTER_RESOURCECODEC(codec_class, resmanager) \
			RESOURCECODECNAME(codec_class)::RegisterResourceCodec(resmanager)

	/// DO THIS WHEN YOU UNLOAD YOUR PLUGIN OR BEFORE YOU CALL c3::System::Release
	#define UNREGISTER_RESOURCECODEC(codec_class, resmanager) \
			RESOURCECODECNAME(codec_class)::UnregisterResourceCodec(resmanager)
}