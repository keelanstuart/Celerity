// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Blob.h>


namespace c3
{

	class BlobImpl : public Blob
	{

	public:

		BlobImpl();

		virtual ~BlobImpl();

		virtual const uint8_t *Data() const;

		virtual size_t Size() const;

		virtual void Release();

		std::vector<uint8_t> m_Data;

	};

	DEFINE_RESOURCETYPE(Blob, 0,
		GUID({ 0x1b2cec8e, 0x59e1, 0x4410, { 0xb5, 0xd9, 0x70, 0x5e, 0x12, 0xbc, 0xf8, 0x16 } }),
		"Blob", "Raw Data");

	DEFINE_RESOURCECODEC(Blobs, Blob, 0,
		GUID({ 0xb5c70b31, 0xbf7d, 0x40a5, { 0x94, 0x3f, 0x50, 0x85, 0x8, 0xb5, 0x8c, 0xf6 } }),
		"Blob de/serializer", "Loads and saves raw data",
		"txt;c3o", "txt;c3o", 0);

};
