// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Material.h>
#include <C3Renderer.h>
#include <C3TextureImpl.h>

namespace c3
{
	class MaterialImpl : public Material
	{

	protected:
		MaterialManager *m_pMatMan;
		Renderer *m_pRend;

		props::TFlags64 m_flags;
		typedef std::pair<const Texture *, const Resource *> TTexOrRes;
		TTexOrRes m_tex[TextureComponentType::NUM_TEXTURETYPES];
		props::TFlags32 m_texflags[TextureComponentType::NUM_TEXTURETYPES];
		glm::fvec4 m_color[ColorComponentType::NUM_COLORTYPES];

		Renderer::CullMode m_CullMode;
		Renderer::WindingOrder m_WindingOrder;

		Renderer::Test m_DepthTest;

		bool m_StencilEnabled;
		Renderer::Test m_StencilTest;
		uint8_t m_StencilRef, m_StencilMask;
		Renderer::StencilOperation m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp;

	public:
		static Material::MTL_ALT_TEXNAME_FUNC s_pfAltTexFilenameFunc;


	public:
		MaterialImpl(MaterialManager *pmatman, Renderer *prend, const Material *copy_from = nullptr);

		virtual ~MaterialImpl();

		virtual void Copy(const Material *from);

		virtual void Release();

		virtual void SetColor(ColorComponentType comptype, const glm::fvec4 *pcolor);

		virtual void SetColor(ColorComponentType comptype, Color::SRGBAColor color);

		virtual const glm::fvec4 *GetColor(ColorComponentType comptype, glm::fvec4 *pcolor = nullptr) const;

		virtual void SetTexture(TextureComponentType comptype, const Texture *ptex, props::TFlags32 texflags = TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR | TEXFLAG_WRAP_U | TEXFLAG_WRAP_V);

		virtual void SetTexture(TextureComponentType comptype, const Resource *ptexres, props::TFlags32 texflags = TEXFLAG_MAGFILTER_LINEAR | TEXFLAG_MINFILTER_LINEAR | TEXFLAG_MINFILTER_MIPLINEAR | TEXFLAG_WRAP_U | TEXFLAG_WRAP_V);

		virtual const Texture *GetTexture(TextureComponentType comptype, props::TFlags32 *texflags = nullptr) const;

		virtual props::TFlags64 &RenderModeFlags();

		virtual void SetCullMode(Renderer::CullMode mode);

		virtual Renderer::CullMode GetCullMode();

		virtual void SetWindingOrder(Renderer::WindingOrder mode);

		virtual Renderer::WindingOrder GetWindingOrder();

		virtual void SetDepthTest(Renderer::Test test);

		virtual Renderer::Test GetDepthTest() const;

		virtual void SetStencilEnabled(bool en);

		virtual bool GetStencilEnabled() const;

		virtual void SetStencilOperation(Renderer::StencilOperation stencil_fail, Renderer::StencilOperation zfail, Renderer::StencilOperation zpass);

		virtual void GetStencilOperation(Renderer::StencilOperation &stencil_fail, Renderer::StencilOperation &zfail, Renderer::StencilOperation &zpass) const;

		virtual void SetStencilTest(Renderer::Test test, uint8_t ref = 0, uint8_t mask = 0xff);

		virtual Renderer::Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const;

		virtual bool Apply(ShaderProgram *shader, Renderer::RenderStateOverrideFlags overridden) const;

	};
}