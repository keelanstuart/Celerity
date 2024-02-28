// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


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
	m_Color = Color::fWhite;
	m_Material = nullptr;
	m_pOwner = nullptr;
}


OmniLightImpl::~OmniLightImpl()
{
}


void OmniLightImpl::Release()
{
	props::IPropertySet *pps = m_pOwner->GetProperties();
	props::IProperty *pp;

	pp = pps->GetPropertyById('LCLR');
	if (pp) pp->ExternalizeReference();

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

	props::IProperty *pp;
	pp = props->CreateReferenceProperty(_T("LightColor"), 'LCLR', &m_Color, props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3);
	if (pp)
	{
		pp->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_RGB);
		pp->Flags().Set(props::IProperty::PROPFLAG(props::IProperty::ASPECTLOCKED));
	}

	return true;
}


void OmniLightImpl::Update(float elapsed_time)
{
	if (!m_pPos)
		return;
}


bool OmniLightImpl::Prerender(Object::RenderFlags flags, int draworder)
{
	if (!m_pMethod)
	{
		props::IProperty *pmethod = m_pOwner->GetProperties()->GetPropertyById('C3RM');
		if (pmethod)
		{
			c3::Resource *pres = m_pOwner->GetSystem()->GetResourceManager()->GetResource(pmethod ? pmethod->AsString() : _T("std.c3rm"));
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

	if (!flags.IsSet(RF_LIGHT))
		return false;

	if (flags.IsSet(RF_SHADOW))
		return false;

	if (flags.IsSet(RF_AUXILIARY))
		return false;

	RenderMethod::Technique *ptech = m_pMethod ? m_pMethod->GetTechnique(m_TechIdx_L) : nullptr;
	if (!ptech || (draworder == ptech->GetDrawOrder()))
	{
		if (m_pOwner->Flags().IsSet(OF_LIGHT) && flags.IsSet(RF_LIGHT))
			return true;
	}

	return false;
}


void OmniLightImpl::Render(Object::RenderFlags flags)
{
	c3::Renderer *prend = m_pOwner->GetSystem()->GetRenderer();

	m_SourceFB = prend->FindFrameBuffer(_T("GBuffer"));

	if (!m_SourceFB)
		return;

	if (!flags.IsSet(RF_LOCKSHADER))
	{
		if (m_pMethod)
		{
			prend->UseMaterial(m_Material);
			prend->UseRenderMethod(m_pMethod);
			m_pMethod->SetActiveTechnique(m_TechIdx_L);
			prend->SetWindingOrder(Renderer::WindingOrder::WO_CW);

			glm::vec2 ss;
			ss.x = (float)m_SourceFB->GetDepthTarget()->Width();
			ss.y = (float)m_SourceFB->GetDepthTarget()->Height();

			props::IProperty *ppos = m_pOwner->GetProperties()->GetPropertyById('POS');
			props::IProperty *pscl = m_pOwner->GetProperties()->GetPropertyById('SCL');
			float scl = pscl->AsVec3F()->x;

			ShaderProgram *ps = m_pMethod->GetTechnique(m_TechIdx_L)->GetPass(0)->GetShader();
			if (ps)
			{
				ps->SetUniform3(m_uniColor, &m_Color);

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
		prend->SetDepthTest(Renderer::Test::DT_ALWAYS);
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
			//pprop->AsVec3F((props::TVec3F *)&m_Color);
			break;

		case 'GRAD':
			m_TexAttenRes = prm->GetResource(pprop->AsString());
			break;
	}
}


bool OmniLightImpl::Intersect(const glm::vec3 * pRayPos, const glm::vec3 * pRayDir, MatrixStack *mats, float *pDistance) const
{
	return false;
}
