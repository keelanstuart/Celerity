// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"

#include <C3OmniLightImpl.h>

using namespace c3;


DECLARE_COMPONENTTYPE(OmniLight, OmniLightImpl);


OmniLightImpl::OmniLightImpl()
{
	m_pPos = nullptr;
	m_FS_deflight = m_VS_deflight = nullptr;
	m_SP_deflight = nullptr;
	m_SourceFB = nullptr;
	m_TexAttenRes = nullptr;
}


OmniLightImpl::~OmniLightImpl()
{
	if (m_SP_deflight)
	{
		m_SP_deflight->Release();
		m_SP_deflight = nullptr;
	}
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
	if (flags.IsSet(RF_FORCE))
		return true;

	if (!pobject->Flags().IsSet(OF_DRAW))
		return false;

	return true;
}


void OmniLightImpl::Render(Object *pobject, Object::RenderFlags flags)
{
	assert(pobject);

	c3::Renderer *prend = pobject->GetSystem()->GetRenderer();

	if (!m_SP_deflight)
	{
		ResourceManager *prm = pobject->GetSystem()->GetResourceManager();

		props::TFlags64 rf = c3::ResourceManager::RESFLAG(c3::ResourceManager::DEMANDLOAD);

		props::IProperty *pvsh = pobject->GetProperties()->GetPropertyById('VSHF');
		props::IProperty *pfsh = pobject->GetProperties()->GetPropertyById('FSHF');
		if (!m_VS_deflight)
			m_VS_deflight = (c3::ShaderComponent *)((prm->GetResource(pvsh ? pvsh->AsString() : _T("def-omnilight.vsh"), rf))->GetData());

		if (!m_FS_deflight)
			m_FS_deflight = (c3::ShaderComponent *)((prm->GetResource(pfsh ? pfsh->AsString() : _T("def-omnilight.fsh"), rf))->GetData());

		m_SP_deflight = pobject->GetSystem()->GetRenderer()->CreateShaderProgram();

		if (m_SP_deflight && m_VS_deflight && m_FS_deflight)
		{
			m_SP_deflight->AttachShader(m_VS_deflight);
			m_SP_deflight->AttachShader(m_FS_deflight);
			if (m_SP_deflight->Link() == ShaderProgram::RETURNCODE::RET_OK)
			{
				m_uniColor = m_SP_deflight->GetUniformLocation(_T("uLightColor"));
				m_uniPos = m_SP_deflight->GetUniformLocation(_T("uLightPos"));
				m_uniRadius = m_SP_deflight->GetUniformLocation(_T("uLightRadius"));
				m_uniScreenSize = m_SP_deflight->GetUniformLocation(_T("uScreenSize"));
				m_uniSampDiff = m_SP_deflight->GetUniformLocation(_T("uSamplerDiffuseMetalness"));
				m_uniSampNorm = m_SP_deflight->GetUniformLocation(_T("uSamplerNormalAmbOcc"));
				m_uniSampPosDepth = m_SP_deflight->GetUniformLocation(_T("uSamplerPosDepth"));
				m_uniSampEmisRough = m_SP_deflight->GetUniformLocation(_T("uSamplerEmissionRoughness"));
				m_uniTexAtten = m_SP_deflight->GetUniformLocation(_T("uSamplerAttenuation"));
			}
		}
	}

	props::IProperty *ppos = pobject->GetProperties()->GetPropertyById('POS');
	props::IProperty *pscl = pobject->GetProperties()->GetPropertyById('SCL');
	float scl = pscl->AsVec3F()->x;

	if (m_SP_deflight && m_SourceFB)
	{
		prend->UseProgram(m_SP_deflight);
		glm::vec2 ss;
		ss.x = (float)m_SourceFB->GetDepthTarget()->Width();
		ss.y = (float)m_SourceFB->GetDepthTarget()->Height();

		m_SP_deflight->SetUniform3(m_uniColor, (const glm::fvec3 *)(m_propColor->AsVec3F()));
		m_SP_deflight->SetUniform3(m_uniPos, (const glm::fvec3 *)ppos->AsVec3F());
		m_SP_deflight->SetUniform1(m_uniRadius, scl);
		m_SP_deflight->SetUniform2(m_uniScreenSize, &ss);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerDiffuseMetalness")), m_uniSampDiff);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerNormalAmbOcc")), m_uniSampNorm);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerPosDepth")), m_uniSampPosDepth);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerEmissionRoughness")), m_uniSampEmisRough);
		if (m_uniTexAtten != ShaderProgram::INVALID_UNIFORM)
		{
			Texture2D *ptex = m_TexAttenRes ? (Texture2D *)(m_TexAttenRes->GetData()) : prend->GetLinearGradientTexture();
			m_SP_deflight->SetUniformTexture(ptex ? ptex : prend->GetLinearGradientTexture(), m_uniTexAtten, -1, TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR);
		}
		prend->SetWorldMatrix(m_pPos->GetTransformMatrix());
		m_SP_deflight->ApplyUniforms();
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
		case 'VSHF':
			pr = prm->GetResource(pprop->AsString());
			m_VS_deflight = pr ? (c3::ShaderComponent *)(pr->GetData()) : nullptr;
			break;

		case 'FSHF':
			pr = prm->GetResource(pprop->AsString());
			m_FS_deflight = pr ? (c3::ShaderComponent *)(pr->GetData()) : nullptr;
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
