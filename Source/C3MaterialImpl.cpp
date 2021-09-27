// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3MaterialImpl.h>
#include <C3MaterialManagerImpl.h>
#include <C3Resource.h>

using namespace c3;

Material::MTL_ALT_TEXNAME_FUNC MaterialImpl::s_pfAltTexFilenameFunc = nullptr;

void Material::SetAlternateTextureFilenameFunc(Material::MTL_ALT_TEXNAME_FUNC func)
{
	MaterialImpl::s_pfAltTexFilenameFunc = func;
}


MaterialImpl::MaterialImpl(MaterialManager *pmatman, Renderer *prend)
{
	m_pMatMan = pmatman;
	m_pRend = prend;

	m_flags = RENDERMODEFLAG(RMF_RENDERFRONT) | RENDERMODEFLAG(RMF_WRITEDEPTH) | RENDERMODEFLAG(RMF_READDEPTH);

	m_tex[ETextureComponentType::TCT_DIFFUSE] = TTexOrRes(prend->GetWhiteTexture(), nullptr);
	m_tex[ETextureComponentType::TCT_NORMAL] = TTexOrRes(prend->GetBlueTexture(), nullptr);
	m_tex[ETextureComponentType::TCT_EMISSIVE] = TTexOrRes(prend->GetBlackTexture(), nullptr);
	m_tex[ETextureComponentType::TCT_SURFACEDESC] = TTexOrRes(prend->GetBlackTexture(), nullptr);
	m_tex[ETextureComponentType::TCT_POSITIONDEPTH] = TTexOrRes(prend->GetBlackTexture(), nullptr);

	m_color[EColorComponentType::CCT_DIFFUSE] = Color::White;
	m_color[EColorComponentType::CCT_EMISSIVE] = Color::Black;
	m_color[EColorComponentType::CCT_SPECULAR] = Color::DarkGrey;

	m_DepthTest = Renderer::Test::DT_LESSEREQUAL;

	m_StencilEnabled = false;
	m_StencilTest = Renderer::Test::DT_ALWAYS;
	m_StencilFailOp = Renderer::StencilOperation::SO_KEEP;
	m_StencilZFailOp = Renderer::StencilOperation::SO_KEEP;
	m_StencilZPassOp = Renderer::StencilOperation::SO_REPLACE;
	m_StencilRef = 0;
	m_StencilMask = 0xff;

	m_WindingOrder = Renderer::WindingOrder::WO_CW;
}


MaterialImpl::~MaterialImpl()
{
}


void MaterialImpl::Release()
{
	m_pMatMan->DestroyMaterial(this);
}


void MaterialImpl::SetColor(ColorComponentType comptype, const glm::fvec4 *pcolor)
{
	m_color[comptype] = pcolor ? *pcolor : Color::Black;
}


const glm::fvec4 *MaterialImpl::GetColor(ColorComponentType comptype, glm::fvec4 *pcolor) const
{
	if (!pcolor)
		return &m_color[comptype];

	*pcolor = m_color[comptype];
	return pcolor;
}


void MaterialImpl::SetTexture(TextureComponentType comptype, Texture *ptex)
{
	m_tex[comptype].first = ptex;
}


void MaterialImpl::SetTexture(TextureComponentType comptype, Resource *ptexres)
{
	m_tex[comptype].second = ptexres;
}


Texture *MaterialImpl::GetTexture(TextureComponentType comptype) const
{
	if (m_tex[comptype].second && (m_tex[comptype].second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a texture, then return it
		Texture *pt = dynamic_cast<Texture *>((Texture *)(m_tex[comptype].second->GetData()));
		if (pt)
			return pt;
	}

	return m_tex[comptype].first;
}


props::TFlags64 &MaterialImpl::RenderModeFlags()
{
	return m_flags;
}


void MaterialImpl::SetWindingOrder(Renderer::WindingOrder mode)
{
	m_WindingOrder = mode;
}


Renderer::WindingOrder MaterialImpl::GetWindingOrder()
{
	return m_WindingOrder;
}


void MaterialImpl::SetDepthTest(Renderer::Test test)
{
	m_DepthTest = test;
}


Renderer::Test MaterialImpl::GetDepthTest() const
{
	return m_DepthTest;
}


void MaterialImpl::SetStencilEnabled(bool en)
{
	m_StencilEnabled = en;
}


bool MaterialImpl::GetStencilEnabled() const
{
	return m_StencilEnabled;
}


void MaterialImpl::SetStencilOperation(Renderer::StencilOperation stencil_fail, Renderer::StencilOperation zfail, Renderer::StencilOperation zpass)
{
	m_StencilFailOp = stencil_fail;
	m_StencilZFailOp = zfail;
	m_StencilZPassOp = zpass;
}


void MaterialImpl::GetStencilOperation(Renderer::StencilOperation &stencil_fail, Renderer::StencilOperation &zfail, Renderer::StencilOperation &zpass) const
{
	stencil_fail = m_StencilFailOp;
	zfail = m_StencilZFailOp;
	zpass = m_StencilZPassOp;
}


void MaterialImpl::SetStencilTest(Renderer::Test test, uint8_t ref, uint8_t mask)
{
	m_StencilTest = test;
	m_StencilRef = ref;
	m_StencilMask = mask;
}


Renderer::Test MaterialImpl::GetStencilTest(uint8_t *ref, uint8_t *mask) const
{
	if (ref)
		*ref = m_StencilRef;

	if (mask)
		*mask = m_StencilMask;

	return m_StencilTest;
}


bool MaterialImpl::Apply(ShaderProgram *shader) const
{
	if (m_flags.IsSet(RENDERMODEFLAG(RMF_WRITEDEPTH) | RENDERMODEFLAG(RMF_READDEPTH)))
		m_pRend->SetDepthMode(Renderer::DepthMode::DM_READWRITE);
	else if (m_flags.IsSet(RENDERMODEFLAG(RMF_WRITEDEPTH)))
		m_pRend->SetDepthMode(Renderer::DepthMode::DM_WRITEONLY);
	else if (m_flags.IsSet(RENDERMODEFLAG(RMF_WRITEDEPTH)))
		m_pRend->SetDepthMode(Renderer::DepthMode::DM_READONLY);
	else
		m_pRend->SetDepthMode(Renderer::DepthMode::DM_DISABLED);

	m_pRend->SetDepthTest(m_DepthTest);

	if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERFRONT) | RENDERMODEFLAG(RMF_RENDERBACK)))
		m_pRend->SetCullMode(Renderer::CullMode::CM_DISABLED);
	else if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERFRONT)))
		m_pRend->SetCullMode(Renderer::CullMode::CM_BACK);
	else if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERBACK)))
		m_pRend->SetCullMode(Renderer::CullMode::CM_FRONT);
	else
		m_pRend->SetCullMode(Renderer::CullMode::CM_ALL);

	m_pRend->SetStencilEnabled(m_StencilEnabled);
	m_pRend->SetStencilTest(m_StencilTest, m_StencilRef, m_StencilMask);
	m_pRend->SetStencilOperation(m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp);

	m_pRend->SetWindingOrder(m_WindingOrder);

	if (shader)
	{
		int32_t ul_coldiff = shader->GetUniformLocation(_T("uColorDiffuse"));
		if (ul_coldiff != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniform4(ul_coldiff, &m_color[CCT_DIFFUSE]);

		int32_t ul_colemis = shader->GetUniformLocation(_T("uColorEmissive"));
		if (ul_colemis != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniform4(ul_colemis, &m_color[CCT_EMISSIVE]);

		int32_t ul_colspec = shader->GetUniformLocation(_T("uColorSpecular"));
		if (ul_colspec != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniform4(ul_colspec, &m_color[CCT_SPECULAR]);

		int32_t ul_texdiff = shader->GetUniformLocation(_T("uSamplerDiffuse"));
		if (ul_texdiff != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniformTexture(ul_texdiff, TCT_DIFFUSE, GetTexture(TCT_DIFFUSE));

		int32_t ul_texnorm = shader->GetUniformLocation(_T("uSamplerNormal"));
		if (ul_texnorm != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniformTexture(ul_texnorm, TCT_NORMAL, GetTexture(TCT_NORMAL));

		int32_t ul_texsurf = shader->GetUniformLocation(_T("uSamplerSurface"));
		if (ul_texsurf != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniformTexture(ul_texsurf, TCT_SURFACEDESC, GetTexture(TCT_SURFACEDESC));

		int32_t ul_texemis = shader->GetUniformLocation(_T("uSamplerEmissive"));
		if (ul_texemis != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniformTexture(ul_texemis, TCT_EMISSIVE, GetTexture(TCT_EMISSIVE));

		int32_t ul_texdepth = shader->GetUniformLocation(_T("uSamplerPosDepth"));
		if (ul_texdepth != ShaderProgram::INVALID_UNIFORM)
			shader->SetUniformTexture(ul_texdepth, TCT_POSITIONDEPTH, GetTexture(TCT_POSITIONDEPTH));
	}

	return true;
}
