// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>


namespace c3
{

	/// <summary>
	/// Blob is intended as a generic resource type.
	/// Want to load some text from a file? An image that's not a texture? Use Blob.
	/// It goes through the ResourceManager to load asynchronously and from zip files, etc.
	/// Do this by explicitly providing the resource type, like so:
	///   auto pres = psys->GetResourceManager()->GetResource(_T("myfile.foo"), 0, RESTYPE(Blob));
	/// </summary>
	class Blob
	{

	public:

		virtual const uint8_t *Data() const = NULL;

		virtual size_t Size() const = NULL;

		virtual void Release() = NULL;

	};

};