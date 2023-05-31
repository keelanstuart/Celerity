// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ResourceImpl.h>
#include <C3ResourceManagerImpl.h>
#include <C3Zip.h>


using namespace c3;


// this should be set when the ResourceManager is created
System *ResourceImpl::s_pSys = nullptr;


ResourceImpl::ResourceImpl(const TCHAR *filename, const TCHAR *options, const ResourceType *prestype, const void *data)
{
	m_Filename = filename;
	m_Options = options;
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
	return (Resource::Status)m_Status;
}


const ResourceType *ResourceImpl::GetType() const
{
	return m_pResType;
}


const TCHAR *ResourceImpl::GetFilename() const
{
	return m_Filename.c_str();
}


const TCHAR *ResourceImpl::GetOptions() const
{
	return m_Options.c_str();
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
			s_pSys->GetLog()->Print(_T("Loading \"%s\" ..."), m_Filename.c_str());

			m_Status = Resource::Status::RS_LOADING;
			ResourceType::LoadResult r;
			if (!m_AuxFlags)
				r = m_pResType->ReadFromFile(s_pSys, m_Filename.c_str(), m_Options.c_str(), &m_Data);
			else
			{
				if (m_AuxFlags.IsSet(RESF_ZIPRES))
				{
					ResourceManagerImpl *prmi = (ResourceManagerImpl *)s_pSys->GetResourceManager();
					const ZipFile *pzf = prmi->GetZipFile(m_Aux);
					if (pzf)
					{
						size_t zfi = pzf->FindFileIndex(m_Filename.c_str());

						void *addr = nullptr;
						size_t len = 0;
						if (pzf->GetContent(zfi, &addr, &len))
						{
							r = m_pResType->ReadFromMemory(s_pSys, (const BYTE *)addr, len, m_Options.c_str(), &m_Data);
						}
						else
						{
							r = ResourceType::LoadResult::LR_ERROR;
						}
					}
				}
			}

			switch (r)
			{
				case ResourceType::LoadResult::LR_NOTFOUND:
					m_Status = Resource::Status::RS_NOTFOUND;
					s_pSys->GetLog()->Print(_T("not found\n"));
					break;

				default:
				case ResourceType::LoadResult::LR_ERROR:
					m_Status = Resource::Status::RS_LOADERROR;
					s_pSys->GetLog()->Print(_T("load error\n"));
					break;

				case ResourceType::LoadResult::LR_SUCCESS:
					m_Status = Resource::Status::RS_LOADED;
					s_pSys->GetLog()->Print(_T("ok\n"));
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


void ResourceImpl::SetAux(uint16_t aux, props::TFlags8 flags)
{
	m_Aux = aux;
	m_AuxFlags = flags;
}
