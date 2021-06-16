// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2020, Keelan Stuart


#pragma once

#include <c3.h>
#include <C3Material.h>

namespace c3
{

	class MaterialManager
	{

	public:

		/// Creates a new Material and returns the index
		virtual Material *CreateMaterial() = NULL;

		/// Removes a material from those that are managed
		virtual bool RemoveMaterial(const Material *mtl) = NULL;

		/// Returns the Texture2D that contains the material colors
		virtual Texture2D *GetMaterialColorTexture(const Material *pmtl) = NULL;

		/// Returns the texture coordinates for the given Material color component in the MaterialManager's texture
		/// mul_type_add is the amount at that you must multiply by the color type (Material::ColorComponentType) and add to coord
		virtual bool GetMaterialTexCoordInfo(const Material *pmaterial, glm::fvec2 &coord, glm::fvec2 &mul_type_add) = NULL;

	};

};