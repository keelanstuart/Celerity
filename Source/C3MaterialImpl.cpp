// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3MaterialImpl.h>
#include <C3MaterialManagerImpl.h>

using namespace c3;


MaterialImpl::MaterialImpl(MaterialManager *pmatman, Renderer *prend)
{
	m_pMatMan = pmatman;
	m_pRend = prend;

	m_flags = RENDERMODEFLAG(RMF_RENDERFRONT) | RENDERMODEFLAG(RMF_WRITEDEPTH) | RENDERMODEFLAG(RMF_READDEPTH);

	m_tex[ETextureComponentType::TCT_DIFFUSE] = prend->GetWhiteTexture();
	m_tex[ETextureComponentType::TCT_NORMAL] = prend->GetBlueTexture();
	m_tex[ETextureComponentType::TCT_EMISSIVE] = prend->GetBlackTexture();
	m_tex[ETextureComponentType::TCT_SURFACEDESC] = prend->GetBlackTexture();
	m_tex[ETextureComponentType::TCT_POSITIONDEPTH] = prend->GetBlackTexture();

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
	m_tex[comptype] = ptex;
}


Texture *MaterialImpl::GetTexture(TextureComponentType comptype) const
{
	return m_tex[comptype];
}


props::TFlags64 &MaterialImpl::RenderModeFlags()
{
	return m_flags;
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
	m_pRend->SetDepthMode(m_DepthMode);
	m_pRend->SetDepthTest(m_DepthTest);
	m_pRend->SetCullMode(m_CullMode);
	m_pRend->SetStencilEnabled(m_StencilEnabled);
	m_pRend->SetStencilTest(m_StencilTest, m_StencilRef, m_StencilMask);
	m_pRend->SetStencilOperation(m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp);

	if (shader)
	{
		int64_t ul_coldiff = shader->GetUniformLocation(_T("colDiffuse"));
		if (ul_coldiff >= 0)
			shader->SetUniform4(ul_coldiff, &m_color[CCT_DIFFUSE]);

		int64_t ul_colemis = shader->GetUniformLocation(_T("colEmissive"));
		if (ul_colemis >= 0)
			shader->SetUniform4(ul_colemis, &m_color[CCT_EMISSIVE]);

		int64_t ul_colspec = shader->GetUniformLocation(_T("colSpecular"));
		if (ul_colspec >= 0)
			shader->SetUniform4(ul_colspec, &m_color[CCT_SPECULAR]);

		int64_t ul_texdiff = shader->GetUniformLocation(_T("sampDiffuse"));
		if (ul_texdiff >= 0)
			shader->SetUniformTexture(ul_texdiff, 0, m_tex[TCT_DIFFUSE]);

		int64_t ul_texnorm = shader->GetUniformLocation(_T("sampNormal"));
		if (ul_texnorm >= 0)
			shader->SetUniformTexture(ul_texnorm, 1, m_tex[TCT_NORMAL]);

		int64_t ul_texsurf = shader->GetUniformLocation(_T("sampSurface"));
		if (ul_texsurf >= 0)
			shader->SetUniformTexture(ul_texsurf, 2, m_tex[TCT_SURFACEDESC]);

		int64_t ul_texemis = shader->GetUniformLocation(_T("sampEmissive"));
		if (ul_texemis >= 0)
			shader->SetUniformTexture(ul_texemis, 3, m_tex[TCT_EMISSIVE]);

		int64_t ul_texdepth = shader->GetUniformLocation(_T("sampPosDepth"));
		if (ul_texdepth >= 0)
			shader->SetUniformTexture(ul_texdepth, 4, m_tex[TCT_POSITIONDEPTH]);
	}

	return true;
}
