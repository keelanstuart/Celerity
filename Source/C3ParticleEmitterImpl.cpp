// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3ParticleEmitterImpl.h>
#include <C3EnvironmentModifierImpl.h>
#include <C3Math.h>
#include <C3Utility.h>
#include <C3Positionable.h>

using namespace c3;


DECLARE_COMPONENTTYPE(ParticleEmitter, ParticleEmitterImpl);


ParticleEmitterImpl::ParticleEmitterImpl()
{
	m_pMethod = nullptr;
	m_pTexture = nullptr;
	m_MaxParticles = 100;
	m_LastEmitPos.x = m_LastEmitPos.y = m_LastEmitPos.z = 0.0f;
}


ParticleEmitterImpl::~ParticleEmitterImpl()
{
}


void ParticleEmitterImpl::Release()
{
	props::IPropertySet *pps = m_pOwner->GetProperties();
	props::IProperty *pp;

	pp = pps->GetPropertyById('PEsh');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PErt');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEsp');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PElf');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEac');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEgr');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PErl');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEss');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEps');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEes');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PErd');
	if (pp) pp->ExternalizeReference();

	pp = pps->GetPropertyById('PEpp');
	if (pp) pp->ExternalizeReference();

	delete this;
}


props::TFlags64 ParticleEmitterImpl::Flags() const
{
	return 0;
}


bool ParticleEmitterImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	Positionable *ppos = (Positionable *)m_pOwner->FindComponent(Positionable::Type());
	if (ppos)
		ppos->GetPosVec(&m_LastEmitPos);

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
		pp->SetInt(m_MaxParticles);
	}

	m_EmitRate.min = 15.0f;
	m_EmitRate.max = 30.0f;
	if (pp = propset->CreateReferenceProperty(_T("EmitRate"), 'PErt', &m_EmitRate, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2))
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::MINMAX));
	}

	m_EmitSpeed.min = 0.0f;
	m_EmitSpeed.max = 1.0f;
	if (pp = propset->CreateReferenceProperty(_T("EmitSpeed"), 'PEsp', &m_EmitSpeed, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2))
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::MINMAX));
	}

	m_ParticleLife.min = 1.0f;
	m_ParticleLife.max = 2.0f;
	pp = propset->CreateReferenceProperty(_T("ParticleLife"), 'PElf', &m_ParticleLife, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2);
	if (pp)
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::MINMAX));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_TIME_SECONDS);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	m_Acceleration.min = 0.0f;
	m_Acceleration.max = 0.0f;
	if (pp = propset->CreateReferenceProperty(_T("Acceleration"), 'PEac', &m_Acceleration, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2))
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::MINMAX));
	}

	m_Gravity = 0.0f;
	propset->CreateReferenceProperty(_T("Gravity"), 'PEgr', &m_Gravity, props::IProperty::PROPERTY_TYPE::PT_FLOAT);

	m_Roll.min = -10.0f;
	m_Roll.max = 10.0f;
	pp = propset->CreateReferenceProperty(_T("Roll"), 'PErl', &m_Roll, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2);
	if (pp)
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::MINMAX));
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_ROTATION_DEG);
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

	m_Radius.inner = 2.0f;
	m_Radius.outer = 2.0f;
	if (pp = propset->CreateReferenceProperty(_T("Radius"), 'PErd', &m_Radius, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2))
	{
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::INNEROUTER));
	}

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

	glm::fmat4x4 mat = glm::identity<glm::fmat4x4>();
	util::ComputeFinalTransform(m_pOwner, &mat);

	glm::fvec3 empos(0, 0, 0), emposdiff;
	Positionable *ppos = (Positionable *)m_pOwner->FindComponent(Positionable::Type());
	if (ppos)
		ppos->GetPosVec(&empos);
	emposdiff = empos - m_LastEmitPos;

	glm::fvec3 epos = mat * glm::fvec4(glm::fvec3(0, 0, 0), 1);
	glm::fvec3 facing = mat * glm::fvec4(glm::fvec3(0, 1, 0), 0);
	glm::fvec3 right = mat * glm::fvec4(glm::fvec3(1, 0, 0), 0);
	glm::fvec3 up = mat * glm::fvec4(glm::fvec3(0, 0, 1), 0);
	glm::fvec3 vscl_up = mat * glm::fvec4(glm::fvec3(0, 0, 1), 1);
	float scl_up = glm::length(vscl_up);

	size_t pc = 0;

	glm::fvec3 acc;
	glm::fvec3 grav;
	m_pOwner->GetSystem()->GetEnvironment()->GetGravity(&grav);
	grav *= m_Gravity;

	for (size_t a = 0; a < m_Active.size(); a++)
	{
		SParticle *p = nullptr;

		size_t i = m_Active[a];
		if (i < m_Particles.size())
			p = &m_Particles[i];

		if (p)
			p->time += elapsed_time;

		if (!p || (p->time >= p->lifetime))
		{
			m_Inactive.push_back(i);
			m_Active.erase(m_Active.begin() + a);
			a--;
			continue;
		}

		acc = p->acc + grav;
		p->pos += (p->vel * elapsed_time);
		p->vel += (acc * elapsed_time);
		p->roll += (p->rvel * elapsed_time);
		p->rvel += (p->racc * elapsed_time);
	}

	if (!m_pOwner->Flags().IsSet(OF_KILL) && (m_EmitRate.min > 0) && (m_EmitRate.max > 0))
	{
		m_EmitTime -= elapsed_time;

		float emit_count_sec = math::RandomRange(m_EmitRate.min, m_EmitRate.max);
		float emit_count_cur = emit_count_sec / fabs(m_EmitTime);
		float emit_time_single = 1.0f / emit_count_sec;
		float pct_single = emit_time_single / fabs(m_EmitTime);
		float pct_cur = 0.0f;

		// if we've lapsed our emission time and have inactive particles to use, then make one
		while ((m_EmitTime < 0.0f) && !m_Inactive.empty())
		{
			m_EmitTime += emit_time_single;

			size_t i = m_Inactive.back();
			m_Inactive.pop_back();

			SParticle &part = m_Particles[i];

			part.time = 0;
			part.lifetime = math::RandomRange(m_ParticleLife.min, m_ParticleLife.max);

			float radius = math::RandomRange(m_Radius.inner, m_Radius.outer);

			switch (m_Shape)
			{
				case EmitterShape::SPHERE:
				{
					// find a random emission vector
					part.vel = glm::normalize(glm::fvec3(math::RandomRange(-1.0f, 1.0f), math::RandomRange(-1.0f, 1.0f), math::RandomRange(-1.0f, 1.0f)));

					// set the emission origin
					part.pos = epos + (part.vel * radius);
					break;
				}

				case EmitterShape::CYLINDER:
				{
					float angle = math::RandomRange(0, glm::two_pi<float>());
					part.vel = (right * cosf(angle)) + (facing * sinf(angle));

					part.pos = epos;
					part.pos += math::RandomRange(0, scl_up) * up;
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
					part.vel = up;
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
			part.acc = part.vel * math::RandomRange(m_Acceleration.min, m_Acceleration.max);

			float spd = math::RandomRange(m_EmitSpeed.min, m_EmitSpeed.max);
			part.vel *= spd;
			part.pos += (emposdiff * pct_cur);

			part.roll = 0.0f;
			part.rvel = glm::radians(math::RandomRange(m_Roll.min, m_Roll.max));
			part.racc = 0.0f; // TODO

			m_Active.push_back(i);

			pct_cur += pct_single;
		}
	}

	m_LastEmitPos = empos;
}


bool ParticleEmitterImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (!m_pMethod)
	{
		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('PErm');
		if (pmethod)
		{
			ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

			c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("particles.c3rm"), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
			{
				m_pMethod = (RenderMethod *)(pres->GetData());

				m_pMethod->FindTechnique(_T("g"), m_TechIdx_G);
			}
		}
	}

	if (flags.IsSet(RF_EFFECT))
		return true;

	if (flags.IsSet(RF_SHADOW))
		return false;

	if (flags.IsSet(RF_LIGHT))
		return false;

	if (flags.IsSet(RF_AUXILIARY))
		return false;

	RenderMethod::Technique *ptech = m_pMethod ? m_pMethod->GetTechnique(m_TechIdx_G) : nullptr;
	if (!ptech || (draworder == ptech->GetDrawOrder()))
	{
		if (m_pOwner->Flags().IsSet(OF_DRAW))
			return true;
	}

	return false;
}


void ParticleEmitterImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

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
			Color::SRGBAColor color;
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
					v->color.r = (uint8_t)(std::lerp((float)m_StartColor.r, (float)m_PeakColor.r, t));
					v->color.g = (uint8_t)(std::lerp((float)m_StartColor.g, (float)m_PeakColor.g, t));
					v->color.b = (uint8_t)(std::lerp((float)m_StartColor.b, (float)m_PeakColor.b, t));
					v->color.a = (uint8_t)(std::lerp((float)m_StartColor.a, (float)m_PeakColor.a, t));
				}
				else
				{
					t = sinf(((pct - m_Peak) / t2) * glm::half_pi<float>());

					v->scale = std::lerp(m_PeakScale, m_EndScale, t);
					v->color.r = (uint8_t)(std::lerp((float)m_PeakColor.r, (float)m_EndColor.r, t));
					v->color.g = (uint8_t)(std::lerp((float)m_PeakColor.g, (float)m_EndColor.g, t));
					v->color.b = (uint8_t)(std::lerp((float)m_PeakColor.b, (float)m_EndColor.b, t));
					v->color.a = (uint8_t)(std::lerp((float)m_PeakColor.a, (float)m_EndColor.a, t));
				}

				v++;
			}

			m_Verts->Unlock();
		}
	}

	auto poldmethod = pr->GetActiveRenderMethod();

	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (m_pMethod)
		{
			m_pMethod->SetActiveTechnique(m_TechIdx_G);
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

	pr->UseRenderMethod(poldmethod);
}


void ParticleEmitterImpl::PropertyChanged(const props::IProperty *pprop)
{
	props::FOURCHARCODE fcc = pprop->GetID();

	switch (fcc)
	{
		case 'PEmp':
		{
			// Only 1-1000 particles
			m_MaxParticles = std::max<int64_t>(std::min<int64_t>(1, pprop->AsInt()), 1000);
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
			// NOTE: for any of these enum provider properties, reset this... sigh. Maybe come up with a better solution later.
			((props::IProperty *)pprop)->SetEnumProvider(this);
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


bool ParticleEmitterImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, bool force) const
{
#if 0
	if (!pRayPos || !pRayDir)
		return false;

	bool ret = false;

	static glm::fmat4x4 imat = glm::identity<glm::fmat4x4>();
	if (!pmat)
		pmat = &imat;

	float dist;
	glm::fvec3 pos = *pmat * glm::fvec4(0, 0, 0, 1);
	glm::fvec3 vscl = *pmat * glm::fvec4(glm::normalize(glm::fvec3(m_Radius.max, m_Radius.max, m_Radius.max)), 0);
	float scl = (vscl.x + vscl.y + vscl.z) / 3.0f;

	ret = glm::intersectRaySphere(*pRayPos, *pRayDir, pos, scl, dist);
	if (ret && pDistance)
	{
		if (dist < *pDistance)
			*pDistance = dist;
		else
			ret = false;
	}

	return ret;
#else
	return false;
#endif
}
