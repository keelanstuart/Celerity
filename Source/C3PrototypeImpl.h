// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#pragma once

#include <C3.h>

namespace c3
{

	class PrototypeImpl : public Prototype
	{

	protected:
		System *m_pSys;
		tstring m_Name;
		tstring m_Group;
		GUID m_GUID;

		props::TFlags64 m_Flags;

		props::IPropertySet *m_Props;

		typedef std::deque<const ComponentType *> TComponentArray;
		TComponentArray m_Components;

	public:

		PrototypeImpl(System *psys, GUID guid);

		virtual ~PrototypeImpl();

		virtual const TCHAR *GetName();

		virtual void SetName(const TCHAR *name);

		virtual GUID GetGUID();

		virtual const TCHAR *GetGroup();

		virtual void SetGroup(const TCHAR *group);

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual bool AddComponent(const ComponentType *pcomp);

		virtual bool RemoveComponent(const ComponentType *pcomp);

		virtual size_t GetNumComponents();

		virtual const ComponentType *GetComponent(size_t index);

	};

};