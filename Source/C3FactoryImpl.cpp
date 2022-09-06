// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


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

		// add all components first, before intializing them...
		for (size_t i = 0, maxi = pproto->GetNumComponents(); i < maxi; i++)
		{
			o->AddComponent(pproto->GetComponent(i), false);
		}

		// now initialize so they can discover each other
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
				uint32_t namelen;
				is->ReadUINT32(namelen);
				tstring name;
				if (namelen)
				{
					name.resize(namelen, _T('#'));
					is->Read(name.data(), sizeof(TCHAR) * namelen);
				}
				p->SetName(name.c_str());

				uint32_t grouplen;
				is->ReadUINT32(grouplen);
				tstring group;
				if (grouplen)
				{
					group.resize(grouplen, _T('#'));
					is->Read(group.data(), sizeof(TCHAR) * grouplen);
				}
				p->SetGroup(group.c_str());

				uint64_t flags;
				is->ReadUINT64(flags);
				p->Flags().SetAll(flags);

				uint32_t numcomps;
				is->ReadUINT32(numcomps);
				for (uint32_t ci = 0; ci < numcomps; ci++)
				{
					GUID g;
					is->Read(&g, sizeof(GUID));

					const ComponentType *pct = FindComponentType(g);
					if (pct)
						p->AddComponent(pct);
				}

				// properties
				size_t bsz = 0;
				is->ReadUINT64(bsz);
				if (bsz)
				{
					uint8_t *buf = (uint8_t *)malloc(bsz);
					size_t bc;
					p->GetProperties()->Deserialize((uint8_t *)buf, bsz, &bc);
					free(buf);
				}
			}

			is->EndBlock();
		}
	}

	return true;
}


bool FactoryImpl::LoadPrototypes(const tinyxml2::XMLNode *proot)
{
	if (!proot)
		return false;

	const tinyxml2::XMLElement *pel = proot->FirstChildElement("prototype");
	while (pel)
	{
		GUID guid;
		const tinyxml2::XMLAttribute *paguid = pel->FindAttribute("guid");
		if (paguid)
		{
			size_t len = strlen(paguid->Value());
			sscanf_s(paguid->Value(), "{%8X-%4hX-%4hX-%2hhX%2hhX-%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX}", &guid.Data1, &guid.Data2, &guid.Data3,
					 &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3], &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);
		}

		Prototype *pproto = new PrototypeImpl(m_pSys, guid);

		m_Prototypes.push_back(pproto);

		const tinyxml2::XMLAttribute *paname = pel->FindAttribute("name");
		if (paname)
		{
			TCHAR *n;
			CONVERT_MBCS2TCS(paname->Value(), n);
			pproto->SetName(n);
		}

		const tinyxml2::XMLAttribute *pagroup = pel->FindAttribute("group");
		if (pagroup)
		{
			TCHAR *g;
			CONVERT_MBCS2TCS(pagroup->Value(), g);
			pproto->SetGroup(g);
		}

		const tinyxml2::XMLAttribute *paflags = pel->FindAttribute("flags");
		if (paflags)
		{
			pproto->Flags().SetAll(paflags->Int64Value());
		}

		const tinyxml2::XMLElement *pcomps = pel->FirstChildElement("components");
		if (pcomps)
		{
			const tinyxml2::XMLElement *pcomp = pcomps->FirstChildElement("component");
			while (pcomp)
			{
				const tinyxml2::XMLAttribute *paguid = pcomp->FindAttribute("guid");
				if (paguid)
				{
					size_t len = strlen(paguid->Value());
					sscanf_s(paguid->Value(), "{%8X-%4hX-%4hX-%2hhX%2hhX-%2hhX%2hhX%2hhX%2hhX%2hhX%2hhX}", &guid.Data1, &guid.Data2, &guid.Data3,
							 &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3], &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);

					const ComponentType *pct = FindComponentType(guid);
					if (pct)
						pproto->AddComponent(pct);

					pcomp = pcomp->NextSiblingElement("component");
				}
			}

			const tinyxml2::XMLElement *proproot = pel->FirstChildElement("powerprops:property_set");

			tinyxml2::XMLPrinter printer;
			proproot->Accept(&printer);
			std::string ps = printer.CStr();
			tstring tps;

			if (!ps.empty())
			{
#ifdef _UNICODE
				std::wstring_convert<deletable_facet<std::codecvt<wchar_t, char, std::mbstate_t>>> conv;
				tps = conv.from_bytes(ps);
#else
				tps = ps;
#endif
			}

			pproto->GetProperties()->DeserializeFromXMLString(tps);
		}

		pel = pel->NextSiblingElement("prototype");
	}

	return true;
}


bool FactoryImpl::SavePrototypes(genio::IOutputStream *os, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc)
{
	if (!os || !os->CanAccess())
		return false;

	for (const auto &it : m_Prototypes)
	{
		os->BeginBlock('PROT');
		if (!pshfunc || pshfunc(it))
		{
			GUID g = it->GetGUID();
			os->Write(&g, sizeof(GUID));

			const TCHAR *name = it->GetName();
			uint32_t namelen = (uint32_t)_tcslen(name);
			os->WriteUINT32(namelen);
			if (namelen)
				os->WriteString(name);

			const TCHAR *group = it->GetGroup();
			uint32_t grouplen = (uint32_t)_tcslen(group);
			os->WriteUINT32(grouplen);
			if (grouplen)
				os->WriteString(group);

			os->WriteUINT64(it->Flags());

			uint32_t numcomps = (uint32_t)it->GetNumComponents();
			os->WriteUINT32(numcomps);

			for (size_t ci = 0; ci < numcomps; ci++)
			{
				const c3::ComponentType *pc = it->GetComponent(ci);
				GUID g = pc->GetGUID();
				os->Write(&g, sizeof(GUID));
			}

			size_t psz = 0;
			it->GetProperties()->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_VERBOSE, nullptr, 0, &psz);
			os->WriteUINT64(psz);
			if (psz)
			{
				void *buf = malloc(psz);
				it->GetProperties()->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_VERBOSE, (uint8_t *)buf, psz);
				os->Write(buf, psz);
				free(buf);
			}
		}
		os->EndBlock();
	}

	return true;
}


bool FactoryImpl::SavePrototypes(tinyxml2::XMLNode *proot, PROTO_SAVE_HUERISTIC_FUNCTION pshfunc)
{
	if (!proot)
		return false;

	tstring ps;

	for (const auto &it : m_Prototypes)
	{
		if (!pshfunc || pshfunc(it))
		{
			tinyxml2::XMLElement *pprotonode = ((tinyxml2::XMLElement *)proot)->InsertNewChildElement("prototype");

			{
				GUID g = it->GetGUID();
				char gs[128];
				_snprintf_s(gs, 128, 128, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", g.Data1, g.Data2, g.Data3,
							g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
				pprotonode->SetAttribute("guid", gs);
			}

			{
				char *n;
				CONVERT_TCS2MBCS(it->GetName(), n);
				pprotonode->SetAttribute("name", n);
			}

			{
				char *g;
				CONVERT_TCS2MBCS(it->GetGroup(), g);
				pprotonode->SetAttribute("group", g);
			}

			{
				pprotonode->SetAttribute("flags", it->Flags());
			}

			tinyxml2::XMLElement *pcomps = pprotonode->InsertNewChildElement("components");
			for (size_t ci = 0; ci < it->GetNumComponents(); ci++)
			{
				const c3::ComponentType *pc = it->GetComponent(ci);

				tinyxml2::XMLElement *pcomp = pcomps->InsertNewChildElement("component");

				GUID g = pc->GetGUID();
				char gs[128];
				_snprintf_s(gs, 128, 128, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", g.Data1, g.Data2, g.Data3,
							 g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
				pcomp->SetAttribute("guid", gs);

				char *ns;
				CONVERT_TCS2MBCS(pc->GetName(), ns);
				pcomp->SetAttribute("name", ns);
			}

			it->GetProperties()->SerializeToXMLString(props::IProperty::SERIALIZE_MODE::SM_BIN_VERBOSE, ps);
			if (!ps.empty())
			{
				tinyxml2::XMLDocument propdoc;
				size_t pssz = (ps.size() * 2) * sizeof(char);
				char *mbcs = (char *)malloc(pssz);
				size_t retval = 0;
				wcstombs_s(&retval, mbcs, pssz, ps.c_str(), pssz);
				propdoc.Parse(mbcs);
				tinyxml2::XMLNode *pnn = propdoc.FirstChildElement()->DeepClone(pprotonode->GetDocument());
				pprotonode->InsertEndChild(pnn);
				free(mbcs);
			}
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
