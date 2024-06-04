// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


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
	m_Flags.SetAll(OF_UPDATE | OF_DRAW);
	m_pParent = nullptr;
}


ObjectImpl::~ObjectImpl()
{
	m_Props->SetChangeListener(nullptr);

	for (const auto &it : m_Components)
	{
		it->GetType()->Destroy(it);
	}
	m_Components.clear();

	for (auto child : m_Children)
	{
		child->Release();
	}
	if (m_pParent)
		m_pParent->RemoveChild(this);

	if (m_Props)
	{
		m_Props->Release();
		m_Props = nullptr;
	}
}


System *ObjectImpl::GetSystem() const
{
	return m_pSys;
}


void ObjectImpl::Release()
{
	delete this;
}


const TCHAR *ObjectImpl::GetName() const
{
	return m_Name.c_str();
}


void ObjectImpl::SetName(const TCHAR *name)
{
	m_Name = name ? name : _T("");
}


GUID ObjectImpl::GetGuid() const
{
	return m_GUID;
}


Object *ObjectImpl::GetParent() const
{
	return m_pParent;
}


void ObjectImpl::SetParent(Object *pparent)
{
	if (pparent == m_pParent)
		return;

	if (m_pParent)
		m_pParent->RemoveChild(this);

	m_pParent = pparent;

	if (m_pParent)
		m_pParent->AddChild(this);
}


size_t ObjectImpl::GetNumChildren() const
{
	return m_Children.size();
}


Object *ObjectImpl::GetChild(size_t index) const
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

		pchild->SetParent(this);
	}
}


void ObjectImpl::RemoveChild(Object *pchild, bool release)
{
	if (!pchild || (pchild == this))
		return;

	TObjectArray::const_iterator it = std::find(m_Children.cbegin(), m_Children.cend(), pchild);
	if (it != m_Children.cend())
	{
		Object *pco = *it;
		m_Children.erase(it);

		pco->SetParent(nullptr);
		if (release)
			pco->Release();
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


size_t ObjectImpl::GetNumComponents() const
{
	return m_Components.size();
}


Component *ObjectImpl::GetComponent(size_t index) const
{
	if (index >= m_Components.size())
		return nullptr;

	return m_Components[index];
}


Component *ObjectImpl::FindComponent(const ComponentType *pctype) const
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

	if (pctype->GetFlags().IsSet(CF_PUSHFRONT))
		m_Components.push_front(pc);
	else
		m_Components.push_back(pc);

	if (init)
		pc->Initialize(this);

	return pc;
}


void ObjectImpl::RemoveComponent(Component *pcomponent)
{
	TComponentArray::iterator it = std::find(m_Components.begin(), m_Components.end(), pcomponent);
	if (it != m_Components.end())
	{
		(*it)->GetType()->Destroy((*it));
		m_Components.erase(it);
	}
}


bool ObjectImpl::HasComponent(const ComponentType *pcomptype) const
{
	for (auto it : m_Components)
	{
		if (it->GetType() == pcomptype)
			return true;
	}

	return false;
}


void ObjectImpl::Update(float elapsed_time)
{
	if (!m_Flags.IsSet(OF_UPDATE))
		return;

	for (const auto &it : m_Components)
	{
		it->Update(elapsed_time);
	}

	for (auto child : m_Children)
	{
		child->Update(elapsed_time);
	}

}


bool ObjectImpl::Render(Object::RenderFlags flags, int draworder, const glm::fmat4x4 *pmat)
{
	// if no transform was provided, build it... this is important for things like selections in the editor
	glm::fmat4x4 imat = {};
	if (!pmat)
	{
		imat = glm::identity<glm::fmat4x4>();
		Object *ppar = m_pParent;
		while (ppar)
		{
			Positionable *ppos = (Positionable *)ppar->FindComponent(Positionable::Type());
			if (ppos)
				imat = *ppos->GetTransformMatrix() * imat;
			ppar = ppar->GetParent();
		}
		pmat = &imat;
	}

	if (m_Flags.IsSet(OF_DRAW)
		|| (flags.IsSet(RF_EDITORDRAW) && m_Flags.IsSet(OF_DRAWINEDITOR))
		|| (flags.IsSet(RF_SHADOW) && m_Flags.IsSet(OF_CASTSHADOW))
		|| (flags.IsSet(RF_LIGHT) && m_Flags.IsSet(OF_LIGHT)))
	{
		Positionable *ppos = (Positionable *)FindComponent(Positionable::Type());
		glm::fmat4x4 mat = ppos ? (*pmat * *ppos->GetTransformMatrix()) : *pmat;

		for (const auto &it : m_Components)
		{
			if (it->Prerender(flags, draworder))
				it->Render(flags, &mat);
		}

		for (auto child : m_Children)
		{
			child->Render(flags, draworder, &mat);
		}

		return true;
	}

	return false;

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
			if (len)
			{
				m_Name.resize(len, _T('#'));
				is->Read(m_Name.data(), sizeof(TCHAR) * len);
			}

			is->Read(&m_Flags, sizeof(props::TFlags64));

			size_t ct;

			// read components
			is->ReadUINT64(ct);
			while (ct--)
			{
				GUID g;
				is->Read(&g, sizeof(GUID));
				AddComponent(m_pSys->GetFactory()->FindComponentType(g));
			}

			size_t propssz, propsbr;
			is->ReadUINT64(propssz);
			if (propssz)
			{
				BYTE *propsbuf = (BYTE *)_alloca(propssz);
				is->Read(propsbuf, propssz);
				m_Props->Deserialize(propsbuf, propssz, &propsbr);
			}

			// read children
			is->ReadUINT64(ct);
			for (size_t i = 0; i < ct; i++)
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


bool ObjectImpl::Save(genio::IOutputStream *os, props::TFlags64 saveflags) const
{
	if (!os)
		return false;

	if (os->BeginBlock('OBJ0'))
	{
		os->Write(&m_GUID, sizeof(GUID));

		uint16_t len = (uint16_t)m_Name.length();
		os->WriteUINT16(len);
		if (len)
			os->Write(m_Name.c_str(), sizeof(TCHAR) * len);

		os->Write(&m_Flags, sizeof(props::TFlags64));

		os->WriteUINT64(GetNumComponents());
		for (auto comp : m_Components)
		{
			GUID g = comp->GetType()->GetGUID();
			os->Write(&g, sizeof(GUID));
		}

		size_t propssz = 0;
		m_Props->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_VERBOSE, nullptr, 0, &propssz);
		os->WriteUINT64(propssz);
		if (propssz)
		{
			BYTE *propsbuf = (BYTE *)_alloca(propssz);
			m_Props->Serialize(props::IProperty::SERIALIZE_MODE::SM_BIN_VERBOSE, propsbuf, propssz);
			os->Write(propsbuf, propssz);
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
	for (size_t i = 0, maxi = m_Props->GetPropertyCount(); i < maxi; i++)
	{
		for (auto comp : m_Components)
		{
			comp->PropertyChanged(m_Props->GetProperty(i));
		}
	}

	Update();
}


bool ObjectImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, Object **ppHitObj, props::TFlags64 flagmask, size_t child_depth) const
{
	if (!pRayPos || !pRayDir)
		return false;

	// If no transform was provided, build it from the parent hierarchy
	glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
	{
		const Object *ppar = this;
		while (ppar)
		{
			Positionable *ppos = dynamic_cast<Positionable *>(ppar->FindComponent(Positionable::Type()));
			if (ppos)
				imat = *ppos->GetTransformMatrix() * imat;
			ppar = ppar->GetParent();
		}
		pmat = &imat;
	}

	// Check if the object passes the flag mask
	if (!m_Flags.AnySet(flagmask))
		return false;

	// Initialize distance to the maximum possible value
	float tmpdist = FLT_MAX;
	if (!pDistance)
	{
		pDistance = &tmpdist;
	}

	bool ret = false;
	float dist = *pDistance;

	// Get the object's positionable transform
	Positionable *ppos = (Positionable *)FindComponent(Positionable::Type());
	glm::fmat4x4 mat = ppos ? (*pmat * *(ppos->GetTransformMatrix())) : *pmat;

	// Check intersection with components
	for (auto comp : m_Components)
	{
		float compDist = dist;
		if (comp->Intersect(pRayPos, pRayDir, &mat, &compDist))
		{
			if (compDist < *pDistance)
			{
				*pDistance = compDist;
				if (ppHitObj)
					*ppHitObj = const_cast<Object *>(static_cast<const Object *>(this));
				ret = true;
			}
		}
	}

	// Check intersection with children
	if (child_depth > 0)
	{
		for (auto child : m_Children)
		{
			if (child->Intersect(pRayPos, pRayDir, &mat, pDistance, ppHitObj, flagmask, child_depth - 1))
			{
				ret = true;
			}
		}
	}

	return ret;
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
