// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


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


props::TFlags64 OmniLightImpl::Flags()
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


bool OmniLightImpl::Prerender(Object *pobject, props::TFlags64 rendflags)
{
	if (rendflags.AnySet(Object::OBJFLAG(Object::LIGHT) || Object::OBJFLAG(Object::DRAWINEDITOR)))
		return true;

	return false;
}


void OmniLightImpl::Render(Object *pobject, props::TFlags64 rendflags)
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
				m_uniSampDiff = m_SP_deflight->GetUniformLocation(_T("uSamplerDiffuse"));
				m_uniSampNorm = m_SP_deflight->GetUniformLocation(_T("uSamplerNormal"));
				m_uniSampPosDepth = m_SP_deflight->GetUniformLocation(_T("uSamplerPosDepth"));
			}
		}
	}

	if (m_SP_deflight)
	{
		prend->UseProgram(m_SP_deflight);
		glm::vec2 ss;
		ss.x = (float)m_SourceFB->GetDepthTarget()->Width();
		ss.y = (float)m_SourceFB->GetDepthTarget()->Height();

		m_SP_deflight->SetUniform3(m_uniColor, (const glm::fvec3 *)(m_propColor->AsVec3F()));
		props::IProperty *ppos = pobject->GetProperties()->GetPropertyById('POS');
		m_SP_deflight->SetUniform3(m_uniPos, (const glm::fvec3 *)ppos->AsVec3F());
		props::IProperty *pscl = pobject->GetProperties()->GetPropertyById('SCL');
		m_SP_deflight->SetUniform1(m_uniRadius, pscl->AsVec3F()->x);
		m_SP_deflight->SetUniform2(m_uniScreenSize, &ss);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerDiffuse")), m_uniSampDiff);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerNormal")), m_uniSampNorm);
		m_SP_deflight->SetUniformTexture(m_SourceFB->GetColorTargetByName(_T("uSamplerPosDepth")), m_uniSampPosDepth);
		prend->SetWorldMatrix(m_pPos->GetTransformMatrix());
		m_SP_deflight->ApplyUniforms();
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

	switch (pprop->GetID())
	{
		case 'VSHF':
			m_VS_deflight = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
			break;

		case 'FSHF':
			m_FS_deflight = (c3::ShaderComponent *)((prm->GetResource(pprop->AsString()))->GetData());
			break;

		case 'LCLR':
			m_propColor = (props::IProperty *)pprop;
			break;
	}
}


bool OmniLightImpl::HitTest(glm::fvec3 *ray_pos, glm::fvec3 *rayvec) const
{
	return false;
}
