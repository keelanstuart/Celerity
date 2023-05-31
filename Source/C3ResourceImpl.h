// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3Resource.h>

namespace c3
{
	class ResourceImpl : public Resource
	{
		friend class System;

	protected:

#pragma pack(push, 1)
		const ResourceType *m_pResType;
		size_t m_RefCt;
		tstring m_Filename;
		tstring m_Options;
		void *m_Data;
		Status m_Status;
		props::TFlags32 m_AuxFlags;
		uint32_t m_Aux;
#pragma pack(pop)

	public:

		static System *s_pSys;

		ResourceImpl(const TCHAR *filename, const TCHAR *options, const ResourceType *prestype, const void *data = nullptr);

		virtual ~ResourceImpl();

		virtual Resource::Status GetStatus() const;
		//virtual void SetStatus(Resource::Status status);

		virtual const ResourceType *GetType() const;

		virtual const TCHAR *GetFilename() const;

		virtual const TCHAR *GetOptions() const;

		virtual void *GetData() const;
		//virtual void SetData(void *data);

		virtual void AddRef();

		virtual void DelRef();

		void SetAux(uint16_t aux, props::TFlags8 flags);

	};
};