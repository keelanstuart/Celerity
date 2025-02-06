// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


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
		tstring m_Source;

		props::TFlags64 m_Flags;

		props::IPropertySet *m_Props;

		typedef std::deque<const ComponentType *> TComponentArray;
		TComponentArray m_Components;

	public:

		PrototypeImpl(System *psys, GUID guid, const TCHAR *source = nullptr);

		virtual ~PrototypeImpl();

		virtual const TCHAR *GetName() const;

		virtual void SetName(const TCHAR *name);

		virtual GUID GetGUID() const;

		virtual const TCHAR *GetGroup() const;

		virtual void SetGroup(const TCHAR *group);

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual bool AddComponent(const ComponentType *pcomp);

		virtual bool RemoveComponent(const ComponentType *pcomp);

		virtual bool HasComponent(const ComponentType *pcomp) const;

		virtual size_t GetNumComponents() const;

		virtual const ComponentType *GetComponent(size_t index) const;

		virtual const TCHAR *GetSource() const;

	};

};