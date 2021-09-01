// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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
	m_Flags.SetAll(OBJFLAG(UPDATE) | OBJFLAG(DRAW));
	m_Owner = nullptr;
}


ObjectImpl::~ObjectImpl()
{
	for (const auto &it : m_Features)
	{
		it->GetType()->Destroy(it);
	}
	m_Features.clear();

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


size_t ObjectImpl::GetNumChildren()
{
	return m_Children.size();
}


Object *ObjectImpl::GetChild(size_t index)
{
	if (index >= m_Children.size())
		return nullptr;

	return m_Children[index];
}


void ObjectImpl::AddChild(Object *pchild)
{
	if (!pchild)
		return;

	if (std::find(m_Children.cbegin(), m_Children.cend(), pchild) != m_Children.cend())
	{
		m_Children.push_back(pchild);

		pchild->SetOwner(this);
	}
}


props::TFlags64 &ObjectImpl::Flags()
{
	return m_Flags;
}


props::IPropertySet *ObjectImpl::GetProperties()
{
	return m_Props;
}


size_t ObjectImpl::GetNumFeatures()
{
	return m_Features.size();
}


Feature *ObjectImpl::GetFeature(size_t index)
{
	if (index >= m_Features.size())
		return nullptr;

	return m_Features[index];
}


Feature *ObjectImpl::FindFeature(const FeatureType *pctype)
{
	Feature *ret = nullptr;

	for (size_t i = 0, maxi = m_Features.size(); i < maxi; i++)
	{
		if (m_Features[i]->GetType() == pctype)
		{
			ret = m_Features[i];
			break;
		}
	}

	return ret;
}


Feature *ObjectImpl::AddFeature(const FeatureType *pctype)
{
	if (!pctype)
		return nullptr;

	Feature *pc = FindFeature(pctype);
	if (pc)
		return pc;

	pc = pctype->Build();
	if (!pc)
		return nullptr;

	m_Features.push_back(pc);

	pc->Initialize(this);

	return pc;
}


void ObjectImpl::RemoveFeature(Feature *pcomportmemt)
{
	TFeatureArray::iterator it = std::find(m_Features.begin(), m_Features.end(), pcomportmemt);
	if (it != m_Features.end())
	{
		(*it)->GetType()->Destroy((*it));
		m_Features.erase(it);
	}
}


void ObjectImpl::Update(float elapsed_time)
{
	if (!m_Flags.IsSet(OBJFLAG(UPDATE)))
		return;

	for (const auto &it : m_Features)
	{
		it->Update(this, elapsed_time);
	}
}


bool ObjectImpl::Prerender(props::TFlags64 rendflags)
{
	if (!rendflags.AnySet(m_Flags))
		return false;

	// TODO: port visibility culling
	bool ret = false;

	for (const auto it : m_Features)
	{
		if (it->Prerender(this, rendflags))
			ret = true;
	}

	return ret;
}


bool ObjectImpl::Render(props::TFlags64 rendflags)
{
	if (!rendflags.AnySet(m_Flags))
		return false;

	for (const auto &it : m_Features)
	{
		it->Render(this, rendflags);
	}

	return true;
}


void ObjectImpl::Postrender(props::TFlags64 rendflags)
{
	if (!rendflags.AnySet(m_Flags))
		return;
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


void ObjectImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	for (const auto &it : m_Features)
	{
		it->PropertyChanged(pprop);
	}
}
