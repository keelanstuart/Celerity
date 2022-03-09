// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		props::IPropertySet *m_Props;

		typedef std::deque<Component *> TComponentArray;
		TComponentArray m_Components;

		Object *m_Owner;

		typedef std::deque<Object *> TObjectArray;
		TObjectArray m_Children;

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

		virtual size_t GetNumChildren();

		virtual Object *GetChild(size_t index);

		virtual void AddChild(Object *pchild);

		virtual void RemoveChild(Object *pchild, bool release = false);

		virtual props::TFlags64 &Flags();

		virtual props::IPropertySet *GetProperties();

		virtual size_t GetNumComponents();

		virtual Component *GetComponent(size_t index);

		virtual Component *FindComponent(const ComponentType *pctype);

		virtual Component *AddComponent(const ComponentType *pctype, bool init = true);

		virtual void RemoveComponent(Component *pcomportmemt);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(props::TFlags64 rendflags);

		virtual bool Render(props::TFlags64 rendflags);

		virtual void Postrender(props::TFlags64 rendflags);

		virtual bool Load(genio::IInputStream *is);

		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags);

		virtual void PostLoad();

		virtual void PropertyChanged(const props::IProperty *pprop);

	};

};
