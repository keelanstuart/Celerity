// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3Resource.h>


namespace c3
{
	class ResourceImpl : public Resource
	{
		friend class System;

	protected:

		const ResourceType *m_pResType;
		size_t m_RefCt;
		tstring m_Filename;
		tstring m_Options;
		void *m_Data;
		Status m_Status;
		props::TFlags32 m_AuxFlags;
		uint32_t m_Aux;

	public:

		static System *s_pSys;

		ResourceImpl(const TCHAR *filename, const TCHAR *options, const ResourceType *prestype, const void *data = nullptr);

		virtual ~ResourceImpl();

		virtual Resource::Status GetStatus() const;

		virtual const ResourceType *GetType() const;

		virtual const TCHAR *GetFilename() const;

		virtual const TCHAR *GetOptions() const;

		virtual void *GetData() const;

		// RISKY, BUT SOMETIMES NECESSARY INTERNALLY
		void OverrideData(void *newdata);

		virtual void AddRef();

		virtual void DelRef();

		void SetAux(uint16_t aux, props::TFlags8 flags);

	};
};