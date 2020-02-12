// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

namespace c3
{

	class FileMapper
	{

	public:

		/// Returns the number of unique extension maps you have registered
		virtual size_t GetNumMappings() = NULL;

		/// Returns information about the mapping at the given index
		virtual const TCHAR *GetMapping(size_t mappingidx) = NULL;

		/// Finds the index of the mapping for the given extension
		virtual bool FindMapping(const TCHAR *extension, size_t *idx) = NULL;

		/// Gets the number of paths that have been registered for the given extension
		virtual size_t GetNumPaths(const TCHAR *extension) = NULL;

		/// Gets the path at the given index for the given extension
		virtual const TCHAR *GetPath(const TCHAR *extension, size_t pathidx) = NULL;

		/// Add the path to the mapping for whatever extension you want
		virtual void AddMapping(const TCHAR *extension, const TCHAR *path) = NULL;

		/// removes the path to the mapping for whatever extension you want
		virtual void RemoveMapping(const TCHAR *extension, const TCHAR *path) = NULL;

		/// Supply a filename only and receive a fully qualified path in return, based on the
		/// file extension mappings you have previously set up.
		/// Returns true if the file was found
		virtual bool FindFile(const TCHAR *filename, TCHAR *fullpath = NULL, size_t fullpathlen = 0) = NULL;

		/// this functions sets the path mappings for multiple extensions
		/// for example, you can set the path mappings for all texture file extensions at once...
		/// exts = "jpg;png;bmp;dds;tga;pcx;gif"
		/// paths = "./textures;./;../data/textures"
		virtual void SetMappingsFromDelimitedStrings(const TCHAR *exts, const TCHAR *paths, const TCHAR delimiter = ';') = NULL;

	};

};
