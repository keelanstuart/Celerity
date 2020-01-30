// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#include "pch.h"

#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>
#include <C3PrototypeImpl.h>
#include <C3ObjectImpl.h>

// required for GUID creation
#include <ObjBase.h>



using namespace c3;


FactoryImpl::TComportmentTypeArray FactoryImpl::s_ComportmentTypes;


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
		o->GetProperties()->AppendPropertySet(pproto->GetProperties());

		for (size_t i = 0, maxi = pproto->GetNumComportments(); i < maxi; i++)
		{
			const ComportmentType *pct = pproto->GetComportment(i);
			if (!pct)
				continue;

			Comportment *pc = o->AddComportment(pct);
			if (!pc)
				continue;
		}
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

		for (size_t i = 0, maxi = pobject->GetNumComportments(); i < maxi; i++)
		{
			Comportment *ppc = pobject->GetComportment(i);
			if (!ppc)
				continue;

			Comportment *pc = o->AddComportment(ppc->GetType());
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

		for (size_t i = 0, maxi = pproto->GetNumComportments(); i < maxi; i++)
		{
			const ComportmentType *pct = pproto->GetComportment(i);
			if (!pct)
				continue;

			p->AddComportment(pct);
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

		for (size_t i = 0, maxi = pobject->GetNumComportments(); i < maxi; i++)
		{
			Comportment *ppc = pobject->GetComportment(i);
			if (!ppc)
				continue;

			ComportmentType *pct = ppc->GetType();
			if (!pct)
				continue;

			p->AddComportment(pct);
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
		for (TPrototypeArray::iterator it = m_Prototypes.begin(), last_it = m_Prototypes.end(); it != last_it; it++)
		{
			if (!_tcscmp((*it)->GetName(), name))
				return (*it);
		}
	}
	else
	{
		for (TPrototypeArray::iterator it = m_Prototypes.begin(), last_it = m_Prototypes.end(); it != last_it; it++)
		{
			if (!_tcsicmp((*it)->GetName(), name))
				return (*it);
		}
	}

	return nullptr;
}


Prototype *FactoryImpl::FindPrototype(GUID guid)
{
	for (TPrototypeArray::iterator it = m_Prototypes.begin(), last_it = m_Prototypes.end(); it != last_it; it++)
	{
		if ((*it)->GetGUID() == guid)
			return (*it);
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
				is->ReadString(name.c_str());
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

	for (TPrototypeArray::const_iterator it = m_Prototypes.cbegin(), last_it = m_Prototypes.cend(); it != last_it; it++)
	{
		if (!pshfunc || pshfunc((*it)))
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

	for (TPrototypeArray::const_iterator it = m_Prototypes.cbegin(), last_it = m_Prototypes.cend(); it != last_it; it++)
	{
		if (!pshfunc || pshfunc((*it)))
		{
			// TODO: implement prototype saving
		}
	}

	return true;
}


bool FactoryImpl::RegisterComportmentType(ComportmentType *pctype)
{
	if (!pctype)
		return false;

	m_pSys->GetLog()->Print(_T("Registering ComportmentType \"%s\" ... "), pctype->GetName());
	for (TComportmentTypeArray::const_iterator it = s_ComportmentTypes.cbegin(), last_it = s_ComportmentTypes.cend(); it != last_it; it++)
	{
		if (!_tcscmp((*it)->GetName(), pctype->GetName()) || ((*it)->GetGUID() == pctype->GetGUID()))
		{
			m_pSys->GetLog()->Print(_T("FAILED (duplicate)\n"));
			return false;
		}
	}

	m_pSys->GetLog()->Print(_T("OK\n"));
	s_ComportmentTypes.push_back(pctype);

	return true;
}


bool FactoryImpl::UnregisterComportmentType(ComportmentType *pctype)
{
	if (!pctype)
		return false;

	TComportmentTypeArray::iterator it = std::find(s_ComportmentTypes.begin(), s_ComportmentTypes.end(), pctype);
	if (it != s_ComportmentTypes.end())
	{
		s_ComportmentTypes.erase(it);
		return true;
	}

	return false;
}


size_t FactoryImpl::GetNumComportmentTypes()
{
	return s_ComportmentTypes.size();
}


const ComportmentType *FactoryImpl::GetComportmentType(size_t index)
{
	if (index >= s_ComportmentTypes.size())
		return nullptr;

	return s_ComportmentTypes[index];
}


const ComportmentType *FactoryImpl::FindComportmentType(const TCHAR *name, bool case_sensitive)
{
	if (!name)
		return nullptr;

	if (case_sensitive)
	{
		for (TComportmentTypeArray::const_iterator it = s_ComportmentTypes.cbegin(), last_it = s_ComportmentTypes.cend(); it != last_it; it++)
		{
			if (!_tcscmp((*it)->GetName(), name))
			{
				return *it;
			}
		}
	}
	else
	{
		for (TComportmentTypeArray::const_iterator it = s_ComportmentTypes.cbegin(), last_it = s_ComportmentTypes.cend(); it != last_it; it++)
		{
			if (!_tcsicmp((*it)->GetName(), name))
			{
				return *it;
			}
		}
	}

	return nullptr;
}


const ComportmentType *FactoryImpl::FindComportmentType(GUID guid)
{
	for (TComportmentTypeArray::const_iterator it = s_ComportmentTypes.cbegin(), last_it = s_ComportmentTypes.cend(); it != last_it; it++)
	{
		if ((*it)->GetGUID() == guid)
		{
			return *it;
		}
	}

	return nullptr;
}
