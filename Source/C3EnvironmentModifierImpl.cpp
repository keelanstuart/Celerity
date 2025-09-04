// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#include "pch.h"

#include <C3EnvironmentModifierImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(EnvironmentModifier, EnvironmentModifierImpl);


EnvironmentModifierImpl::EnvironmentModifierImpl() :
	m_Gravity(0.0f, 0.0f, -9.8f),
	m_BackgroundColor(Color::fBlack),
	m_AmbColor(Color::fDarkGrey),
	m_SunColor(Color::fNaturalSunlight),
	m_SunDir(glm::normalize(glm::fvec3(0.2, 0.1, -0.7)))
{
	m_pPos = nullptr;

	m_LastFrameNum = 0;
	m_pSkyTexture = nullptr;
	m_pSkyMethod = nullptr;
	m_pSkyMtl = nullptr;
	m_pSkyModel = nullptr;
}


EnvironmentModifierImpl::~EnvironmentModifierImpl()
{
}


void EnvironmentModifierImpl::Release()
{
	delete this;
}


props::TFlags64 EnvironmentModifierImpl::Flags() const
{
	return m_Flags;
}


bool EnvironmentModifierImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *ps = pobject->GetProperties();
	if (!ps)
		return false;

	props::IProperty *pp;

	pp = ps->GetPropertyById('eBGC');
	if (!pp && ((pp = ps->CreateProperty(_T("uBackgroundColor"), 'eBGC')) && pp))
		pp->SetVec3F(*(props::TVec3F *)&m_BackgroundColor);
	else
		PropertyChanged(pp);
	if (pp)
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB);

	pp = ps->GetPropertyById('eAMB');
	if (!pp && ((pp = ps->CreateProperty(_T("uAmbientColor"), 'eAMB')) && pp))
		pp->SetVec3F(*(props::TVec3F *)&m_AmbColor);
	else
		PropertyChanged(pp);
	if (pp)
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_AMBIENT_COLOR);

	pp = ps->GetPropertyById('eSNC');
	if (!pp && ((pp = ps->CreateProperty(_T("uSunColor"), 'eSNC')) && pp))
		pp->SetVec3F(*(props::TVec3F *)&m_SunColor);
	else
		PropertyChanged(pp);
	if (pp)
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_COLOR);

	pp = ps->GetPropertyById('eSND');
	if (!pp && ((pp = ps->CreateProperty(_T("uSunDirection"), 'eSND')) && pp))
		pp->SetVec3F(*(props::TVec3F *)&m_SunDir);
	else
		PropertyChanged(pp);
	if (pp)
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SUN_DIRECTION);

	pp = ps->GetPropertyById('GRAV');
	if (!pp && ((pp = ps->CreateProperty(_T("Gravity"), 'GRAV')) && pp))
		pp->SetVec3F(*(props::TVec3F *)&m_Gravity);
	else
		PropertyChanged(pp);

	if (pp = ps->CreateProperty(_T("Sky.Texture"), 'SkTx'))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		pp->SetString(_T("circular_sky.tga"));
	}

	if (pp = ps->CreateProperty(_T("Sky.RenderMethod"), 'SkRm'))
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_FILENAME);
		pp->SetString(_T("skybox.c3rm"));
	}

	return true;
}


void EnvironmentModifierImpl::Update(float elapsed_time)
{
	if (!m_pPos)
		return;

	bool apply = m_pOwner->GetParent() ? false : true;

	System *psys = m_pOwner->GetSystem();

	Object *pco = psys->GetGlobalObjectRegistry()->GetRegisteredObject(GlobalObjectRegistry::OD_CAMERA_ROOT);
	Positionable *pcp = pco ? (Positionable *)pco->FindComponent(Positionable::Type()) : nullptr;
	if (pcp)
	{
		glm::fmat4x4 tm = *m_pPos->GetTransformMatrix();
		glm::fmat4x4 tn = *m_pPos->GetTransformMatrixNormal();

		static glm::fvec3 p[6] ={ glm::fvec3(1, 0, 0), glm::fvec3(0, 1, 0), glm::fvec3(0, 0, 1), glm::fvec3(-1, 0, 0), glm::fvec3(0, -1, 0), glm::fvec3(0, 0, -1) };

		size_t i;
		for (i = 0; i < 6; i++)
		{
			glm::fvec3 q = glm::fvec4(p[i], 1) * tm;
			glm::fvec3 n = glm::fvec4(p[i], 0) * tn;
			glm::fvec3 d = q - *(pcp->GetPosVec());
			if (glm::dot(n, d) <= 0)
				break;
		}

		apply = (i >= 6);
	}

	if (apply)
	{
		Environment *penv = m_pOwner->GetSystem()->GetEnvironment();

		penv->SetAmbientColor(m_AmbColor);
		penv->SetBackgroundColor(m_BackgroundColor);
		penv->SetSunColor(m_SunColor);
		penv->SetSunDirection(m_SunDir);
		penv->SetGravity(m_Gravity);
	}

	m_bCameraInside = apply;
}


bool EnvironmentModifierImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	size_t curframe = m_pOwner->GetSystem()->GetRenderer()->GetCurrentFrameNumber();
	if (m_LastFrameNum != curframe)
	{
		m_LastFrameNum = curframe;
		return true;
	}

	return false;
}


void EnvironmentModifierImpl::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)
{
	Renderer *pr = m_pOwner->GetSystem()->GetRenderer();
	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();

	if (!m_pSkyTexture)
	{
		props::IProperty *pp = m_pOwner->GetProperties()->GetPropertyById('SkTx');
		if (pp)
		{
			Resource *pres = prm->GetResource(pp->AsString(), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::Status::RS_LOADED))
			{
				m_pSkyTexture = dynamic_cast<Texture2D *>((Texture2D *)(pres->GetData()));
			}
		}
	}

	if (!m_pSkyTexture)
		m_pSkyTexture = pr->GetBlackTexture();

	if (!m_pSkyMtl)
		m_pSkyMtl = pr->GetMaterialManager()->CreateMaterial();

	if (!m_pSkyMethod)
	{
		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('SkRm');
		if (pmethod)
		{
			c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("skybox.c3rm"), RESF_DEMANDLOAD);
			if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				m_pSkyMethod = (RenderMethod *)(pres->GetData());
		}
	}

	if (!m_pSkyModel)
	{
		Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(_T("[hemisphere_med.model]"));
		m_pSkyModel = (Model *)pres->GetData();
	}

	if (m_pSkyModel && m_pSkyTexture && m_pSkyMethod && m_pSkyMtl)
	{
		Renderer::BlendMode oldbm = pr->GetBlendMode();
		Renderer::DepthMode olddm = pr->GetDepthMode();
		Renderer::CullMode oldcm = pr->GetCullMode();

		m_pSkyMethod->SetActiveTechnique(0);

		glm::fvec4 c = glm::fvec4(m_SunColor.x, m_SunColor.y, m_SunColor.z, 1.0f);
		m_pSkyMtl->SetColor(Material::CCT_DIFFUSE, &c);
		m_pSkyMtl->SetTexture(Material::TCT_DIFFUSE, m_pSkyTexture);

		glm::fmat4x4 mat;
		Object *pcamobj = m_pOwner->GetSystem()->GetGlobalObjectRegistry()->GetRegisteredObject(GlobalObjectRegistry::OD_CAMERA);
		if (pcamobj)
		{
			Camera *pcamcomp = dynamic_cast<Camera *>(pcamobj->FindComponent(Camera::Type()));
			if (pcamcomp)
			{
				// use the eye point position
				glm::fvec3 pos;
				pcamcomp->GetEyePos(&pos);
				mat = glm::translate(pos);
			}
		}

		pr->SetWorldMatrix(&mat);

		pr->UseRenderMethod(m_pSkyMethod);
		pr->UseMaterial(m_pSkyMtl);
		m_pSkyModel->Draw(&mat, false);

		pr->SetBlendMode(oldbm);
		pr->SetDepthMode(olddm);
		pr->SetCullMode(oldcm);
	}
}


void EnvironmentModifierImpl::PropertyChanged(const props::IProperty *pprop)
{
	if (!pprop)
		return;

	switch (pprop->GetID())
	{
		case 'SkTx':
			m_pSkyTexture = nullptr;
			break;

		case 'C3RM':
		case 'SkRm':
			m_pSkyMethod = nullptr;
			break;

		case 'eBGC':
			pprop->AsVec3F((props::TVec3F *)&m_BackgroundColor);
			break;

		case 'eAMB':
			pprop->AsVec3F((props::TVec3F *)&m_AmbColor);
			break;

		case 'eSNC':
			pprop->AsVec3F((props::TVec3F *)&m_SunColor);
			break;

		case 'eSND':
			pprop->AsVec3F((props::TVec3F *)&m_SunDir);
			break;

		case 'GRAV':
			pprop->AsVec3F((props::TVec3F *)&m_Gravity);
			break;
	}
}


bool EnvironmentModifierImpl::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, glm::fvec3 *pNormal, bool force) const
{
	return false;
}


