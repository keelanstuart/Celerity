// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"

#include <C3PrototypeImpl.h>
#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>

using namespace c3;


PrototypeImpl::PrototypeImpl(System *psys, GUID guid)
{
	m_pSys = psys;
	m_GUID = guid;
	m_Flags.SetAll(OF_UPDATE | OF_DRAW);

	m_Props = props::IPropertySet::CreatePropertySet();
}


PrototypeImpl::~PrototypeImpl()
{
	m_Components.clear();

	if (m_Props)
	{
		m_Props->Release();
		m_Props = nullptr;
	}
}


const TCHAR *PrototypeImpl::GetName()
{
	return m_Name.c_str();
}


void PrototypeImpl::SetName(const TCHAR *name)
{
	m_Name = name ? name : _T("");
}


GUID PrototypeImpl::GetGUID()
{
	return m_GUID;
}


const TCHAR *PrototypeImpl::GetGroup()
{
	return m_Group.c_str();
}


void PrototypeImpl::SetGroup(const TCHAR *group)
{
	m_Group = group ? group : _T("");
}


props::TFlags64 &PrototypeImpl::Flags()
{
	return m_Flags;
}


props::IPropertySet *PrototypeImpl::GetProperties()
{
	return m_Props;
}


bool PrototypeImpl::AddComponent(const ComponentType *pcomptype)
{
	if (!pcomptype)
		return false;

	if (std::find(m_Components.cbegin(), m_Components.cend(), pcomptype) != m_Components.cend())
		return false;

	m_Components.push_back(pcomptype);

	return true;
}


bool PrototypeImpl::RemoveComponent(const ComponentType *pcomp)
{
	TComponentArray::iterator it = std::find(m_Components.begin(), m_Components.end(), pcomp);
	if (it != m_Components.end())
	{
		m_Components.erase(it);

		return true;
	}

	return false;
}


bool PrototypeImpl::HasComponent(const ComponentType *pcomp)
{
	TComponentArray::iterator it = std::find(m_Components.begin(), m_Components.end(), pcomp);
	if (it != m_Components.end())
	{
		return true;
	}

	return false;
}


size_t PrototypeImpl::GetNumComponents()
{
	return m_Components.size();
}


const ComponentType *PrototypeImpl::GetComponent(size_t index)
{
	if (index >= m_Components.size())
		return nullptr;

	return m_Components[index];
}
