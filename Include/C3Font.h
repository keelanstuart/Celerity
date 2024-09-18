// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>
#include <C3ColorDefs.h>
#include <C3Math.h>


namespace c3
{


// Font rendering flags
#define FONTFLAG_ITALIC		0x0001
#define FONTFLAG_VTOP		0x0002
#define FONTFLAG_VCENTER	0x0004
#define FONTFLAG_VBOTTOM	0x0008
#define FONTFLAG_HLEFT		0x0010
#define FONTFLAG_HCENTER	0x0020
#define FONTFLAG_HRIGHT		0x0040



	class Font
	{

	public:

		static void C3_API ParseFontDescIntoLOGFONT(const TCHAR *fontdesc, LOGFONT &lf);

		// Draws the specified text to the given VertexBuffer.
		// To actually draw the text on screen, set this Font's material (or use a clone), then call Renderer::DrawPrimitives(TRILIST)
		// with this return value as the number of prims
		virtual size_t RenderText(const TCHAR *text, VertexBuffer *pverts, props::TFlags32 draw_flags = 0, math::FRect2D *extents = nullptr, float tabwidth = 4.0f) const = NULL;

		// Function to get extent of text
		virtual void GetTextExtent(const TCHAR *text, math::FRect2D &extent, float tabwidth = 4.0f) const = NULL;
	
		virtual const Material *GetMaterial() const = NULL;
	};

};
