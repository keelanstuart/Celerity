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
	m_Flags.SetAll(OBJFLAG_UPDATE | OBJFLAG_DRAW);
	m_Owner = nullptr;
}


ObjectImpl::~ObjectImpl()
{
	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->GetType()->Destroy((*it));
	}
	m_Comportments.clear();

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


Comportment *ObjectImpl::FindComportment(const ComportmentType *pctype)
{
	Comportment *ret = nullptr;

	for (size_t i = 0, maxi = m_Comportments.size(); i < maxi; i++)
	{
		if (m_Comportments[i]->GetType() == pctype)
		{
			ret = m_Comportments[i];
			break;
		}
	}

	return ret;
}


Comportment *ObjectImpl::AddComportment(const ComportmentType *pctype)
{
	if (!pctype)
		return nullptr;

	Comportment *pc = FindComportment(pctype);
	if (pc)
		return pc;

	pc = pctype->Build();
	if (!pc)
		return nullptr;

	m_Comportments.push_back(pc);

	pc->Initialize(this);

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
	if (!m_Flags.IsSet(OBJFLAG_UPDATE))
		return;

	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->Update(this, elapsed_time);
	}
}


bool ObjectImpl::Prerender(props::TFlags64 rendflags)
{
	if (!rendflags.AnySet(m_Flags))
		return false;

	// TODO: port visibility culling
	bool ret = false;

	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		if ((*it)->Prerender(this, rendflags))
			ret = true;
	}

	return ret;
}


bool ObjectImpl::Render(props::TFlags64 rendflags)
{
	if (!rendflags.AnySet(m_Flags))
		return false;

	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->Render(this, rendflags);
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

	for (TComportmentArray::const_iterator it = m_Comportments.cbegin(), last_it = m_Comportments.cend(); it != last_it; it++)
	{
		(*it)->PropertyChanged(pprop);
	}
}
