// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>
#include <C3PrototypeImpl.h>
#include <C3ObjectImpl.h>

// required for GUID creation
#include <ObjBase.h>



using namespace c3;


FactoryImpl::TComponentTypeArray FactoryImpl::s_ComponentTypes;
FactoryImpl::TFlowNodeTypeArray FactoryImpl::s_FlowNodeTypes;


FactoryImpl::FactoryImpl(SystemImpl *psys)
{
	m_pSys = psys;
}


FactoryImpl::~FactoryImpl()
{
	for (TPrototypeArray::iterator it = m_Prototypes.begin(), last_it = m_Prototypes.end(); it != last_it; it++)
	{
		delete (PrototypeImpl *)(*it);
	}
}


Object *FactoryImpl::Build(Prototype *pproto, GUID *override_guid)
{
	GUID guid;
	if (!override_guid)
		CoCreateGuid(&guid);

	Object *o = new ObjectImpl(m_pSys, override_guid ? *override_guid : guid);

	if (pproto)
	{
		o->SetName(pproto->GetName());
		o->Flags().SetAll(pproto->Flags());

		for (size_t i = 0, maxi = pproto->GetNumComponents(); i < maxi; i++)
		{
			o->AddComponent(pproto->GetComponent(i), false);
		}

		for (size_t i = 0, maxi = o->GetNumComponents(); i < maxi; i++)
		{
			Component *pc = o->GetComponent(i);
			if (!pc)
				continue;

			pc->Initialize(o);
		}

		o->GetProperties()->AppendPropertySet(pproto->GetProperties());
	}

	o->PostLoad();

	return o;
}


Object *FactoryImpl::Build(Object *pobject, GUID *override_guid)
{
	GUID guid;
	if (!override_guid)
		CoCreateGuid(&guid);

	Object *o = new ObjectImpl(m_pSys, override_guid ? *override_guid : guid);

	if (pobject)
	{
		o->SetName(pobject->GetName());
		o->Flags().SetAll(pobject->Flags());
		o->GetProperties()->AppendPropertySet(pobject->GetProperties());

		for (size_t i = 0, maxi = pobject->GetNumComponents(); i < maxi; i++)
		{
			Component *ppc = pobject->GetComponent(i);
			if (!ppc)
				continue;

			Component *pc = o->AddComponent(ppc->GetType(), false);
		}

		for (size_t i = 0, maxi = pobject->GetNumComponents(); i < maxi; i++)
		{
			Component *pc = o->GetComponent(i);
			if (!pc)
				continue;

			pc->Initialize(o);
		}
	}

	o->PostLoad();

	return o;
}


Prototype *FactoryImpl::CreatePrototype(Prototype *pproto)
{
	GUID guid;
	CoCreateGuid(&guid);
	Prototype *p = new PrototypeImpl(m_pSys, guid);

	m_Prototypes.push_back(p);

	if (pproto)
	{
		p->SetName(pproto->GetName());
		p->Flags().SetAll(pproto->Flags());
		p->GetProperties()->AppendPropertySet(pproto->GetProperties());

		for (size_t i = 0, maxi = pproto->GetNumComponents(); i < maxi; i++)
		{
			p->AddComponent(pproto->GetComponent(i));
		}
	}

	return p;
}


Prototype *FactoryImpl::MakePrototype(Object *pobject)
{
	GUID guid;
	CoCreateGuid(&guid);
	Prototype *p = new PrototypeImpl(m_pSys, guid);

	m_Prototypes.push_back(p);

	if (pobject)
	{
		p->SetName(pobject->GetName());
		p->Flags().SetAll(pobject->Flags());
		p->GetProperties()->AppendPropertySet(pobject->GetProperties());

		for (size_t i = 0, maxi = pobject->GetNumComponents(); i < maxi; i++)
		{
			Component *ppc = pobject->GetComponent(i);
			if (!ppc)
				continue;

			p->AddComponent(ppc->GetType());
		}
	}

	return p;
}


void FactoryImpl::RemovePrototype(Prototype *pproto)
{
	TPrototypeArray::iterator it = std::find(m_Prototypes.begin(), m_Prototypes.end(), pproto);
	if (it == m_Prototypes.end())
		return;

	delete (PrototypeImpl *)(*it);
}


size_t FactoryImpl::GetNumPrototypes()
{
	return m_Prototypes.size();
}


Prototype *FactoryImpl::GetPrototype(size_t index)
{
	if (index >= m_Prototypes.size())
		return nullptr;

	return m_Prototypes[index];
}


Prototype *FactoryImpl::FindPrototype(const TCHAR *name, bool case_sensitive)
{
	if (case_sensitive)
	{
		for (auto &it : m_Prototypes)
		{
			if (!_tcscmp(it->GetName(), name))
				return it;
		}
	}
	else
	{
		for (auto &it : m_Prototypes)
		{
			if (!_tcsicmp(it->GetName(), name))
				return it;
		}
	}

	return nullptr;
}


Prototype *FactoryImpl::FindPrototype(GUID guid)
{
	for (auto &it : m_Prototypes)
	{
		if (it->GetGUID() == guid)
			return it;
	}

	return nullptr;
}


bool FactoryImpl::LoadPrototypes(genio::IInputStream *is)
{
	if (!is || !is->CanAccess())
		return false;

	// keep loading protoype blocks until we run out
	while (is->NextBlockId() == 'PROT')
	{
		if (is->BeginBlock('PROT'))
		{
			GUID guid;
			is->Read(&guid, sizeof(GUID));

			Prototype *p = new PrototypeImpl(m_pSys, guid);
			if (p)
			{
				tstring name;
				uint32_t namelen;
				is->ReadUINT32(namelen);
				name.resize(namelen, _T('#'));
				is->ReadString((TCHAR *)name.c_str());
				p->SetName(name.c_str());

				uint64_t flags;
				is->ReadUINT64(flags);
				p->Flags().SetAll(flags);

				// properties
				if (is->BeginBlock('PROP'))
				{
					//p->GetProp
					//p->GetProperties()->Deserialize()
				}
			}

			is->EndBlock();
		}
	}

	return true;
}


bool FactoryImpl::LoadPrototypes(tinyxml2::XMLNode *proot)
{
	return false;
}


bool FactoryImpl::SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc)
{
	if (!os || !os->CanAccess())
		return false;

	for (const auto &it : m_Prototypes)
	{
		if (!pshfunc || pshfunc(it))
		{
			// TODO: implement prototype saving
		}
	}

	return true;
}


bool FactoryImpl::SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc)
{
	if (!proot)
		return false;

	for (const auto &it : m_Prototypes)
	{
		if (!pshfunc || pshfunc(it))
		{
			// TODO: implement prototype saving
		}
	}

	return true;
}


bool FactoryImpl::RegisterComponentType(ComponentType *pctype)
{
	if (!pctype)
		return false;

	m_pSys->GetLog()->Print(_T("Registering ComponentType \"%s\" ... "), pctype->GetName());
	for (const auto &it : s_ComponentTypes)
	{
		if (!_tcscmp(it->GetName(), pctype->GetName()) || (it->GetGUID() == pctype->GetGUID()))
		{
			m_pSys->GetLog()->Print(_T("FAILED (duplicate)\n"));
			return false;
		}
	}

	m_pSys->GetLog()->Print(_T("OK\n"));
	s_ComponentTypes.push_back(pctype);

	return true;
}


bool FactoryImpl::UnregisterComponentType(ComponentType *pctype)
{
	if (!pctype)
		return false;

	TComponentTypeArray::iterator it = std::find(s_ComponentTypes.begin(), s_ComponentTypes.end(), pctype);
	if (it != s_ComponentTypes.end())
	{
		s_ComponentTypes.erase(it);
		return true;
	}

	return false;
}


size_t FactoryImpl::GetNumComponentTypes()
{
	return s_ComponentTypes.size();
}


const ComponentType *FactoryImpl::GetComponentType(size_t index)
{
	if (index >= s_ComponentTypes.size())
		return nullptr;

	return s_ComponentTypes[index];
}


const ComponentType *FactoryImpl::FindComponentType(const TCHAR *name, bool case_sensitive)
{
	if (!name)
		return nullptr;

	if (case_sensitive)
	{
		for (const auto &it : s_ComponentTypes)
		{
			if (!_tcscmp(it->GetName(), name))
			{
				return it;
			}
		}
	}
	else
	{
		for (const auto &it : s_ComponentTypes)
		{
			if (!_tcsicmp(it->GetName(), name))
			{
				return it;
			}
		}
	}

	return nullptr;
}


const ComponentType *FactoryImpl::FindComponentType(GUID guid)
{
	for (const auto &it : s_ComponentTypes)
	{
		if (it->GetGUID() == guid)
		{
			return it;
		}
	}

	return nullptr;
}


bool FactoryImpl::RegisterFlowNodeType(FlowNodeType *pfntype)
{
	if (!pfntype)
		return false;

	m_pSys->GetLog()->Print(_T("Registering FlowNodeType \"%s\" ... "), pfntype->GetName());
	for (const auto &it : s_FlowNodeTypes)
	{
		if (!_tcscmp(it->GetName(), pfntype->GetName()) || (it->GetGUID() == pfntype->GetGUID()))
		{
			m_pSys->GetLog()->Print(_T("FAILED (duplicate)\n"));
			return false;
		}
	}

	m_pSys->GetLog()->Print(_T("OK\n"));
	s_FlowNodeTypes.push_back(pfntype);

	return true;
}


bool FactoryImpl::UnregisterFlowNodeType(FlowNodeType *pfntype)
{
	if (!pfntype)
		return false;

	TFlowNodeTypeArray::iterator it = std::find(s_FlowNodeTypes.begin(), s_FlowNodeTypes.end(), pfntype);
	if (it != s_FlowNodeTypes.end())
	{
		s_FlowNodeTypes.erase(it);
		return true;
	}

	return false;
}


size_t FactoryImpl::GetNumFlowNodeTypes()
{
	return s_FlowNodeTypes.size();
}


const FlowNodeType *FactoryImpl::GetFlowNodeType(size_t index)
{
	if (index >= s_FlowNodeTypes.size())
		return nullptr;

	return s_FlowNodeTypes[index];
}


const FlowNodeType *FactoryImpl::FindFlowNodeType(const TCHAR *name, bool case_sensitive)
{
	if (!name)
		return nullptr;

	if (case_sensitive)
	{
		for (const auto &it : s_FlowNodeTypes)
		{
			if (!_tcscmp(it->GetName(), name))
			{
				return it;
			}
		}
	}
	else
	{
		for (const auto &it : s_FlowNodeTypes)
		{
			if (!_tcsicmp(it->GetName(), name))
			{
				return it;
			}
		}
	}

	return nullptr;
}


const FlowNodeType *FactoryImpl::FindFlowNodeType(GUID guid)
{
	for (const auto &it : s_FlowNodeTypes)
	{
		if (it->GetGUID() == guid)
		{
			return it;
		}
	}

	return nullptr;
}
