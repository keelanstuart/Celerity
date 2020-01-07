// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class ObjectImpl : public Object
	{
		SystemImpl *m_pSys;
		tstring m_Name;
		GUID m_GUID;
		props::TFlags64 m_Flags;

		Object *m_Owner;

		props::IPropertySet *m_Props;

		typedef std::deque<Comportment *> TComportmentArray;
		TComportmentArray m_Comportments;

	public:

		ObjectImpl(SystemImpl *psys, GUID guid);

		virtual ~ObjectImpl();

		virtual System *GetSystem();

		virtual void Release();

		virtual const TCHAR *GetName();

		virtual void SetName(const TCHAR *name);

		virtual GUID GetGuid();

		virtual Object *GetOwner();

		virtual void SetOwner(Object *powner);

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual size_t GetNumComportments();

		virtual Comportment *GetComportment(size_t index);

		virtual Comportment *AddComportment(ComportmentType *pctype);

		virtual void RemoveComportment(Comportment *pcomportmemt);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(props::TFlags64 rendflags);

		virtual bool Render(props::TFlags64 rendflags);

		virtual void Postrender(props::TFlags64 rendflags);

		virtual bool Load(genio::IInputStream *is);

		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags);

		virtual void PostLoad();

		virtual void PropertyChanged(const props::IPropertySet *ppropset, const props::IProperty *pprop);

	};

};
