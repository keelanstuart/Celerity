// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2025, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3TextRenderer.h>
#include <C3ScriptableImpl.h>
#include <C3Math.h>

namespace c3
{

	class TextRendererImpl : public TextRenderer, props::IPropertyChangeListener, props::IProperty::IEnumProvider
	{

	protected:
		Object *m_pOwner;

		props::TFlags64 m_Flags;
#define IEF_UPDATEIMAGE		0x0001
#define IEF_UPDATETEXT		0x0002

		Color::SRGBAColor m_ImgColor;
		Color::SRGBAColor m_TextColor;

		bool m_BackgroundEnabled;
		Texture2D *m_pImage;
		RenderMethod *m_pMethodImage;
		size_t m_TechImage;
		RenderMethod *m_pMethodText;
		size_t m_TechText;
		Material *m_pTextMtl;
		Material *m_pImgMtl;
		VertexBuffer *m_pTextVB;
		Font *m_pFont;
		size_t m_TextQuads;
		glm::fvec4 m_ImageMargins;

		math::FRect2D m_Rect;

		using ImageMode = enum
		{
			IM_STRETCH = 0,
			IM_WRAPPROPORTIONAL,

			IM_FORCE64BIT = 0xFFFFFFFFFFFFFFFF
		};

		using VerticalAlignment = enum
		{
			VA_TOP = 0,
			VA_CENTER,
			VA_BOTTOM,

			VA_FORCE64BIT = 0xFFFFFFFFFFFFFFFF
		};
		VerticalAlignment m_VAlign;

		using HorizontalAlignment = enum
		{
			HA_LEFT = 0,
			HA_CENTER,
			HA_RIGHT,

			HA_FORCE64BIT = 0xFFFFFFFFFFFFFFFF
		};
		HorizontalAlignment m_HAlign;

	public:

		TextRendererImpl();

		virtual ~TextRendererImpl();

		virtual void Release();

		virtual const ComponentType *GetType() const;

		virtual props::TFlags64 Flags() const;

		virtual bool Initialize(Object *pobject);

		virtual void Update(float elapsed_time = 0.0f);

		virtual bool Prerender(Object::RenderFlags flags, int draworder);

		virtual void Render(Object::RenderFlags rendflags, const glm::fmat4x4 *pmat);

		virtual void PropertyChanged(const props::IProperty *pprop);

		virtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *pmat, float *pDistance, bool force) const;

		virtual size_t GetNumValues(const props::IProperty *pprop) const;

		virtual const TCHAR *GetValue(const props::IProperty *pprop, size_t ordinal, TCHAR *buf, size_t bufsize) const;

	};

	DEFINE_COMPONENTTYPE(TextRenderer, TextRendererImpl, GUID({0x2a1a3303, 0xb99f, 0x4f3b, {0xb6, 0x88, 0x5d, 0x13, 0xf4, 0xae, 0x77, 0xd0}}), "TextRenderer", "TextRenderer contains combined text and image rendering and event response scripting (requires Scriptable)", 0);

};