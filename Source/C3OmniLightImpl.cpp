// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#include "pch.h"

#include <C3OmniLightImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(OmniLight, OmniLightImpl);


OmniLightImpl::OmniLightImpl()
{
	m_pPos = nullptr;
	m_SourceFB = nullptr;
	m_TexAttenRes = nullptr;
	m_pMethod = nullptr;
}


OmniLightImpl::~OmniLightImpl()
{
}


void OmniLightImpl::Release()
{
	delete this;
}


props::TFlags64 OmniLightImpl::Flags() const
{
	return m_Flags;
}


bool OmniLightImpl::Initialize(Object *pobject)
{
	if (nullptr == (m_pOwner = pobject))
		return false;

	// get a positionable feature from the object -- and if we can't, don't proceed
	if (nullptr == (m_pPos = dynamic_cast<PositionableImpl *>(pobject->FindComponent(Positionable::Type()))))
		return false;

	props::IPropertySet *props = pobject->GetProperties();
	if (!props)
		return false;

	return true;
}


void OmniLightImpl::Update(Object *pobject, float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool OmniLightImpl::Prerender(Object *pobject, Object::RenderFlags flags)
{
	if (flags.IsSet(RF_FORCE) || flags.IsSet(RF_EDITORDRAW))
		return true;

	if (!pobject->Flags().IsSet(OF_DRAW))
		return false;

	if (!flags.IsSet(RF_LIGHT))
		return false;

	return true;
}


void OmniLightImpl::Render(Object *pobject, Object::RenderFlags flags)
{
	assert(pobject);
	if (!Prerender(pobject, flags))
		return;

	ResourceManager *prm = pobject->GetSystem()->GetResourceManager();
	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (!m_pMethod)
		{
			props::IProperty *pmethod = pobject->GetProperties()->GetPropertyById('C3RM');
			if (prm)
			{
				c3::Resource *pres = prm->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"));
				if (pres && (pres->GetStatus() == Resource::RS_LOADED))
				{
					m_pMethod = (RenderMethod *)(pres->GetData());

					if (m_pMethod->FindTechnique(_T("l"), m_TechIdx_L))
					{
						RenderMethod::Technique *pt = m_pMethod->GetTechnique(m_TechIdx_L);
						RenderMethod::Pass *pp = pt ? pt->GetPass(0) : nullptr;

						if (pt)
							pt->ApplyPass(0);

						ShaderProgram *ps = pp ? pp->GetShader() : nullptr;
						if (ps)
						{
							m_uniColor = ps->GetUniformLocation(_T("uLightColor"));
							m_uniPos = ps->GetUniformLocation(_T("uLightPos"));
							m_uniRadius = ps->GetUniformLocation(_T("uLightRadius"));
							m_uniScreenSize = ps->GetUniformLocation(_T("uScreenSize"));
							m_uniSampDiff = ps->GetUniformLocation(_T("uSamplerDiffuseMetalness"));
							m_uniSampNorm = ps->GetUniformLocation(_T("uSamplerNormalAmbOcc"));
							m_uniSampPosDepth = ps->GetUniformLocation(_T("uSamplerPosDepth"));
							m_uniSampEmisRough = ps->GetUniformLocation(_T("uSamplerEmissionRoughness"));
							m_uniTexAtten = ps->GetUniformLocation(_T("uSamplerAttenuation"));
						}
					}
				}
			}
		}
		else
		{
			prend->UseMaterial();
			prend->UseRenderMethod(m_pMethod);
			m_pMethod->SetActiveTechnique(m_TechIdx_L);

			glm::vec2 ss;
			ss.x = (float)m_SourceFB->GetDepthTarget()->Width();
			ss.y = (float)m_SourceFB->GetDepthTarget()->Height();

			props::IProperty *ppos = pobject->GetProperties()->GetPropertyById('POS');
			props::IProperty *pscl = pobject->GetProperties()->GetPropertyById('SCL');
			float scl = pscl->AsVec3F()->x;

			ShaderProgram *ps = m_pMethod->GetTechnique(m_TechIdx_L)->GetPass(0)->GetShader();
			if (ps)
			{
				ps->SetUniform3(m_uniColor, (const glm::fvec3 *)(m_propColor->AsVec3F()));
				ps->SetUniform3(m_uniPos, (const glm::fvec3 *)ppos->AsVec3F());
				ps->SetUniform1(m_uniRadius, scl);
				ps->SetUniform2(m_uniScreenSize, &ss);
				ps->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerDiffuseMetalness")), m_uniSampDiff);
				ps->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerNormalAmbOcc")), m_uniSampNorm);
				ps->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerPosDepth")), m_uniSampPosDepth);
				ps->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerEmissionRoughness")), m_uniSampEmisRough);

				if (m_uniTexAtten != ShaderProgram::INVALID_UNIFORM)
				{
					Texture2D *ptex = m_TexAttenRes ? (Texture2D *)(m_TexAttenRes->GetData()) : prend->GetLinearGradientTexture();
					ps->SetUniformTexture(ptex ? ptex : prend->GetLinearGradientTexture(), m_uniTexAtten, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
				}

				prend->SetWorldMatrix(m_pPos->GetTransformMatrix());
			}
		}
	}

	m_Bounds.SetOrigin(-1, -1, -1);
	m_Bounds.SetExtents(2.0f, 2.0f, 2.0f);
	m_Bounds.Align(m_pPos->GetTransformMatrix());

	bool isinside = m_Bounds.IsPointInside(prend->GetEyePosition());

	if (isinside)
	{
		prend->SetCullMode(c3::Renderer::CullMode::CM_FRONT);
		prend->SetDepthTest(c3::Renderer::Test::DT_ALWAYS);
	}
	else
	{
		prend->SetCullMode(c3::Renderer::CullMode::CM_BACK);
		prend->SetDepthTest(c3::Renderer::Test::DT_LESSEREQUAL);
	}

	prend->GetCubeMesh()->Draw();
}


void OmniLightImpl::SetSourceFrameBuffer(FrameBuffer *psource)
{
	m_SourceFB = psource;
}


void OmniLightImpl::PropertyChanged(const props::IProperty *pprop)
{
	assert(pprop);
	if (!m_pOwner)
		return;

	ResourceManager *prm = m_pOwner->GetSystem()->GetResourceManager();
	Resource *pr = nullptr;

	switch (pprop->GetID())
	{
		case 'C3RM':
			m_pMethod = nullptr;
			break;

		case 'LCLR':
			m_propColor = (props::IProperty *)pprop;
			break;

		case 'GRAD':
			m_TexAttenRes = prm->GetResource(pprop->AsString());
			break;
	}
}


bool OmniLightImpl::Intersect(const glm::vec3 * pRayPos, const glm::vec3 * pRayDir, float *pDistance) const
{
	return false;
}
