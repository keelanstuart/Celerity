// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3FactoryImpl.h>
#include <C3SystemImpl.h>
#include <C3PrototypeImpl.h>
#include <C3ObjectImpl.h>
#include <C3Utility.h>

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

	size_t i = m_Children.size();
	while (i)
	{
		Object *pchild = m_Children[--i];
		pchild->Release();
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

	size_t i = m_Children.size();
	while (i)
	{
		Object *pc = m_Children[--i];
		pc->Update(elapsed_time);

		if (pc->Flags().IsSet(OF_KILL))
			RemoveChild(pc, true);
	}

	for (const auto &it : m_Components)
	{
		it->Update(elapsed_time);
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
		|| (flags.IsSet(RF_LIGHT) && m_Flags.IsSet(OF_LIGHT))
		|| flags.IsSet(RF_FORCE))
	{
		Positionable *ppos = (Positionable *)FindComponent(Positionable::Type());
		glm::fmat4x4 mat = ppos ? (*pmat * *ppos->GetTransformMatrix()) : *pmat;

		if (!flags.IsSet(RF_SHADOW) || m_Flags.IsSet(OF_CASTSHADOW))
		{
			for (const auto &it : m_Components)
			{
				if (it->Prerender(flags, draworder))
					it->Render(flags, &mat);
			}
		}

		if (m_Flags.IsSet(OF_DRAW) || flags.IsSet(RF_FORCE))
		{
			for (auto child : m_Children)
			{
				child->Render(flags, draworder, &mat);
			}
		}

		return true;
	}

	return false;

}


bool ObjectImpl::Load(genio::IInputStream *is, Object *parent, MetadataLoadFunc loadmd, CameraLoadFunc loadcam, EnvironmentLoadFunc loadenv, CustomLoadFunc loadcust)
{
	if (!is)
		return false;

	genio::FOURCHARCODE b;

	size_t celdepth = 0;

	SetParent(parent);

	b = is->NextBlockId();
	if ((b != 'CEL0') && (b != 'OBJ0'))
		return false;

	do
	{
		b = is->NextBlockId();

		switch (b)
		{
			case 'CEL0':
				if (is->BeginBlock(b))
				{
					uint16_t len;

					tstring name, description, author, website, copyright;
					is->ReadUINT16(len);
					name.resize(len);
					if (len)
						is->ReadString(name.data());

					is->ReadUINT16(len);
					description.resize(len);
					if (len)
						is->ReadString(description.data());

					is->ReadUINT16(len);
					author.resize(len);
					if (len)
						is->ReadString(author.data());

					is->ReadUINT16(len);
					website.resize(len);
					if (len)
						is->ReadString(website.data());

					is->ReadUINT16(len);
					copyright.resize(len);
					if (len)
						is->ReadString(copyright.data());

					if (loadmd)
						loadmd(name, description, author, website, copyright);

					celdepth++;
				}
				break;

			case 'CAM0':
				if (is->BeginBlock(b))
				{
					if (loadcam)
					{
						Object *pcam = m_pSys->GetFactory()->Build();
						pcam->Load(is, nullptr);

						float yaw, pitch;
						is->ReadFloat(yaw);
						is->ReadFloat(pitch);

						loadcam(pcam, yaw, pitch);
					}

					is->EndBlock();
				}
				break;

			case 'ENV0':
				if (is->BeginBlock(b))
				{
					if (loadenv)
					{
						glm::fvec4 clearcolor, fogcolor, shadowcolor;
						float fogdensity;

						is->Read(&clearcolor, sizeof(glm::fvec4));
						is->Read(&shadowcolor, sizeof(glm::fvec4));
						is->Read(&fogcolor, sizeof(glm::fvec4));
						is->ReadFloat(fogdensity);

						loadenv(clearcolor, shadowcolor, fogcolor, fogdensity);
					}

					is->EndBlock();
				}
				break;

			case 'OBJ0':
				if (is->BeginBlock(b))
				{
					is->Read(&m_GUID, sizeof(GUID));

					uint16_t len;
					is->ReadUINT16(len);
					if (len)
					{
						m_Name.resize(len, _T('.'));
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
							obj->Load(is, this, nullptr, nullptr, nullptr, loadcust);
						}
					}

					is->EndBlock();
				}

				PostLoad();
				b = 0;	// exit this loop once the object is loaded
				break;

			default:
				if (is->BeginBlock(b))
				{
					if (loadcust)
						loadcust(is);

					is->EndBlock();
				}
				break;
		}
	}
	while (b);

	while (celdepth)
	{
		celdepth--;
		is->EndBlock();
	}

	return true;
}


bool ObjectImpl::Save(genio::IOutputStream *os, props::TFlags64 saveflags, MetadataSaveFunc savemd, CameraSaveFunc savecam, EnvironmentSaveFunc saveenv, CustomSaveFunc savecust) const
{
	if (!os)
		return false;

	if (m_Flags.IsSet(OF_TEMPORARY))
		return true;

	if (savemd && os->BeginBlock('CEL0'))
	{
		tstring name, description, author, website, copyright;
		savemd(name, description, author, website, copyright);

		uint16_t len;

		len = (uint16_t)name.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString(name.data());

		len = (uint16_t)description.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString(description.data());

		len = (uint16_t)author.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString(author.data());

		len = (uint16_t)website.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString(website.data());

		len = (uint16_t)copyright.length();
		os->WriteUINT16(len);
		if (len)
			os->WriteString(copyright.c_str());
	}

	if (savecam && os->BeginBlock('CAM0'))
	{
		Object *pcam;
		float yaw, pitch;
		savecam(&pcam, yaw, pitch);

		pcam->Save(os, 0);
		os->WriteFloat(yaw);
		os->WriteFloat(pitch);

		os->EndBlock();
	}

	if (saveenv && os->BeginBlock('ENV0'))
	{
		glm::fvec4 clearcolor, shadowcolor, fogcolor;
		float fogdensity;
		saveenv(clearcolor, shadowcolor, fogcolor, fogdensity);

		os->Write(&clearcolor, sizeof(glm::fvec4));
		os->Write(&shadowcolor, sizeof(glm::fvec4));
		os->Write(&fogcolor, sizeof(glm::fvec4));
		os->WriteFloat(fogdensity);

		os->EndBlock();
	}

	if (os->BeginBlock('OBJ0'))
	{
		os->Write(&m_GUID, sizeof(GUID));

		uint16_t len = (uint16_t)m_Name.length();
		os->WriteUINT16(len);
		if (len)
			os->Write(m_Name.c_str(), sizeof(TCHAR) * len);

		props::TFlags64 f = m_Flags;
		os->Write(&f, sizeof(props::TFlags64));

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

	if (savemd)
		os->EndBlock();

	return true;
}


void ObjectImpl::PostLoad()
{
	for (size_t i = 0, maxi = m_Props->GetPropertyCount(); i < maxi; i++)
		PropertyChanged(m_Props->GetProperty(i));

	Update();
}


bool ObjectImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, Object **ppHitObj, props::TFlags64 flagmask, size_t child_depth, bool force) const
{
	if (!pRayPos || !pRayDir)
		return false;

	// If no transform was provided, build it from the parent hierarchy
	glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
	{
		const Object *ppar = GetParent();
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
	if (!force && !m_Flags.AnySet(flagmask))
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
		glm::fvec3 compNorm;
		if (comp->Intersect(pRayPos, pRayDir, &mat, &compDist, &compNorm, force))
		{
			if (compDist < *pDistance)
			{
				*pDistance = compDist;

				if (ppHitObj)
					*ppHitObj = const_cast<Object *>(static_cast<const Object *>(this));

				if (pNormal)
					*pNormal = compNorm;

				ret = true;
			}
		}
	}

	// Check intersection with children
	if (child_depth > 0)
	{
		for (auto child : m_Children)
		{
			if (child->Intersect(pRayPos, pRayDir, &mat, pDistance, pNormal, ppHitObj, flagmask, child_depth - 1, force))
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

	// a composition object - loads temporary children from a file
	if ((pprop->GetID() == 'FILE') && (pprop->GetType() == props::IProperty::PT_STRING))
	{
		// delete all the existing temporary child objects... then load from the 
		util::ObjectArrayAction(m_Children, [&](Object *pobj)
		{
			if (pobj->Flags().IsSet(OF_TEMPORARY))
				pobj->Release();
		});

		genio::IInputStream *is = genio::IInputStream::Create();
		is->Assign(pprop->AsString());
		if (is->Open())
		{
			Object *pco = m_pSys->GetFactory()->Build();
			pco->Load(is, m_pParent);

			// mark all the objects we loaded as temporary... they're created on the fly
			util::RecursiveObjectAction(pco, [&](Object *pobj)
			{
				pobj->Flags().Set(OF_TEMPORARY);
			});
		}
	}
}
