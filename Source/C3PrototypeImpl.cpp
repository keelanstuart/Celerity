// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3PrototypeImpl.h>
#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>

using namespace c3;


PrototypeImpl::PrototypeImpl(System *psys, GUID guid)
{
	m_pSys = psys;
	m_GUID = guid;
	m_Flags.SetAll(c3::Object::OBJFLAG(c3::Object::UPDATE) | c3::Object::OBJFLAG(c3::Object::DRAW));

	m_Props = props::IPropertySet::CreatePropertySet();
}


PrototypeImpl::~PrototypeImpl()
{
	m_Features.clear();

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


bool PrototypeImpl::AddFeature(const FeatureType *pcomptype)
{
	if (!pcomptype)
		return false;

	if (std::find(m_Features.cbegin(), m_Features.cend(), pcomptype) != m_Features.cend())
		return false;

	m_Features.push_back(pcomptype);

	return true;
}


bool PrototypeImpl::RemoveFeature(const FeatureType *pcomp)
{
	TFeatureArray::iterator it = std::find(m_Features.begin(), m_Features.end(), pcomp);
	if (it != m_Features.end())
	{
		m_Features.erase(it);

		return true;
	}

	return false;
}


size_t PrototypeImpl::GetNumFeatures()
{
	return m_Features.size();
}


const FeatureType *PrototypeImpl::GetFeature(size_t index)
{
	if (index >= m_Features.size())
		return nullptr;

	return m_Features[index];
}
