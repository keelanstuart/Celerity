// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3PrototypeImpl.h>
#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>

using namespace c3;


PrototypeImpl::PrototypeImpl(System *psys, GUID guid)
{
	m_pSys = psys;
	m_GUID = guid;
	m_Flags.SetAll(OBJFLAG_UPDATE | OBJFLAG_DRAW);

	m_Props = props::IPropertySet::CreatePropertySet();
}


PrototypeImpl::~PrototypeImpl()
{
	m_Comportments.clear();

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


props::TFlags64 &PrototypeImpl::Flags()
{
	return m_Flags;
}


props::IPropertySet *PrototypeImpl::GetProperties()
{
	return m_Props;
}


bool PrototypeImpl::AddComportment(const ComportmentType *pcomptype)
{
	if (!pcomptype)
		return false;

	if (std::find(m_Comportments.cbegin(), m_Comportments.cend(), pcomptype) != m_Comportments.cend())
		return false;

	m_Comportments.push_back(pcomptype);

	return true;
}


bool PrototypeImpl::RemoveComportment(const ComportmentType *pcomp)
{
	TComportmentArray::iterator it = std::find(m_Comportments.begin(), m_Comportments.end(), pcomp);
	if (it != m_Comportments.end())
	{
		m_Comportments.erase(it);

		return true;
	}

	return false;
}


size_t PrototypeImpl::GetNumComportments()
{
	return m_Comportments.size();
}


const ComportmentType *PrototypeImpl::GetComportment(size_t index)
{
	if (index >= m_Comportments.size())
		return nullptr;

	return m_Comportments[index];
}
