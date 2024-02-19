// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2024, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Renderer.h>
#include <C3ColorDefs.h>


namespace c3
{


// Font rendering flags
#define FONTFLAG_ITALIC		0x0001



	class Font
	{

	public:

		static void C3_API ParseFontDescIntoLOGFONT(const TCHAR *fontdesc, LOGFONT &lf);

		// Draws the specified text to the given VertexBuffer.
		// To actually draw the text on screen, set this Font's material (or use a clone), then call Renderer::DrawPrimitives(TRILIST)
		// with this return value as the number of prims
		virtual size_t RenderText(const TCHAR *text, VertexBuffer *pverts, props::TFlags32 draw_flags = 0, float tabwidth = 4.0f) const = NULL;

		// Function to get extent of text
		virtual void GetTextExtent(const TCHAR *text, RECT &extent, float tabwidth = 4.0f) const = NULL;
	
		virtual const Material *GetMaterial() const = NULL;
	};

};
