// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3Resource.h>


namespace c3
{
	class ResourceImpl : public Resource
	{
		friend class System;
		friend class ResourceManagerImpl;

	protected:

		const ResourceType *m_pResType;
		const ResourceCodec *m_pCodec;
		size_t m_RefCt;
		tstring m_Filename;
		tstring m_Options;
		void *m_Data;
		Status m_Status;
		props::TFlags32 m_AuxFlags;
		uint32_t m_Aux;

	public:

		static System *s_pSys;

		ResourceImpl(const TCHAR *filename,	const TCHAR *options,
			const ResourceType *prestype, const ResourceCodec *pcodec = nullptr,
			const void *data = nullptr);

		virtual ~ResourceImpl();

		virtual Resource::Status GetStatus() const;

		virtual const ResourceType *GetType() const;

		virtual const ResourceCodec *GetCodec() const;

		virtual void SetCodec(const ResourceCodec *pcodec);

		virtual const TCHAR *GetFilename() const;

		virtual const TCHAR *GetOptions() const;

		virtual void *GetData() const;

		// RISKY, BUT SOMETIMES NECESSARY INTERNALLY
		void OverrideData(void *newdata);

		// optional convenience
		void SetStatus(Status st) { m_Status = st; }

		void SetType(const ResourceType *prestype) { m_pResType = prestype; }

		virtual void AddRef();

		virtual void DelRef();

		void SetAux(uint16_t aux, props::TFlags8 flags);

	};
};
