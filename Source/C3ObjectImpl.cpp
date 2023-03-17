// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


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
	if (powner == m_Owner)
		return;

	if (m_Owner)
		m_Owner->RemoveChild(this);

	m_Owner = powner;

	if (m_Owner)
		m_Owner->AddChild(this);
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
	if (it != m_Children.cend())
	{
		Object *pco = *it;
		m_Children.erase(it);

		pco->SetOwner(nullptr);
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


bool ObjectImpl::Prerender(Object::RenderFlags flags)
{
#if 0
/*
	if (flags.IsSet(RF_FORCE) || (flags.IsSet(RF_EDITORDRAW) && m_Flags.IsSet(OF_DRAWINEDITOR)))
		return true;

	if (!m_Flags.IsSet(OF_DRAW))
		return false;

	if (flags.IsSet(RF_LIGHT) && !m_Flags.IsSet(OF_LIGHT))
		return false;
*/
	// TODO: port visibility culling

	for (const auto it : m_Components)
		if (it->Prerender(this, flags))
			return true;

	return false;
#else
	return true;
#endif
}


bool ObjectImpl::Render(Object::RenderFlags flags)
{
	if (!Prerender(flags))
		return false;

	for (const auto &it : m_Components)
	{
		it->Render(flags);
	}

	for (auto child : m_Children)
	{
		child->Render(flags);
	}

	Postrender(flags);

	return true;
}


void ObjectImpl::Postrender(Object::RenderFlags flags)
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


bool ObjectImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, float *pDistance, Object **ppHitObj, size_t child_depth) const
{
	if (!pRayPos || !pRayDir)
		return false;

	float tmpdist;
	if (!pDistance)
	{
		pDistance = &tmpdist;
		tmpdist = FLT_MAX;
	}

	bool ret = false;

	glm::fmat4x4 t, tn;

	Positionable *ppos = (Positionable *)((ObjectImpl *)this)->FindComponent(Positionable::Type());
	OmniLight *plight = (OmniLight *)((ObjectImpl *)this)->FindComponent(OmniLight::Type());

	if (ppos)
	{
		ppos->GetTransformMatrix(&t);
		ppos->GetTransformMatrixNormal(&tn);

		if (plight)
		{
			glm::fvec3 invscl = 1.0f / *(ppos->GetScl());
			t = t * glm::scale(glm::identity<glm::fmat4x4>(), invscl);
		}
	}
	else
	{
		t = glm::identity<glm::fmat4x4>();
		tn = glm::identity<glm::fmat4x4>();
	}

	glm::fmat4x4 invt = glm::inverse(t);
	glm::fmat4x4 invtn = glm::inverse(tn);

	glm::vec3 raypos = invt * glm::vec4(pRayPos->x, pRayPos->y, pRayPos->z, 1);
	glm::vec3 raydir = glm::normalize(invtn * glm::vec4(pRayDir->x, pRayDir->y, pRayDir->z, 0));

	float dist = FLT_MAX;
	for (auto comp : m_Components)
	{
		if (comp->Intersect(&raypos, &raydir, &dist))
		{
			if (pDistance)
			{
				if (dist < *pDistance)
				{
					*pDistance = dist;
					if (ppHitObj)
						*ppHitObj = (Object *)this;
				}
			}
			else if (ppHitObj)
			{
				*ppHitObj = (Object *)this;
			}

			ret = true;
		}
	}

	if (child_depth > 0)
	{
		Object *hitobj = nullptr;
		for (auto child : m_Children)
		{
			ret |= child->Intersect(&raypos, &raydir, pDistance, &hitobj, child_depth - 1);
		}

		if (ppHitObj)
			*ppHitObj = hitobj;
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
