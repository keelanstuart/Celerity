// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2026, Keelan Stuart


#pragma once

#include <C3Font.h>


namespace c3
{

	class FontImpl : public Font
	{

	public:
		// Constructor / destructor
		FontImpl(Renderer *prend, const TCHAR *fontname, size_t fontsize);
		virtual ~FontImpl();

		// Draws the specified text to the given VertexBuffer.
		// To actually draw the text on screen, set this Font's material (or use a clone), then call Renderer::DrawPrimitives(TRILIST)
		// with this return value as the number of prims
		virtual size_t RenderText(const TCHAR *text, VertexBuffer *pverts, props::TFlags32 draw_flags = 0, math::FRect2D *extents = nullptr, float tabwidth = 4.0f) const final;

		// Function to get extent of text
		virtual void GetTextExtent(const TCHAR *text, math::FRect2D &extent, float tabwidth = 4.0f) const final;

		virtual const Material *GetMaterial() const final;

		bool Initialize();

	protected:

		Renderer *m_pRend;
		tstring m_Name;
		size_t m_Size;
		bool m_Initialized;
		float m_Kerning;
		float m_ExtraLineSpacing;
		float m_SpaceWidth;

		Texture2D *m_Tex;
		Material *m_Mtl;

		using GlyphInfo = struct
		{
			struct
			{
				glm::fvec2 ul, lr;
			} uv;
			size_t w, h;
		};
		using TGlyphInfoMap = std::unordered_map<TCHAR, GlyphInfo>;
		TGlyphInfoMap m_GlyphInfo;

	};

};
