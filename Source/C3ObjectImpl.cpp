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
	m_Props->SetChangeListener(this);
	m_Flags.SetAll(OBJFLAG(UPDATE) | OBJFLAG(DRAW));
	m_Owner = nullptr;
}


ObjectImpl::~ObjectImpl()
{
	for (const auto &it : m_Components)
	{
		it->GetType()->Destroy(it);
	}
	m_Components.clear();

	for (auto child : m_Children)
	{
		child->Release();
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
	if (!pchild || (pchild == this))
		return;

	if (std::find(m_Children.cbegin(), m_Children.cend(), pchild) == m_Children.cend())
	{
		m_Children.push_back(pchild);

		pchild->SetOwner(this);
	}
}


void ObjectImpl::RemoveChild(Object *pchild, bool release)
{
	if (!pchild || (pchild == this))
		return;

	TObjectArray::const_iterator it = std::find(m_Children.cbegin(), m_Children.cend(), pchild);
	if (it == m_Children.cend())
	{
		(*it)->SetOwner(nullptr);
		if (release)
			(*it)->Release();

		m_Children.erase(it);
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


size_t ObjectImpl::GetNumComponents()
{
	return m_Components.size();
}


Component *ObjectImpl::GetComponent(size_t index)
{
	if (index >= m_Components.size())
		return nullptr;

	return m_Components[index];
}


Component *ObjectImpl::FindComponent(const ComponentType *pctype)
{
	Component *ret = nullptr;

	for (size_t i = 0, maxi = m_Components.size(); i < maxi; i++)
	{
		if (m_Components[i]->GetType() == pctype)
		{
			ret = m_Components[i];
			break;
		}
	}

	return ret;
}


Component *ObjectImpl::AddComponent(const ComponentType *pctype, bool init)
{
	if (!pctype)
		return nullptr;

	Component *pc = FindComponent(pctype);
	if (pc)
		return pc;

	pc = pctype->Build();
	if (!pc)
		return nullptr;

	m_Components.push_back(pc);

	if (init)
		pc->Initialize(this);

	return pc;
}


void ObjectImpl::RemoveComponent(Component *pcomportmemt)
{
	TComponentArray::iterator it = std::find(m_Components.begin(), m_Components.end(), pcomportmemt);
	if (it != m_Components.end())
	{
		(*it)->GetType()->Destroy((*it));
		m_Components.erase(it);
	}
}


void ObjectImpl::Update(float elapsed_time)
{
	if (!m_Flags.IsSet(OBJFLAG(UPDATE)))
		return;

	for (const auto &it : m_Components)
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

	for (const auto it : m_Components)
	{
		if (it->Prerender(this, rendflags))
			ret = true;
	}

	return ret;
}


bool ObjectImpl::Render(props::TFlags64 rendflags)
{
	if (!Prerender(rendflags))
		return false;

	for (const auto &it : m_Components)
	{
		it->Render(this, rendflags);
	}

	for (auto child : m_Children)
	{
		child->Render(rendflags);
	}

	Postrender(rendflags);

	return true;
}


void ObjectImpl::Postrender(props::TFlags64 rendflags)
{
}


bool ObjectImpl::Load(genio::IInputStream *is)
{
	if (!is)
		return false;

	genio::FOURCHARCODE b = is->NextBlockId();
	if (b == 'OBJ0')
	{
		if (is->BeginBlock(b))
		{
			is->Read(&m_GUID, sizeof(GUID));

			uint16_t len;
			is->ReadUINT16(len);
			m_Name.resize(len);
			is->ReadString((TCHAR *)(m_Name.c_str()));

			is->Read(&m_Flags, sizeof(props::TFlags64));

			size_t propssz, propsbr;
			is->ReadUINT64(propssz);
			if (propssz)
			{
				BYTE *propsbuf = (BYTE *)_alloca(propssz);
				is->Read(propsbuf, propssz);
				m_Props->Deserialize(propsbuf, propssz, &propsbr);
			}

			size_t ct;

			// read components
			is->ReadUINT64(ct);
			while (ct--)
			{
				GUID g;
				is->Read(&g, sizeof(GUID));
				AddComponent(m_pSys->GetFactory()->FindComponentType(g));
			}

			// read children
			is->ReadUINT64(ct);
			while (ct--)
			{
				Object *obj = m_pSys->GetFactory()->Build();
				if (obj)
				{
					obj->Load(is);
					AddChild(obj);
				}
			}

			is->EndBlock();
		}

		PostLoad();
	}

	return true;
}


bool ObjectImpl::Save(genio::IOutputStream *os, props::TFlags64 saveflags)
{
	if (!os)
		return false;

	if (os->BeginBlock('OBJ0'))
	{
		os->Write(&m_GUID, sizeof(GUID));

		uint16_t len = (uint16_t)m_Name.length();
		os->WriteUINT16(len);
		os->WriteString((TCHAR *)(m_Name.c_str()));

		os->Write(&m_Flags, sizeof(props::TFlags64));

		size_t propssz = 0;
		if (m_Props->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_TERSE, nullptr, 0, &propssz) && propssz)
		{
			BYTE *propsbuf = (BYTE *)_alloca(propssz);
			m_Props->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_TERSE, propsbuf, propssz);
			os->Write(propsbuf, propssz);
		}

		os->WriteUINT64(GetNumComponents());
		for (auto comp : m_Components)
		{
			GUID g = comp->GetType()->GetGUID();
			os->Write(&g, sizeof(GUID));
		}

		os->WriteUINT64(GetNumChildren());
		for (auto child : m_Children)
		{
			child->Save(os, saveflags);
		}

		os->EndBlock();
	}

	return true;
}


void ObjectImpl::PostLoad()
{

}


void ObjectImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	for (const auto &it : m_Components)
	{
		it->PropertyChanged(pprop);
	}
}
