// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ResourceImpl.h>


using namespace c3;


ResourceImpl::ResourceImpl(System *psys, const TCHAR *filename, const ResourceType *prestype, const void *data)
{
	m_pSys = psys;
	m_Filename = filename;
	m_pResType = prestype;
	m_Data = (void *)data;
	if (!m_Data)
	{
		m_Status = Resource::Status::RS_NONE;
		m_RefCt = 0;
	}
	else
	{
		m_Status = Resource::Status::RS_LOADED;
		m_RefCt = 1;
	}
}


ResourceImpl::~ResourceImpl()
{
	// Deleting resources while still referencing them? Naughty you.
	assert(m_RefCt == 0);

	if (m_Data)
	{
		if (m_pResType)
			m_pResType->Unload(this);

		m_Data = nullptr;
	}

	m_Status = Resource::Status::RS_NONE;
}


Resource::Status ResourceImpl::GetStatus() const
{
	return m_Status;
}


const ResourceType *ResourceImpl::GetType() const
{
	return m_pResType;
}


const TCHAR *ResourceImpl::GetFilename() const
{
	return m_Filename.c_str();
}


void *ResourceImpl::GetData() const
{
	return m_Data;
}


void ResourceImpl::AddRef()
{
	// if a previous status was an error, don't bother coming in here...
	if (m_Status > Resource::Status::RS_LOADED)
		return;

	if (!m_RefCt)
	{
		if (m_pResType)
		{
			m_pSys->GetLog()->Print(_T("Loading \"%s\" ..."), m_Filename.c_str());

			m_Status = Resource::Status::RS_LOADING;
			ResourceType::LoadResult r = m_pResType->ReadFromFile(m_pSys, m_Filename.c_str(), &m_Data);
			switch (r)
			{
				case ResourceType::LoadResult::LR_NOTFOUND:
					m_Status = Resource::Status::RS_NOTFOUND;
					m_pSys->GetLog()->Print(_T("not found\n"));
					break;

				default:
				case ResourceType::LoadResult::LR_ERROR:
					m_Status = Resource::Status::RS_LOADERROR;
					m_pSys->GetLog()->Print(_T("load error\n"));
					break;

				case ResourceType::LoadResult::LR_SUCCESS:
					m_Status = Resource::Status::RS_LOADED;
					m_pSys->GetLog()->Print(_T("ok\n"));
					break;
			}
		}
	}

	m_RefCt++;
}


void ResourceImpl::DelRef()
{
	if (m_RefCt > 0)
	{
		m_RefCt--;

		if (!m_RefCt)
		{
			if (m_pResType && m_Data && (m_Status == Resource::Status::RS_LOADED))
			{
				m_pResType->Unload(m_Data);
				m_Data = nullptr;
				m_Status = Resource::Status::RS_NONE;
			}
		}
	}
}
