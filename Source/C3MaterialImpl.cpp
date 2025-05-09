// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2025, Keelan Stuart


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


MaterialImpl::MaterialImpl(MaterialManager *pmatman, Renderer *prend, const Material *copy_from)
{
	m_pMatMan = pmatman;
	m_pRend = prend;

	if (!copy_from)
	{
		m_flags = RENDERMODEFLAG(RMF_RENDERFRONT) | RENDERMODEFLAG(RMF_WRITEDEPTH) | RENDERMODEFLAG(RMF_READDEPTH);

		m_tex[ETextureComponentType::TCT_DIFFUSE] = TTexOrRes(prend->GetWhiteTexture(), nullptr);
		m_tex[ETextureComponentType::TCT_NORMAL] = TTexOrRes(prend->GetDefaultNormalTexture(), nullptr);
		m_tex[ETextureComponentType::TCT_EMISSIVE] = TTexOrRes(prend->GetBlackTexture(), nullptr);
		m_tex[ETextureComponentType::TCT_SURFACEDESC] = TTexOrRes(prend->GetDefaultDescTexture(), nullptr);
		m_tex[ETextureComponentType::TCT_POSITIONDEPTH] = TTexOrRes(prend->GetBlackTexture(), nullptr);

		for (size_t i = 0; i < ETextureComponentType::NUM_TEXTURETYPES; i++)
			m_texflags[i] = TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR | TEXFLAG_WRAP_U | TEXFLAG_WRAP_V;

		m_color[EColorComponentType::CCT_DIFFUSE] = Color::fWhite;
		m_color[EColorComponentType::CCT_EMISSIVE] = Color::fBlack;
		m_color[EColorComponentType::CCT_SPECULAR] = Color::fDarkGrey;

		m_DepthTest = Renderer::Test::DT_LESSER;

		m_StencilEnabled = false;
		m_StencilTest = Renderer::Test::DT_ALWAYS;
		m_StencilFailOp = Renderer::StencilOperation::SO_KEEP;
		m_StencilZFailOp = Renderer::StencilOperation::SO_KEEP;
		m_StencilZPassOp = Renderer::StencilOperation::SO_KEEP;
		m_StencilRef = 0;
		m_StencilMask = 0xff;

		m_WindingOrder = Renderer::WindingOrder::WO_CW;
	}
	else
	{
		Copy(copy_from);
	}
}


MaterialImpl::~MaterialImpl()
{
}


void MaterialImpl::Copy(const Material *from)
{
	if (!from)
		return;

	m_flags				= ((const MaterialImpl *)from)->m_flags;
	m_CullMode			= ((const MaterialImpl *)from)->m_CullMode;
	m_WindingOrder		= ((const MaterialImpl *)from)->m_WindingOrder;
	m_DepthTest			= ((const MaterialImpl *)from)->m_DepthTest;
	m_StencilEnabled	= ((const MaterialImpl *)from)->m_StencilEnabled;
	m_StencilTest		= ((const MaterialImpl *)from)->m_StencilTest;
	m_StencilRef		= ((const MaterialImpl *)from)->m_StencilRef;
	m_StencilMask		= ((const MaterialImpl *)from)->m_StencilMask;
	m_StencilFailOp		= ((const MaterialImpl *)from)->m_StencilFailOp;
	m_StencilZFailOp	= ((const MaterialImpl *)from)->m_StencilZFailOp;
	m_StencilZPassOp	= ((const MaterialImpl *)from)->m_StencilZPassOp;

	for (size_t i = 0; i < TextureComponentType::NUM_TEXTURETYPES; i++)
	{
		m_tex[i] = ((const MaterialImpl *)from)->m_tex[i];
		m_texflags[i] = ((const MaterialImpl *)from)->m_texflags[i];
	}

	for (size_t i = 0; i < ColorComponentType::NUM_COLORTYPES; i++)
		m_color[i] = ((const MaterialImpl *)from)->m_color[i];
}


void MaterialImpl::Release()
{
	m_pMatMan->DestroyMaterial(this);
}


void MaterialImpl::SetColor(ColorComponentType comptype, const glm::fvec4 *pcolor)
{
	m_color[comptype] = pcolor ? *pcolor : Color::fBlack;
}


void MaterialImpl::SetColor(ColorComponentType comptype, Color::SRGBAColor color)
{
	Color::ConvertIntToVec(color, (props::TVec4F &)m_color[comptype]);
}


const glm::fvec4 *MaterialImpl::GetColor(ColorComponentType comptype, glm::fvec4 *pcolor) const
{
	if (!pcolor)
		return &m_color[comptype];

	*pcolor = m_color[comptype];
	return pcolor;
}


void MaterialImpl::SetTexture(TextureComponentType comptype, Texture *ptex, props::TFlags32 texflags)
{
	m_tex[comptype].first = ptex;

	// Since the Resource takes precedent over the bare Texture, clear out the Resource if you set the Texture
	m_tex[comptype].second = nullptr;

	m_texflags[comptype] = texflags;
}


void MaterialImpl::SetTexture(TextureComponentType comptype, Resource *ptexres, props::TFlags32 texflags)
{
	m_tex[comptype].second = ptexres;

	m_texflags[comptype] = texflags;
}


Texture *MaterialImpl::GetTexture(TextureComponentType comptype, props::TFlags32 *texflags) const
{
	Texture *ret = nullptr;

	if (m_tex[comptype].second && (m_tex[comptype].second->GetStatus() == Resource::Status::RS_LOADED))
	{
		// if the resource is loaded and actually a texture, then return it
		Texture *pt = dynamic_cast<Texture *>((Texture *)(m_tex[comptype].second->GetData()));
		if (pt)
			ret = pt;
	}

	if (!ret)
		ret = m_tex[comptype].first;

	if (texflags)
		*texflags = m_texflags[comptype];

	return ret;
}


props::TFlags64 &MaterialImpl::RenderModeFlags()
{
	return m_flags;
}


void MaterialImpl::SetCullMode(Renderer::CullMode mode)
{
	m_CullMode = mode;
}


Renderer::CullMode MaterialImpl::GetCullMode()
{
	return m_CullMode;
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


bool MaterialImpl::Apply(ShaderProgram *shader, Renderer::RenderStateOverrideFlags overridden) const
{
	if (!overridden.IsSet(RSOF_DEPTHMODE))
	{
		if (m_flags.IsSet(RENDERMODEFLAG(RMF_WRITEDEPTH) | RENDERMODEFLAG(RMF_READDEPTH)))
			m_pRend->SetDepthMode(Renderer::DepthMode::DM_READWRITE);
		else if (m_flags.IsSet(RENDERMODEFLAG(RMF_WRITEDEPTH)))
			m_pRend->SetDepthMode(Renderer::DepthMode::DM_WRITEONLY);
		else if (m_flags.IsSet(RENDERMODEFLAG(RMF_READDEPTH)))
			m_pRend->SetDepthMode(Renderer::DepthMode::DM_READONLY);
		else
			m_pRend->SetDepthMode(Renderer::DepthMode::DM_DISABLED);
	}

	if (!overridden.IsSet(RSOF_FILLMODE))
	{
		if (m_flags.IsSet(RENDERMODEFLAG(RMF_WIREFRAME)))
			m_pRend->SetFillMode(Renderer::FillMode::FM_WIRE);
		else
			m_pRend->SetFillMode(Renderer::FillMode::FM_FILL);
	}

	if (!overridden.IsSet(RSOF_DEPTHTEST))
	{
		m_pRend->SetDepthTest(m_DepthTest);
	}

	if (!overridden.IsSet(RSOF_CULLMODE))
	{
		if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERFRONT) | RENDERMODEFLAG(RMF_RENDERBACK)))
			m_pRend->SetCullMode(Renderer::CullMode::CM_DISABLED);
		else if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERFRONT)))
			m_pRend->SetCullMode(Renderer::CullMode::CM_BACK);
		else if (m_flags.IsSet(RENDERMODEFLAG(RMF_RENDERBACK)))
			m_pRend->SetCullMode(Renderer::CullMode::CM_FRONT);
		else
			m_pRend->SetCullMode(Renderer::CullMode::CM_ALL);
	}

	if (!overridden.IsSet(RSOF_STENCIL))
	{
		m_pRend->SetStencilEnabled(m_StencilEnabled);
	}

	if (!overridden.IsSet(RSOF_STENCILFUNC))
	{
		m_pRend->SetStencilTest(m_StencilTest, m_StencilRef, m_StencilMask);
	}

	if (!overridden.IsSet(RSOF_STENCILOP))
	{
			m_pRend->SetStencilOperation(m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp);
	}

	if (!overridden.IsSet(RSOF_WINDINGORDER))
	{
		m_pRend->SetWindingOrder(m_WindingOrder);
	}

	if (shader)
	{
		props::TFlags32 texflags;

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
		{
			Texture *ptex = GetTexture(TCT_DIFFUSE, &texflags);
			shader->SetUniformTexture(ptex, ul_texdiff, -1, texflags);
		}

		int32_t ul_texnorm = shader->GetUniformLocation(_T("uSamplerNormal"));
		if (ul_texnorm != ShaderProgram::INVALID_UNIFORM)
		{
			Texture *ptex = GetTexture(TCT_NORMAL, &texflags);
			shader->SetUniformTexture(ptex, ul_texnorm, -1, texflags);
		}

		int32_t ul_texsurf = shader->GetUniformLocation(_T("uSamplerSurfaceDesc"));
		if (ul_texsurf != ShaderProgram::INVALID_UNIFORM)
		{
			Texture *ptex = GetTexture(TCT_SURFACEDESC, &texflags);
			shader->SetUniformTexture(ptex, ul_texsurf, -1, texflags);
		}

		int32_t ul_texemis = shader->GetUniformLocation(_T("uSamplerEmissive"));
		if (ul_texemis != ShaderProgram::INVALID_UNIFORM)
		{
			Texture *ptex = GetTexture(TCT_EMISSIVE, &texflags);
			shader->SetUniformTexture(ptex, ul_texemis, -1, texflags);
		}

		int32_t ul_texdepth = shader->GetUniformLocation(_T("uSamplerPosDepth"));
		if (ul_texdepth != ShaderProgram::INVALID_UNIFORM)
		{
			Texture *ptex = GetTexture(TCT_POSITIONDEPTH, &texflags);
			shader->SetUniformTexture(ptex, ul_texdepth, -1, texflags);
		}
	}

	return true;
}
