// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class PrototypeImpl : public Prototype
	{

	protected:
		System *m_pSys;
		tstring m_Name;
		GUID m_GUID;

		props::TFlags64 m_Flags;

		props::IPropertySet *m_Props;

		typedef std::deque<Comportment *> TComportmentArray;
		TComportmentArray m_Comportments;

	public:

		PrototypeImpl(System *psys, GUID guid);

		virtual ~PrototypeImpl();

		virtual const TCHAR *GetName();

		virtual void SetName(const TCHAR *name);

		virtual GUID GetGUID();

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual bool AddComportment(Comportment *pcomp);

		virtual bool RemoveComportment(Comportment *pcomp);

		virtual size_t GetNumComportments();

		virtual Comportment *GetComportment(size_t index);

	};

};