// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>

namespace c3
{

	class Texture;

	class Material
	{

	public:

		typedef enum
		{
			CCT_DIFFUSE = 0,		// diffuse lighting modulator, RGBA maps literally (alpha is opacity)
			CCT_SPECULAR,			// specular lighting modulator, RGB maps literally, A maps to specular strength
			CCT_EMISSIVE,			// emissive lighting modulator, RGB maps literally, A ignored

			NUM_COLORTYPES
		} ColorComponentType;

		typedef enum
		{
			TCT_DIFFUSE = 0,		// diffuse map, RGBA maps literally (alpha is opacity)
			TCT_NORMAL,				// normal map, RGB maps to XYZ
			TCT_SPECULAR,			// specular map, RGB maps literally, A maps to specular strength
			TCT_EMISSIVE,			// emissive map, RGB maps literally, A ignored

			NUM_TEXTURETYPES
		} TextureComponentType;


		virtual void Release() = NULL;

		/// Sets the given material component color
		virtual void SetColor(TextureComponentType comptype, glm::fvec4 *pcolor) = NULL;

		/// GetColor returns the given component color
		virtual const glm::fvec4 *GetColor(ColorComponentType comptype, glm::fvec4 *pcolor) const = NULL;

		/// Sets the filename for the given texture type
		virtual void SetTextureFilename(TextureComponentType comptype, const TCHAR *filename) = NULL;

		/// Returns the active filename for the given texture type
		virtual const TCHAR *GetTextureFilename(ColorComponentType comptype) const = NULL;

		/// Gets the texture for the given type
		virtual Texture *GetTexture(TextureComponentType comptype) = NULL;

	};

};
