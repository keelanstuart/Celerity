// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#include "pch.h"

#include <C3BlobImpl.h>
#include <C3Resource.h>


using namespace c3;


DECLARE_RESOURCETYPE(Blob);

c3::ResourceType::LoadResult RESOURCETYPENAME(Blob)::ReadFromFile(c3::System *psys, const TCHAR *filename, const TCHAR *options, void **returned_data) const
{
	ResourceType::LoadResult ret = ResourceType::LoadResult::LR_ERROR;

	if (returned_data)
	{
		*returned_data = nullptr;

		HANDLE h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			BlobImpl *pblob = new BlobImpl();
			*returned_data = pblob;

			LARGE_INTEGER fs;
			fs.LowPart = GetFileSize(h, (LPDWORD)&(fs.HighPart));

			pblob->m_Data.resize(fs.QuadPart);

			DWORD cb;

			// address this file size limit later???
			ReadFile(h, pblob->m_Data.data(), fs.LowPart, &cb, nullptr);
			CloseHandle(h);

			ret = ResourceType::LoadResult::LR_SUCCESS;
		}
	}

	return ret;
}

c3::ResourceType::LoadResult RESOURCETYPENAME(Blob)::ReadFromMemory(c3::System *psys, const TCHAR *contextname, const BYTE *buffer, size_t buffer_length, const TCHAR *options, void **returned_data) const
{
	if (returned_data)
	{
		BlobImpl *pblob = new BlobImpl();
		*returned_data = pblob;

		pblob->m_Data.resize(buffer_length);

		memcpy(pblob->m_Data.data(), buffer, buffer_length);
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}

bool RESOURCETYPENAME(Blob)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	bool ret = false;

	BlobImpl *pblob = dynamic_cast<BlobImpl *>((BlobImpl *)data);
	if (pblob)
	{
		HANDLE h = CreateFile(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD cb;

			// address this file size limit later???
			ret = WriteFile(h, pblob->Data(), (DWORD)pblob->Size(), &cb, nullptr);
			CloseHandle(h);
		}
	}

	return ret;
}


void RESOURCETYPENAME(Blob)::Unload(void *data) const
{
	((Blob *)data)->Release();
}


BlobImpl::BlobImpl()
{
}


BlobImpl::~BlobImpl()
{
}


const uint8_t *BlobImpl::Data() const
{
	return m_Data.data();
}


size_t BlobImpl::Size() const
{
	return m_Data.size();
}


void BlobImpl::Release()
{
	delete this;
}
