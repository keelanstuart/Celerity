// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3ParticleEmitterImpl.h>
#include <C3EnvironmentModifierImpl.h>
#include <C3Math.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ParticleEmitter, ParticleEmitterImpl);


ParticleEmitterImpl::ParticleEmitterImpl()
{
	m_pMethod = nullptr;
	m_pTexture = nullptr;
}


ParticleEmitterImpl::~ParticleEmitterImpl()
{
}


void ParticleEmitterImpl::Release()
{

}


props::TFlags64 ParticleEmitterImpl::Flags() const
{
	return 0;
}


bool ParticleEmitterImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	props::IPropertySet *propset = pobject->GetProperties();
	if (!propset)
		return false;

	m_Time = 0;

	m_Shape = EmitterShape::SPHERE;
	props::IProperty *pp = propset->CreateProperty(_T("Shape"), 'PEsh');
	if (pp)
	{
		pp->SetEnumProvider(this);
		pp->SetEnumVal(m_Shape);
	}

	pp = propset->CreateProperty(_T("ParticleRenderMethod"), 'PErm');
	if (pp)
	{
		pp->SetString(_T("particles.c3rm"));
	}

	pp = propset->CreateProperty(_T("MaxParticles"), 'PEmp');
	if (pp)
	{
		pp->SetInt(100);
	}

	m_EmitRateMin = 20.0f;
	propset->CreateReferenceProperty(_T("EmitRateMin"), 'PEcn', &m_EmitRateMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitRateMax = 30.0f;
	propset->CreateReferenceProperty(_T("EmitRateMax"), 'PEcx', &m_EmitRateMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitSpeedMin = 0.5f;
	propset->CreateReferenceProperty(_T("EmitSpeedMin"), 'PEsn', &m_EmitSpeedMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EmitSpeedMax = 1.25f;
	propset->CreateReferenceProperty(_T("EmitSpeedMax"), 'PEsx', &m_EmitSpeedMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_ParticleLifeMin = 1.75f;
	pp = propset->CreateReferenceProperty(_T("ParticleLifeMin"), 'PEln', &m_ParticleLifeMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_ParticleLifeMax = 2.0f;
	pp = propset->CreateReferenceProperty(_T("ParticleLifeMax"), 'PElx', &m_ParticleLifeMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_AccelerationMin = -0.1f;
	propset->CreateReferenceProperty(_T("AccelerationMin"), 'PEan', &m_AccelerationMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_AccelerationMax = -0.01f;
	propset->CreateReferenceProperty(_T("AccelerationMax"), 'PEax', &m_AccelerationMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_Gravity = 0.0f;
	propset->CreateReferenceProperty(_T("Gravity"), 'PEgr', &m_Gravity, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_RollMin = -0.1f;
	pp = propset->CreateReferenceProperty(_T("RollMin"), 'PErn', &m_RollMin, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_RollMax = 0.1f;
	pp = propset->CreateReferenceProperty(_T("RollMax"), 'PErx', &m_RollMax, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_RAD);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_StartScale = 1.0f;
	propset->CreateReferenceProperty(_T("StartScale"), 'PEss', &m_StartScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_PeakScale = 2.0f;
	propset->CreateReferenceProperty(_T("PeakScale"), 'PEps', &m_PeakScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_EndScale = 0.0f;
	propset->CreateReferenceProperty(_T("EndScale"), 'PEes', &m_EndScale, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_StartColor = c3::Color::iDarkBlueFT;
	pp = propset->CreateProperty(_T("StartColor"), 'PEsc');
	if (pp)
	{
		pp->SetInt(m_StartColor.i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_PeakColor = c3::Color::iCyanQT;
	pp = propset->CreateProperty(_T("PeakColor"), 'PEpc');
	if (pp)
	{
		pp->SetInt(m_PeakColor.i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_EndColor = c3::Color::iWhiteFT;
	pp = propset->CreateProperty(_T("EndColor"), 'PEec');
	if (pp)
	{
		pp->SetInt(m_EndColor.i);
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGBA);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_InnerRadius = 2.0f;
	propset->CreateReferenceProperty(_T("InnerRadius"), 'PEir', &m_InnerRadius, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_OuterRadius = 2.0f;
	propset->CreateReferenceProperty(_T("OuterRadius"), 'PEor', &m_OuterRadius, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	pp = propset->CreateProperty(_T("Texture"), 'PEtx');
	if (pp)
	{
		pp->SetString(_T("softparticle.tga"));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
	}

	m_Peak = 0.25f;
	pp = propset->CreateReferenceProperty(_T("Peak"), 'PEpp', &m_Peak, props::IProperty::PROPERTY_TYPE::PT_FLOAT);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_PERCENTAGE);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();
	m_pMaterial = pr->GetMaterialManager()->CloneMaterial(pr->GetWhiteMaterial());

	return true;
}


void ParticleEmitterImpl::Update(float elapsed_time)
{
	if (elapsed_time == 0)
		return;

	Positionable *ppos = (Positionable *)m_pOwner->FindComponent(Positionable::Type());
	if (!ppos)
		return;

	glm::fvec3 epos;
	glm::fvec3 edir;
	if (ppos)
	{
		epos = *(ppos->GetPosVec());
		edir = *(ppos->GetFacingVector());
	}
	else
	{
		epos = glm::fvec3(0, 0, 0);
		edir = glm::fvec3(0, 0, 1);
	}

	size_t pc = 0;

	glm::fvec3 acc;
	glm::fvec3 grav;
	m_pOwner->GetSystem()->GetEnvironment()->GetGravity(&grav);
	grav *= m_Gravity;

	for (size_t a = 0; a < m_Active.size(); a++)
	{
		size_t i = m_Active[a];
		SParticle &p = m_Particles[i];

		p.time += elapsed_time;
		if (p.time >= p.lifetime)
		{
			m_Inactive.push_back(i);
			m_Active.erase(m_Active.begin() + a);
			a--;
			continue;
		}

		acc = p.acc + grav;
		p.pos += (p.vel * elapsed_time);
		p.vel += (acc * elapsed_time);
		p.roll += (p.rvel * elapsed_time);
		p.rvel += (p.racc * elapsed_time);
	}

	if (!m_pOwner->Flags().IsSet(OF_KILL))
	{
		m_EmitTime -= elapsed_time;

		// if we've lapsed our emission time and have inactive particles to use, then make one
		while ((m_EmitTime < 0.0f) && !m_Inactive.empty())
		{
			m_EmitTime += 1.0f / math::RandomRange(m_EmitRateMin, m_EmitRateMax);

			size_t i = m_Inactive.back();
			m_Inactive.pop_back();

			SParticle &part = m_Particles[i];

			part.time = 0;
			part.lifetime = math::RandomRange(m_ParticleLifeMin, m_ParticleLifeMax);

			float radius = math::RandomRange(m_InnerRadius, m_OuterRadius);

			glm::fvec3 facing;
			ppos->GetFacingVector(&facing);
			glm::fvec3 right;
			ppos->GetLocalRightVector(&right);
			glm::fvec3 up;
			ppos->GetLocalUpVector(&up);

			switch (m_Shape)
			{
				case EmitterShape::SPHERE:
				{
					// find a random emission vector
					part.vel = glm::normalize(glm::fvec3(math::RandomRange(-1.0f, 1.0f), math::RandomRange(-1.0f, 1.0f), math::RandomRange(-1.0f, 1.0f)));

					// set the emission origin
					part.pos = epos;
					break;
				}

				case EmitterShape::CYLINDER:
				{
					float angle = math::RandomRange(0, glm::two_pi<float>());
					part.vel = (right * cosf(angle)) + (facing * sinf(angle));

					part.pos = epos;
					part.pos += math::RandomRange(0, ppos->GetSclZ()) * up;
					part.pos += (part.vel * radius);
					break;
				}

				case EmitterShape::CONE:
				{
					float c = glm::radians(radius);
					float hangle = math::RandomRange(-c, c);
					float vangle = math::RandomRange(-c, c);
					part.vel = facing + (right * sinf(hangle)) + (up * sinf(vangle));
					part.pos = epos;
					break;
				}

				case EmitterShape::PLANE:
				{
					ppos->GetLocalUpVector(&part.vel);
					part.pos = epos + (facing * math::RandomRange(-radius, radius)) + (right * math::RandomRange(-radius, radius));
					break;
				}

				case EmitterShape::RAY:
				{
					part.vel = facing;
					part.pos = epos;
					break;
				}
			}

			// use the initial velocity as the acceleration, to be modified by the object-global 
			part.acc = part.vel * math::RandomRange(m_AccelerationMin, m_AccelerationMax);

			float spd = math::RandomRange(m_EmitSpeedMin, m_EmitSpeedMax);
			part.vel *= spd;

			part.roll = 0.0f;
			part.rvel = glm::radians(math::RandomRange(m_RollMin, m_RollMax));
			part.racc = 0.0f; // TODO

			m_Active.push_back(i);
		}
	}
}


bool ParticleEmitterImpl::Prerender(Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!m_pOwner->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void ParticleEmitterImpl::Render(Object::RenderFlags flags)
{
	if (flags.IsSet(RF_SHADOW))
		return;

	if (flags.IsSet(RF_LIGHT))
		return;

	if (flags.IsSet(RF_AUXILIARY))
		return;

	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();

	if (!m_Verts)
		m_Verts = pr->CreateVertexBuffer();

	if (m_Verts && !m_Active.empty())
	{
		constexpr VertexBuffer::ComponentDescription vd[] =
		{
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 3, c3::VertexBuffer::ComponentDescription::Usage::VU_POSITION},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 1, c3::VertexBuffer::ComponentDescription::Usage::VU_SIZE},
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_F32, 1, c3::VertexBuffer::ComponentDescription::Usage::VU_WEIGHT},	// encode roll as "weight" (TODO add semantic)
			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_U8, 4, c3::VertexBuffer::ComponentDescription::Usage::VU_COLOR0},

			{c3::VertexBuffer::ComponentDescription::ComponentType::VCT_NONE, 0, c3::VertexBuffer::ComponentDescription::Usage::VU_NONE},
		};

#pragma pack(push, 1)
		struct SParticleVert
		{
			glm::fvec3 pos;
			float scale;
			float roll;
			glm::uint8_t color[4];
		} *v;
#pragma pack(pop)

		float t2 = 1.0f - m_Peak;
		if (VertexBuffer::RETURNCODE::RET_OK == m_Verts->Lock((void **)&v, m_Particles.size(), vd, VBLOCKFLAG_WRITE | VBLOCKFLAG_DYNAMIC))
		{
			for (size_t i = 0, maxi = m_Active.size(); i < maxi; i++)
			{
				SParticle &p = m_Particles[m_Active[i]];

				v->pos = p.pos;
				v->roll = p.roll;

				// there's an idea of "peak" values... the m
				float pct = p.time / p.lifetime, t;
				if (pct < m_Peak)
				{
					t = sinf(pct / m_Peak * glm::half_pi<float>());

					v->scale = std::lerp(m_StartScale, m_PeakScale, t);
					v->color[0] = (uint8_t)(std::lerp((float)m_StartColor.r, (float)m_PeakColor.r, t));
					v->color[1] = (uint8_t)(std::lerp((float)m_StartColor.g, (float)m_PeakColor.g, t));
					v->color[2] = (uint8_t)(std::lerp((float)m_StartColor.b, (float)m_PeakColor.b, t));
					v->color[3] = (uint8_t)(std::lerp((float)m_StartColor.a, (float)m_PeakColor.a, t));
				}
				else
				{
					t = sinf(((pct - m_Peak) / t2) * glm::half_pi<float>());

					v->scale = std::lerp(m_PeakScale, m_EndScale, t);
					v->color[0] = (uint8_t)(std::lerp((float)m_PeakColor.r, (float)m_EndColor.r, t));
					v->color[1] = (uint8_t)(std::lerp((float)m_PeakColor.g, (float)m_EndColor.g, t));
					v->color[2] = (uint8_t)(std::lerp((float)m_PeakColor.b, (float)m_EndColor.b, t));
					v->color[3] = (uint8_t)(std::lerp((float)m_PeakColor.a, (float)m_EndColor.a, t));
				}

				v++;
			}

			m_Verts->Unlock();
		}
	}

	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (!m_pMethod)
		{
			props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('PErm');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("particles.c3rm"), RESF_DEMANDLOAD);
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				{
					m_pMethod = (RenderMethod *)(pres->GetData());

					m_pMethod->FindTechnique(_T("g"), m_TechIdx_G);
				}
			}
		}

		if (m_pMethod)
		{
			m_pMethod->SetActiveTechnique(m_TechIdx_G);
			m_pMethod->GetActiveTechnique()->SetMode(RenderMethod::Technique::TECHMODE_NORMAL);
			pr->UseRenderMethod(m_pMethod);
		}
	}

	if (!flags.IsSet(RF_LOCKMATERIAL))
	{
		if (!m_pTexture)
		{
			props::IProperty *ptex = m_pOwner->GetProperties()->GetPropertyById('PEtx');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(ptex ? ptex->AsString() : _T("softparticle.tga"), RESF_DEMANDLOAD);
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				{
					m_pTexture = dynamic_cast<Texture2D *>((Texture2D *)(pres->GetData()));
				}
			}
		}
		else
		{
			if (m_pMaterial)
				m_pMaterial->SetTexture(Material::TextureComponentType::TCT_DIFFUSE, m_pTexture);
		}
	}

	if (m_Active.size())
	{
		pr->UseMaterial(m_pMaterial);
		pr->DrawPrimitives(c3::Renderer::EPrimType::POINTLIST, m_Active.size());
	}
}


void ParticleEmitterImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();

	switch (fcc)
	{
		case 'PEmp':
		{
			m_MaxParticles = pprop->AsInt();
			if (m_Particles.size() < m_MaxParticles)
				m_Particles.resize(m_MaxParticles);
			m_Active.clear();
			m_Active.reserve(m_MaxParticles);
			m_Inactive.clear();
			m_Inactive.reserve(m_MaxParticles);
			for (size_t i = 0; i < m_MaxParticles; i++)
				m_Inactive.push_back(i);
			break;
		}

		case 'PErm':
		{
			// just null this - it will be re-acquired the next time we draw
			m_pMethod = nullptr;
			break;
		}

		case 'PEtx':
		{
			m_pTexture = nullptr;
			break;
		}

		case 'PEsh':
		{
			m_Shape = (EmitterShape)pprop->AsInt();
			break;
		}

		case 'PEsc':
		{
			m_StartColor.i = (uint32_t)(pprop->AsInt());
			m_StartColor.a = 255;
			break;
		}

		case 'PEpc':
		{
			m_PeakColor.i = (uint32_t)(pprop->AsInt());
			m_PeakColor.a = 255;
			break;
		}

		case 'PEec':
		{
			m_EndColor.i = (uint32_t)(pprop->AsInt());
			m_EndColor.a = 255;
			break;
		}
	}
}


std::vector<tstring> ShapeNames = { _T("Sphere"), _T("Ray"), _T("Cone"), _T("Cylinder"), _T("Plane") };

size_t ParticleEmitterImpl::GetNumValues(const props::IProperty *pprop) const
{
	assert(pprop);

	switch (pprop->GetID())
	{
		case 'PEsh':
		{
			return ShapeNames.size();
			break;
		}

		default:
			break;
	}

	return 1;
}


const TCHAR *ParticleEmitterImpl::GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const
{
	assert(pprop);

	const TCHAR *ret = nullptr;

	switch (pprop->GetID())
	{
		case 'PEsh':
			ret = ShapeNames[ordinal].c_str();
			break;

		default:
			break;
	}

	if (ret && buf && bufsize)
		_tcscpy_s(buf, bufsize, ret);

	return ret;
}


bool ParticleEmitterImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, MatrixStack *mats, float *pDistance) const
{
	bool ret = false;

	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();
	const Mesh *pm = prend->GetBoundsMesh();

	if (pm)
	{
		float dist;
		size_t faceidx;
		glm::vec2 uv;

		ret = pm->Intersect(pRayPos, pRayDir, &dist, &faceidx, &uv, mats->Top());
		if (ret && pDistance)
		{
			if (dist < *pDistance)
				*pDistance = dist;
			else
				ret = false;
		}
	}

	return ret;
}
