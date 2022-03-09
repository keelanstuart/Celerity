// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3Resource.h>

namespace c3
{
	class ResourceImpl : public Resource
	{

	protected:
		System *m_pSys;
		const ResourceType *m_pResType;
		size_t m_RefCt;
		tstring m_Filename;
		void *m_Data;
		Status m_Status;

	public:

		ResourceImpl(System *psys, const TCHAR *filename, const ResourceType *prestype, void *data = nullptr);

		virtual ~ResourceImpl();

		virtual Resource::Status GetStatus() const;
		//virtual void SetStatus(Resource::Status status);

		virtual const ResourceType *GetType() const;

		virtual const TCHAR *GetFilename() const;

		virtual void *GetData() const;
		//virtual void SetData(void *data);

		virtual void AddRef();

		virtual void DelRef();

	};
};