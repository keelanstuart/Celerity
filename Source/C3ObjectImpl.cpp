// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>
#include <C3PrototypeImpl.h>
#include <C3ObjectImpl.h>

using namespace c3;


ObjectImpl::ObjectImpl(SystemImpl *psys, GUID guid)
{
	m_pSys = psys;
	m_GUID = guid;
	m_Props = props::IPropertySet::CreatePropertySet();
	m_Owner = nullptr;
}


ObjectImpl::~ObjectImpl()
{
	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->GetType()->Destroy((*it));
	}

	if (m_Props)
	{
		m_Props->Release();
		m_Props = nullptr;
	}
}


System *ObjectImpl::GetSystem()
{
	return m_pSys;
}


void ObjectImpl::Release()
{
	delete this;
}


const TCHAR *ObjectImpl::GetName()
{
	return m_Name.c_str();
}


void ObjectImpl::SetName(const TCHAR *name)
{
	m_Name = name ? name : _T("");
}


GUID ObjectImpl::GetGuid()
{
	return m_GUID;
}


Object *ObjectImpl::GetOwner()
{
	return m_Owner;
}


void ObjectImpl::SetOwner(Object *powner)
{
	m_Owner = powner;
}


props::TFlags64 &ObjectImpl::Flags()
{
	return m_Flags;
}


props::IPropertySet *ObjectImpl::GetProperties()
{
	return m_Props;
}


size_t ObjectImpl::GetNumComportments()
{
	return m_Comportments.size();
}


Comportment *ObjectImpl::GetComportment(size_t index)
{
	if (index >= m_Comportments.size())
		return nullptr;

	return m_Comportments[index];
}


Comportment *ObjectImpl::AddComportment(ComportmentType *pctype)
{
	if (pctype)
		return nullptr;

	Comportment *pc = pctype->Build(this);
	if (!pc)
		return nullptr;

	m_Comportments.push_back(pc);

	return pc;
}


void ObjectImpl::RemoveComportment(Comportment *pcomportmemt)
{
	TComportmentArray::iterator it = std::find(m_Comportments.begin(), m_Comportments.end(), pcomportmemt);
	if (it != m_Comportments.end())
	{
		(*it)->GetType()->Destroy((*it));
		m_Comportments.erase(it);
	}
}


void ObjectImpl::Update(float elapsed_time)
{
	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->Update(elapsed_time);
	}
}


bool ObjectImpl::Prerender(props::TFlags64 rendflags)
{
	// TODO: port visibility culling
	return true;
}


bool ObjectImpl::Render(props::TFlags64 rendflags)
{
	return true;
}


void ObjectImpl::Postrender(props::TFlags64 rendflags)
{

}


bool ObjectImpl::Load(genio::IInputStream *is)
{
	if (!is)
		return false;

	return true;
}


bool ObjectImpl::Save(genio::IOutputStream *os, props::TFlags64 saveflags)
{
	if (!os)
		return false;

	return true;
}


void ObjectImpl::PostLoad()
{

}


void ObjectImpl::PropertyChanged(const props::IPropertySet *ppropset, const props::IProperty *pprop)
{
	if (!pprop)
		return;
}
