// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright � 2001-2021, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Material.h>
#include <C3Renderer.h>
#include <C3Texture.h>

namespace c3
{

	class MaterialImpl : public Material
	{

	protected:
		MaterialManager *m_pMatMan;
		Renderer *m_pRend;

		props::TFlags64 m_flags;
		Texture *m_tex[TextureComponentType::NUM_TEXTURETYPES];
		glm::fvec4 m_color[ColorComponentType::NUM_COLORTYPES];

		Renderer::DepthMode m_DepthMode;
		Renderer::Test m_DepthTest;

		Renderer::CullMode m_CullMode;

		bool m_StencilEnabled;
		Renderer::Test m_StencilTest;
		uint8_t m_StencilRef, m_StencilMask;
		Renderer::StencilOperation m_StencilFailOp, m_StencilZFailOp, m_StencilZPassOp;


	public:
		MaterialImpl(MaterialManager *pmatman, Renderer *prend);

		virtual ~MaterialImpl();

		virtual void Release();

		virtual void SetColor(ColorComponentType comptype, const glm::fvec4 *pcolor);

		virtual const glm::fvec4 *GetColor(ColorComponentType comptype, glm::fvec4 *pcolor = nullptr) const;

		virtual void SetTexture(TextureComponentType comptype, Texture *ptex);

		virtual Texture *GetTexture(TextureComponentType comptype) const;

		virtual props::TFlags64 &RenderModeFlags();

		virtual void SetDepthTest(Renderer::Test test);

		virtual Renderer::Test GetDepthTest() const;

		virtual void SetStencilEnabled(bool en);

		virtual bool GetStencilEnabled() const;

		virtual void SetStencilOperation(Renderer::StencilOperation stencil_fail, Renderer::StencilOperation zfail, Renderer::StencilOperation zpass);

		virtual void GetStencilOperation(Renderer::StencilOperation &stencil_fail, Renderer::StencilOperation &zfail, Renderer::StencilOperation &zpass) const;

		virtual void SetStencilTest(Renderer::Test test, uint8_t ref = 0, uint8_t mask = 0xff);

		virtual Renderer::Test GetStencilTest(uint8_t *ref = nullptr, uint8_t *mask = nullptr) const;

		virtual bool Apply(ShaderProgram *shader) const;

	};
}